/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Ground ZERO improvements copyright pending 1994, 1995 by James Hilke   *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "ground0.h"

extern char *color_table[];

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_skills	);
DECLARE_DO_FUN(do_outfit	);

GOD_TYPE *get_god args ((char *a_name));
int port = 4000;

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
#endif

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
int	accept		args( ( int s, void *addr, int *addrlen ) );
int	bind		args( ( int s, const void *addr, int addrlen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname,
 				const void *optval, int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if	0
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static	long			theKeys	[4];

int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	int		errno;
#endif

#if	defined(MSDOS)
int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
/*int	bind		args( ( int s, struct sockaddr *name, int namelen ) );*/
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
/*int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );*/
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int setsockopt		args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
/*int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );*/
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA    *descriptor_free;	/* Free list for descriptors	*/
DESCRIPTOR_DATA    *descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA    *d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* time of this pulse */	
ROOM_DATA	   *ammo_repop[3];
ROOM_DATA          *safe_area, *someimp_area;
ROOM_DATA          *god_general_area, *explosive_area;
ROOM_DATA	   *store_area;
ROOM_DATA          *red_repop, *blue_repop, *yellow_repop, *green_repop;
int                 iteration = 0;
int		    ground0_down;	/* Shutdown + return value      */

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
void	new_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );

void argument_error ()
{
  fprintf (stderr, "Syntax: ground0 -p <port>\n");
  exit(STATUS_BOOT_ERROR);
}

void parse_arguments (int argv, char **argc)
{
  int count;

  for (count = 1; count < argv; count++)
    if (argc[count][0] != '-')
      argument_error ();
    else
      switch (argc[count][1])
        {
        case 'p':
          count++;
          if (count < argv && is_number (argc[count]))
            {
              port = atoi (argc[count]);
              break;
            }
	  argument_error ();
        default:
	  argument_error ();
	}
}

int main( int argv, char **argc )
{
    struct timeval now_time;

#if defined(unix)
    int control;
#endif

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time 	= (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit (STATUS_ERROR);
    }

    parse_arguments (argv, argc);

    /*
     * Run the game.
     */
    control = init_socket( port );
    log_string ("Socket initiated.");
    boot_db( );
    sprintf( log_buf, "Ground ZERO is ready to rock on port %d.", port );
    log_string( log_buf );
    game_loop_unix( control );
    close (control);

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    sprintf (log_buf, "Exitting with value %d.", ground0_down);
    log_string (log_buf);
    exit( ground0_down );
}



#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit (STATUS_ERROR);
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close(fd);
	exit (STATUS_ERROR);
    }

#if 0
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close(fd);
	    exit (STATUS_ERROR);
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
      fprintf (stderr, "bind() failed.  Retrying . . .\n");
      while (bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
	sleep(1);
    }


    if ( listen( fd, 3 ) < 0 )
    {
	perror("Init socket: listen");
	close(fd);
	exit (STATUS_ERROR);
    }

    return fd;
}
#endif

#if defined(unix)
void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !ground0_down )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

	iteration++;
#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit (STATUS_ERROR);
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    new_descriptor( control );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		d->outtop	= 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
	      if ( d->wait > 0 )
		{
		  --d->wait;
		  continue;
		}
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL)
		      save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;

		if (d->showstr_point)
		    show_string(d,d->incomm);
		else if ( d->connected == CON_PLAYING )
		    interpret( d->character, d->incomm );
		else
		    nanny( d, d->incomm );

		d->incomm[0]	= '\0';
	    }
	    else
	      if (d->connected == CON_RESOLVE_ADDRESS)
		nanny (d, d->incomm);
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( d->wait > 0 )
	    {
		--d->wait;
		continue;
	    }
	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, d->fcommand ) )
		{
		    if ( d->character != NULL)
		      save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit (STATUS_ERROR);
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void new_descriptor( int control )
{
    static DESCRIPTOR_DATA d_zero;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    BAN_DATA *pban;
    struct sockaddr_in sock;
    int desc;
    int size;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    if ( descriptor_free == NULL )
    {
	dnew		= alloc_perm( sizeof(*dnew) );
    }
    else
    {
	dnew		= descriptor_free;
	descriptor_free	= descriptor_free->next;
    }

    *dnew		= d_zero;
    dnew->descriptor	= desc;
    dnew->connected	= CON_RESOLVE_ADDRESS;
    dnew->showstr_head	= NULL;
    dnew->showstr_point = NULL;
    dnew->outsize	= 2000;
    dnew->outbuf	= alloc_mem( dnew->outsize );
    dnew->adm_data      = NULL;
    dnew->wait          = 0;
    dnew->character     = 0;
    dnew->max_out	= 5000;
    dnew->next		= descriptor_list;
    descriptor_list	= dnew;

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
      sbug ("getpeername");
    }
    else
      {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	static int count = 1000;

	sprintf (dnew->host, "%d.resolve", count);
	sprintf (buf, "rm %s", dnew->host);
	system (buf);
	if (!fork ())
	  {
	    int addr;
	    struct hostent *from = NULL;
	    FILE *fl;
	    
	    addr = ntohl( sock.sin_addr.s_addr );
	    sprintf( buf, "%d.%d.%d.%d",
		     ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
		     ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
		     );
	    sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	    log_string( log_buf );

	    from = gethostbyaddr((char*)&sock.sin_addr, sizeof (sock.sin_addr),
				 AF_INET );
	    fl = fopen ("temp.resolve", "w");
	    if (from)
	      {
		log_string ("writing address %s to file %s", from->h_name,
			    dnew->host);
		fprintf (fl, "%s\n", from->h_name);
	      }
	    else
	      {
		log_string ("writing address %s to file %s", buf, dnew->host);
		fprintf (fl, "%s\n", buf);
	      }
	    fclose (fl);
	    /* if we just wrote directly to the file there could be a race
	       condition where we have created the file but not written the
	       address yet */
	    sprintf (buf, "mv temp.resolve %s", dnew->host);
	    system (buf);
	    exit (1);
	  }
	count++;
	if (count > 9999)
	  count = 1000;
      }
}

#endif



void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    log_string ("closing: connected = %d, host = %s", dclose->connected,
		dclose->host);
    if ((dclose->connected == CON_RESOLVE_ADDRESS) &&
	!str_prefix ("resolve.", dclose->host))
      {
	char buf[100];

	sprintf (buf, "rm %s", dclose->host);
	log_string ("removing resolve file '%s' on close of socket.",
		    dclose->host);
	system (buf);
      }
    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "Closing link to %s.", ch->names );
	log_string( log_buf );
	if ( dclose->connected == CON_PLAYING )
	{
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    ch->desc = NULL;
	    if (!IS_IMMORTAL (ch))
	      ch->ld_timer = LD_TICKS;
	    if ((ch->in_room->level < 0) && !IS_IMMORTAL (ch) && 
		!ch->in_room->interior_of)
	      {
		log_string ("Transporting the character.");
		do_teleport (ch, "");
	      }
	}
	else
	{
	    free_char( dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    /* RT socket leak fix -- I hope */
    free_mem(dclose->outbuf,dclose->outsize);
/*    free_string(dclose->showstr_head); */
    dclose->next	= descriptor_free;
    descriptor_free	= dclose;
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	return FALSE;
    }

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
/*
    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 25 )
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
		strcpy( d->incomm, "quit" );
	    }
	}
    }
*/

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{

  /* we do the descriptor writes twice now to give ourselves an empty buffer
     to put a prompt in, in the case of truncated buffer writes */
    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop != 0 )
      {
	/*
	 * Snoop-o-rama.
	 */
	if ( d->snoop_by != NULL )
	  {
	    if (d->character != NULL)
	      write_to_buffer( d->snoop_by, d->character->names,0);
	    write_to_buffer( d->snoop_by, "> ", 2 );
	    write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
	  }
	
	/*
	 * OS-dependent output.
	 */
	if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
	  {
	    d->outtop = 0;
	    return FALSE;
	  }
	else
	  d->outtop = 0;
      }
    /*
     * Bust a prompt.
     */
    if (!ground0_down && d->showstr_point)
	write_to_buffer(d,"\n\r[Hit Return to continue]\n\r",0);
    else if (fPrompt && !ground0_down && d->connected == CON_PLAYING)
      {
   	CHAR_DATA *ch;
	CHAR_DATA *victim;

	ch = d->character;

	if (!ch)
	  {
	    do_immtalk (NULL, "%%NULL character in descriptor detected."
			"Closing link.");
	    return FALSE;
	  }	    

	ch = d->original ? d->original : d->character;
	if (!IS_SET(ch->comm, COMM_COMPACT) )
	    write_to_buffer( d, "\n\r", 2 );

	if ( IS_SET(ch->comm, COMM_PROMPT) )
	{
	    char buf[40];

	    ch = d->character;
	    if (ch->in_room->inside_mob)
	      sprintf( buf, "`H%d/%d``hp", ch->in_room->inside_mob->hit,
		       ch->in_room->inside_mob->max_hit);
	    else
	      sprintf( buf, "`H%d/%d``hp", ch->hit,ch->max_hit);
	    if( ch->invis_level)
	      {
		char buf2[10];
		sprintf( buf2, " (`w%d``)" , ch->invis_level );
		strcat( buf, buf2 );
	      }
	    strcat( buf, "\r\n>");
	    send_to_char( buf, ch ); 
	  }

	if (IS_SET(ch->comm,COMM_TELNET_GA))
	    write_to_buffer(d,go_ahead_str,0);
      }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->names,0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
}


#define CNUM(x) ch->pcdata->x 
void process_color(CHAR_DATA *ch, char a) {
  int c = 0;
  char send_it_anyway[2];
  switch(a) {
  case '`': /* off color */
    c=0xf;
    break;
  case 'A':  /* combat melee opponent */
    c=CNUM(color_combat_o);
    break;
  case 'a': /* action */
    c=CNUM(color_action);
    break;
  case 'w': /* wizi mobs */
    c=CNUM(color_wizi);
    break;
  case 'X': /* invis mobs */
    c=CNUM(color_xy);
    break;
  case 'L': /* hidden mobs */
    c=CNUM(color_level);
    break;
  case 'H': /* hp */
    c=CNUM(color_hp);
    break;
  case 'D': /*Room Desc text */
    c=CNUM(color_desc);
    break;
  case 'O': /* objects */
    c=CNUM(color_obj);
    break;
  case 's': /* say */
    c=CNUM(color_say);
    break;
  case 't': /* tell */
    c=CNUM(color_tell);
    break;
  case 'r': /* reply */
    c=CNUM(color_reply);
    break;
  case 'E': /*exits */
    c=CNUM(color_exits);
    break;
  case 'C': /*condition color self */
    c=CNUM(color_combat_condition_s);
    break;
  case 'c': /* condition opponent */
    c=CNUM(color_combat_condition_o);
    break;
  case '1': /* 'blood' (bright red) */
    c=8;
    break;
  case '2':  /* Dark red */
    c=0;
    break;
  case '3': /* dark green */
    c=1;
    break;
  case '4': /* brown */
    c=2;
    break;
  case '5':  /* Dark Blue  */
    c=3;
    break;
  case '6': /* Dark Purple */
    c=4;
    break;
  case '7': /* Cyan */
    c=5;
    break;			
  case '8':  /* Bright Gray */
    c=6;
    break;
  case '9': /* Bright Black */
    c=7;
    break;
  case '0': /* Bright Green */
    c=9;
    break;
  case '!':  /* Yellow */
    c=10;
    break;
  case '@': /* Bright Blue */
    c=11;
    break;
  case '#': /* Bright Purple */
    c=12;
    break;			
  case '$':  /* Bright Cyan */
    c=13;
    break;
  case '%':  /* White */
    c=14;
    break;
  default: /* unknown ignore */
    sprintf (send_it_anyway, "%c", a);
    if (!IS_NPC(ch)) write_to_buffer(ch->desc,send_it_anyway, 1);
    return;
  }
  if (!IS_NPC(ch)) write_to_buffer(ch->desc,color_table[c], 
				   strlen(color_table[c]));
}

void send_to_char ( const char *txt, CHAR_DATA * ch )
{   
char *a,*b;
int length, l,curlen=0;
a=txt;
length = strlen(txt);

if ( txt != NULL && ch->desc != NULL ) {
     while(curlen<length) {
     	b=a;
     	l=0;
        while(curlen<length && *a!='`') {
   	     l++;
   	     curlen++;
   	     a++;
   	}
        if (l)	write_to_buffer(ch->desc,b,l);
        if (*a) {
        a++;
        curlen++;
        if (curlen<length && ch->color) {
        	process_color(ch, *a++);
        	curlen++;
        }
	else{
	a++;
	curlen++;
	}
        }
     }
 }

}

/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
  if (length <= 0)
    length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand && (d->connected != CON_GET_NAME))
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    if (!ground0_down)
      {
	if (d->outtop + 1 > d->max_out)
	  return;
	if (d->outtop+length > d->max_out)
	  length = d->max_out - d->outtop - 1;
      }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

        if (d->outsize >= 32000)
	{
	  if (d->outtop+1 == d->outsize)
	    return;
	  bug("Buffer overflow. Truncating.\n\r",0);
	  length = d->outsize - d->outtop - 1;
	  break;
 	}
	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
    strncpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    } 

    return TRUE;
}

GOD_TYPE *get_god (char *a_name)
{
  sh_int count;
  char *the_first, the_second[MAX_STRING_LENGTH];

  the_first = capitalize (a_name);

  for (count = 0; the_first[count];count++)
    the_second[count] = the_first[count];
  the_second[count] = 0;
  for (count = 0; imp_table[count].rl_name[0]; count++)
    if (!str_cmp (the_second, capitalize (imp_table[count].rl_name)))
      return &imp_table[count];
  for (count = 0; god_table[count].rl_name[0]; count++)
    if (!str_cmp (the_second, capitalize (god_table[count].rl_name)))
      return &god_table[count];
  return NULL;
}

/* ok, so who exactly runs this place? (this is hard coded instead of being
   thrown into a boot file for security reasons*/
const struct god_type imp_table [] =
{
  {"someimp", 10, "someplayer", "somepass", "god-someimp", 0},
  {"",0,"","",""}
};

ACCOUNT_DATA *account_info (char *account_name)
{
  ACCOUNT_DATA *tracker;

  for (tracker = accounts_list; tracker; tracker = tracker->next)
    if (!str_cmp (account_name, tracker->login))
      return tracker;
  return NULL;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    DESCRIPTOR_DATA *d_old, *d_next;
    BAN_DATA *pban;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    char *pwdnew;
    char *p;
    int iClass,race,i,notes;
    NOTE_DATA *pnote;
    bool fOld;
    OBJ_DATA *obj;
    sh_int count;

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_RESOLVE_ADDRESS:
      {
	FILE *fl;
	int count = 0;
	int dummy;

	if ((fl = fopen (d->host, "r")) == NULL)
	  {
	    write_to_descriptor (d->descriptor, ".", 0);
	    break;
	  }

	waitpid (d->host_lookup_pid, &dummy, WNOHANG);

	log_string ("reading file %s", d->host);
	sprintf (buf, "rm %s", d->host);
	d->host[0] = fgetc (fl);

	while ((d->host[++count] = fgetc(fl)) != '\n')
	  ;
	d->host[count] = '\0';
	log_string ("host is %s", d->host);
	fclose (fl);
	system (buf);

	/*
	 * Swiftest: I added the following to ban sites.  I don't
	 * endorse banning of sites, but Copper has few descriptors now
	 * and some people from certain sites keep abusing access by
	 * using automated 'autodialers' and leaving connections hanging.
	 *
	 * Furey: added suffix check by request of Nickel of HiddenWorlds.
	 */
	for ( pban = ban_list; pban != NULL; pban = pban->next )
	  {
	    if ( !str_suffix( pban->name, d->host ) ||
		 !str_prefix( pban->name, d->host ))
	      {
		write_to_buffer(d,"Your site has been banned from this "
				"game.\n\rThe reason(s) for this "
				"is/are:\n\r", 0);
		if (pban->reason)
		  write_to_buffer( d, pban->reason, 0);
		write_to_buffer(d, "\n\rYou may mail chewy@rice.edu if "
				    "you need clarification.\n\r", 0);
		sprintf (log_buf, "BANNING %s.", d->host);
		log_string (log_buf);
		close_socket (d);
		return;
		/*		close( desc );
		free_mem( d->outbuf, d->outsize );
		d->next		= descriptor_free;
		descriptor_free	= d;*/
	      }
	  }

	sprintf (buf, "\n\rVery few games allow you to blow "
		 "people up.\n\rHere is one that does.\n\r\n\r"
		 "Ground ZERO started up at %s\n\r"
		 "The system time is %s\n\r"
		 "Show color intro? (y/n) : ", str_boot_time,
		 (char *) ctime( &current_time ));
	write_to_descriptor (d->descriptor, buf, 0);
	d->connected = CON_GET_ANSI;
	break;
      }

    case CON_GET_ANSI:
	if ((argument[0] != '\0') && (argument[0] != 'n') && 
	    (argument[0] != 'N'))
	  {
	    extern char *help_greeting1, *help_greeting2a, *help_greeting2b;
	    char *the_greetinga, *the_greetingb;

	    if (number_range (0, 1))
	      {
		the_greetinga = help_greeting1;
		the_greetingb = NULL;
	      }
	    else
	      {
		the_greetinga = help_greeting2a;
		the_greetingb = help_greeting2b;
	      }
	    
	    write_to_descriptor(d->descriptor, the_greetinga, 0);
	    if (the_greetingb)
	      {
		DESC_WAIT_STATE (d, 8);
		write_to_buffer(d, the_greetingb, 0);
	      }
	  }

	write_to_buffer(d, "               Original DikuMUD by Hans "
			"Staerfeldt, Katja Nyboe,\n\r               Tom "
			"Madsen, Michael Seifert, and Sebastian Hammer\n\r"
			"               Based on MERC 2.1 code by Hatchet,"
			"Furey, and Kahn\n\r               ROM 2.3 code "
			"written by Russ Taylor, copyright 1993-1994\n\r               Ground ZERO base "
			"by Cory Hilke.\n\r\n\r"
			"[1;33mWelcome soldier.[0m\n\rPlease state "
			"your name [1;31mclearly[0m : ", 0);
	d->connected = CON_GET_NAME;
	break;
	  
    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	log_string ("name enterred:");
	log_string (argument);
	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    return;
	}

	if (!d->adm_data && (d->adm_data = get_god (argument)))
	    {
	      write_to_buffer(d, "Password: ", 0);
	      write_to_buffer( d, echo_off_str, 0 );
	      d->connected = CON_GET_ADMIN_PASS;
	      return;
	    }

	if ((ch = load_char_obj( d, argument )) == NULL)
	  fOld = FALSE;
	else
	  fOld = TRUE;

	d->character->trust = 0;
	if (d->adm_data) 
	  if (is_name (argument, d->adm_data->game_names))
	    {
	      log_string ("Trusting the admin.");
	      d->character->trust = d->adm_data->trust;
	      d->character->kills = 0;
	      d->character->hit = HIT_POINTS_IMMORTAL;
	      d->character->max_hit = HIT_POINTS_IMMORTAL;
	    }
	  else
	    {
	      write_to_buffer (d, "You are not authorized to play that "
			       "character.\n\rName your character: ", 0);
	      d->connected = CON_GET_NAME;
	      return;
	    }
	
	if ( ch && IS_SET(ch->act, PLR_DENY) )
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if (fOld && ch->pcdata->account)
	  {
	    ACCOUNT_DATA *accnt;
	    char *pwd;

	    if ((accnt = account_info (ch->pcdata->account)) == NULL)
	      {
		write_to_buffer (d, "It looks like your pfile is corrupted.  "
				 "Please report this message to an "
				 "administrator.\n\r", 0);
		close_socket (d);
		return;
	      }
	    else
	      {
		pwd = accnt->password;
		if ((pwd[0] == '*') && !d->adm_data)
		  {
		    write_to_buffer (d, "Your account has been disabled.\n\r",
				     0);
		    close_socket (d);
		    return;
		  }
	      }
	  }

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_HERO(ch)) 
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
 	    if (newlock)
	    {
                write_to_buffer( d, "The game is newlocked.\n\r", 0 );
                close_socket( d );
                return;
            }
	    for ( pban = nban_list; pban != NULL; pban = pban->next )
	      {
		if ( !str_suffix( pban->name, d->host ) )
		  {
		    write_to_buffer(d,"Your site has lost the privaledge of "
				    "having the ability to create new "
				    "characters here.\n\rThe reason(s) for "
				    "this is/are:\n\r",	0);
		    if (pban->reason)
		      write_to_buffer(d, pban->reason, 0);
		    write_to_buffer(d, "\n\rYou may mail chewy@rice.edu "
				    "if you need clarification.\n\r", 0);
		    sprintf (log_buf, "NEWBIE BANNING %s.", pban->name);
		    log_string (log_buf);
		    close_socket( d );
		    return;
		  }
	      }

	    sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_ADMIN_PASS:

	write_to_buffer( d, "\n\r", 2 );
	if ( strcmp( crypt( argument, d->adm_data->password), 
		    d->adm_data->password )) 
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    sprintf (log_buf, "Admin %s failed to enter a valid pass.", 
		     d->adm_data->rl_name);
	    log_string (log_buf);
	    close_socket( d );
	    return;
	}
 
	write_to_buffer( d, echo_on_str, 0 );

	sprintf (log_buf, "Admin %s has connected.", d->adm_data->rl_name);
	log_string (log_buf);

	write_to_buffer (d, "What character will you be? ", 0);
	d->connected = CON_GET_NAME;
	break;

    case CON_GET_OLD_PASSWORD:

	write_to_buffer( d, "\n\r", 2 );
	if ( strcmp( crypt( argument, ch->pcdata->password ),
		     ch->pcdata->password )) 
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}
 
	if (!ch->pcdata->password || !ch->pcdata->password[0])
	{
	    write_to_buffer( d, "Warning! Null password!\n\r",0 );
	    write_to_buffer( d, "Please report old password with bug.\n\r",0);
	    write_to_buffer( d, 
		"Type 'password null <new password>' to fix.\n\r",0);
	}

	write_to_buffer( d, echo_on_str, 0 );

	if (d->adm_data)
	  {
	    write_to_buffer (d, "Account is irrelevant.\n\r", 0);
	    if ( check_reconnect( d, ch->names, TRUE ) )
	      return;

	    if ( check_playing( d, ch->names ) )
	      return;

	    do_help (ch, "imotd");
	    d->connected = CON_READ_IMOTD;
	    return;
	  }

	if (ch->pcdata->account)
	  {
	    sprintf (buf, "Account: %s\n\rIf you enjoy this game, try to "
		     "get just ONE person you know to start playing\n\rhere "
		     "regularly.  If everyone does this, we could be "
		     "huge!\n\rPassword: %s", ch->pcdata->account,
		     echo_off_str);
	    write_to_buffer (d, buf, 0);
	    d->connected = CON_GET_ACCNT_PASS;
	    return;
	  }
	else
	  {
	    write_to_buffer (d, "We have an account system here.\n\rYou "
			     "must request an account by e-mailing chewy"
			     "@owlnet.rice.edu with a\n\rpassword "
			     "that you want to use to access the "
			     "account.\n\rIf you have requested an account "
			     "and would like to add this character to "
			     "that\n\raccount, enter the name of "
			     "the account now.  Otherwise you may press "
			     "return\n\rto enter as a guest (this will "
			     "allow you to get a flavor for the game "
			     "before\n\rrequesting an account, but you will "
			     "be limitted in some of the things you "
			     "can\n\rdo in the game.)\n\rAccount name: ",
			     0);
	    d->connected = CON_GET_NEW_ACCOUNT;
	    return;
	  }
	break;
    case CON_GET_ACCNT_PASS_ADDING:
    case CON_GET_ACCNT_PASS:
      {
	ACCOUNT_DATA *accnt;
	char *pwd;

	write_to_buffer( d, "\n\r", 2 );
	one_argument (argument, arg);
	accnt = account_info (ch->pcdata->account);
	pwd = accnt->password;
	if ( strcmp( crypt( arg, pwd), pwd )) 
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );

	if (d->connected == CON_GET_ACCNT_PASS_ADDING)
	  {
	    write_to_buffer (d, "Account information updated.\n\r", 0);
	    do_save (ch, "");
	  }

	if (accnt->character && str_cmp (accnt->character, ch->names))
	  {
	    write_to_buffer (d, "You are only allowed to play one character "
			     "per war.\n\r", 0);
	    close_socket (d);
	    return;
	  }

	if (!accnt->character)
	  accnt->character = str_dup (ch->names);

	if ( check_reconnect( d, ch->names, TRUE ) )
	    return;

	if ( check_playing( d, ch->names ) )
	    return;
		    
	sprintf( log_buf, "%s@%s has connected.", ch->names, d->host );
	log_string( log_buf );
	if ( IS_HERO(ch) )
	  {
	    do_help (ch, "imotd");
	    d->connected = CON_READ_IMOTD;
	  }
	else
	  {
	    do_help (ch, "motd");
	    d->connected = CON_READ_MOTD;
	  }
	break;
      }
    case CON_GET_NEW_ACCOUNT:
      argument = one_argument (argument, arg);
      if (!arg[0])
	{
	  if ( check_reconnect( d, ch->names, TRUE ) )
	    return;

	  if ( check_playing( d, ch->names ) )
	    return;
		    
	  sprintf( log_buf, "%s@%s has connected.", ch->names, d->host );
	  log_string( log_buf );
	  do_help (ch, "motd");
	  d->connected = CON_READ_MOTD;
	  return;
	}
      else
	{
	  if (account_info (arg))
	    {
	      ch->pcdata->account = str_dup(arg);
	      sprintf (buf, "password: %s", echo_off_str);
	      write_to_buffer (d, buf, 0);
	      d->connected = CON_GET_ACCNT_PASS_ADDING;
	      return;
	    }
	  else
	    {
	      write_to_buffer (d, "That account does not exist.\n\rAccount "
			       "name: ", 0);
	      return;
	    }
	}
      break;


/* RT code for breaking link */
 
    case CON_BREAK_CONNECT:
	switch( *argument )
	{
	case 'y' : case 'Y':
            for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
	    {
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
		    continue;

		if (str_cmp(ch->names,d_old->character->names))
		    continue;

		close_socket(d_old);
	    }
	    if (check_reconnect(d,ch->names,TRUE))
	    	return;
	    write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Name: ",0);
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"Please type Y or N? ",0);
	    break;
	}
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "New character.\n\rGive me a password for %s: %s",
		ch->names, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->names );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string (ch->pcdata->password);
	ch->pcdata->password = str_dup (pwdnew);
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->password ),
		     ch->pcdata->password ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );
        write_to_buffer( d, "What is your sex (M/F)? ", 0 );
        d->connected = CON_GET_NEW_SEX;
        break;
        

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    
			    ch->sex = SEX_MALE;
                            ch->sex = SEX_MALE;
			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
			    ch->sex = SEX_FEMALE;
                            ch->sex = SEX_FEMALE;
			    break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    return;
	}
	sprintf( log_buf, "%s@%s has connected.", ch->names, d->host );
	log_string( log_buf );
	do_help (ch, "motd");

	sprintf( buf, "is `!FRESH MEAT``");
	set_title( ch, buf );
	ch->kills = 0;
	
	d->connected = CON_READ_MOTD;
	do_save (ch, "");
	break;

    case CON_READ_IMOTD:
	write_to_buffer(d,"\n\r",2);
	do_help (ch, "motd");
        d->connected = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:
	ch->next	= char_list;
	char_list	= ch;
	d->connected	= CON_PLAYING;

	ch->ld_timer = 0;

	if (!ch->trust)
	  ch->hit = ch->max_hit = HIT_POINTS_MORTAL;

	do_outfit(ch,"");

	send_to_char("\n\r",ch);

	REMOVE_BIT (ch->affected_by, AFF_BLIND); /* in case they were still 
						    blind from previous game */
	char_to_room( ch, safe_area);
	if (d->adm_data && find_location (ch, d->adm_data->room_name))
	  {
	    char_from_room (ch);
	    char_to_room (ch, find_location (ch, d->adm_data->room_name));
	  }

	act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );

	/* check notes */
	notes = 0;

	for ( pnote = note_list; pnote != NULL; pnote = pnote->next)
	    if (is_note_to(ch,pnote) && str_cmp(ch->names,pnote->sender)
	    &&  pnote->date_stamp > ch->last_note)
	     	notes++;
	
	if (notes == 1)
	    send_to_char("\n\rYou have one new note waiting.\n\r",ch);

	else if (notes > 1)
	{
	    sprintf(buf,"\n\rYou have %d new notes waiting.\n\r",notes);
	    send_to_char(buf,ch);
	}

	recalc_num_on ();
	
	break;
    }

    return;
}

/* this should be called whenever a character begins playing where they weren't
   before (ie connected becomes con_playing) */
void recalc_num_on ()
{
  DESCRIPTOR_DATA *d;
  int i = 0;

  for ( d = descriptor_list; d != NULL; d = d->next )
    if (d->connected == CON_PLAYING)
      if (!IS_IMMORTAL (d->character))
	i++;
  
  if ((i%18 == 0) && (i > max_on))
    {
      CHAR_DATA *all_chars;
      char buf[MAX_STRING_LENGTH];
      
      sprintf (buf, "%d players on!!  Parachutes fill the air as new "
	       "supplies are dropped from cargo planes above.\n\rScouts "
	       "discover new parts of the city that have yet to be "
	       "explored.\n\r", i);
      for (all_chars = char_list; all_chars; all_chars = all_chars->next)
	send_to_char (buf, all_chars);
      
      expand_city();
      expansions++;
      scatter_objects ();
    }
  
  max_on = UMAX(i,max_on);
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    /*
     * Reserved words.
     */

  if (!str_prefix ("self", name))
	return FALSE;

  if ( is_name( name, "red blue bounty all auto immortal self someone "
	       "enforcer something the you god safe list out" ) )
	return FALSE;

    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
	return FALSE;

    if (strlen (name) > 13)
	return FALSE;

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    return TRUE;
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->names, ch->names ) )
	{
	    if ( fConn == FALSE )
	    {
                free_string( d->character->pcdata->password );
                d->character->pcdata->password =
		  str_dup( ch->pcdata->password );
	    }
	    else
	    {
	      if ((!ch->pcdata->account || !ch->pcdata->account[0]) &&
		  d->character->pcdata->account &&
		  d->character->pcdata->account[0])
		ch->pcdata->account = str_dup (d->character->pcdata->account);

	      free_char( d->character );

		d->character = ch;
		
		if (d->adm_data)
		  {
		    d->character->trust = d->adm_data->trust;
		    d->character->kills = 0;
		    d->character->max_hit = HIT_POINTS_IMMORTAL;
		  }
		else
		  {
		    d->character->trust = 0;
		    ch->max_hit = HIT_POINTS_MORTAL;
		  }

		ch->desc	 = d;
		ch->timer	 = 0;
		send_to_char( "Reconnecting.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		sprintf( log_buf, "%s@%s reconnected.", ch->names, d->host );
		log_string( log_buf );
		d->connected = CON_PLAYING;
		ch->ld_timer = 0;
		recalc_num_on ();
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->names : dold->character->names ) )
	{
	    write_to_buffer( d, "That character is already playing.\n\r",0);
	    write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}

/*
 * Write to one char   TJT.
 */
/*   void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
        write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}
*/
/*
 * Send a page to one char.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
  char *tmp_head, *tmp_point;
  if ( txt == NULL || ch->desc == NULL)
    return;
	
  if (ch->desc->showstr_head != NULL)
    {
      tmp_head = ch->desc->showstr_head;
      tmp_point = ch->desc->showstr_point;
      ch->desc->showstr_head = alloc_mem(strlen(txt) +
					 strlen(ch->desc->showstr_point) + 1);
      strcpy (ch->desc->showstr_head, tmp_point);
      strcpy (ch->desc->showstr_head + strlen(tmp_point), txt);
      ch->desc->showstr_point = ch->desc->showstr_head;
      free_string (tmp_head);
    }
  else
    {
      ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
      strcpy(ch->desc->showstr_head,txt);
      ch->desc->showstr_point = ch->desc->showstr_head;
    }
}


/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[4*MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
	if (d->showstr_head)
	{
	    free_string(d->showstr_head);
	    d->showstr_head = 0;
	}
    	d->showstr_point  = 0;
	return;
    }

    if (d->character)
	show_lines = d->character->lines;
    else
	show_lines = 0;

    for (scan = buffer; ; scan++, d->showstr_point++)
    {
	if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
	    && (toggle = -toggle) < 0)
	    lines++;

	else if (!*scan || (show_lines > 0 && lines >= show_lines))
	{
	    *scan = '\0';
	    if (d->connected == CON_PLAYING)
	      send_to_char(buffer, d->character);
	    else
	      write_to_buffer (d, buffer, 0);
	    for (chk = d->showstr_point; isspace(*chk); chk++);
	    {
		if (!*chk)
		{
		    if (d->showstr_head)
        	    {
            		free_string(d->showstr_head);
            		d->showstr_head = 0;
        	    }
        	    d->showstr_point  = 0;
    		}
	    }
	    return;
	}
    }
    return;
}
	

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->sex;
}

void act (const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
	  int type)
{
    /* to be compatible with older code */
    act_new(format,ch,arg1,arg2,type,POS_RESTING);
}

void act_new( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type, int min_pos)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    char i[MAX_INPUT_LENGTH];
    char *point, *temp_i;
 
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
        if ( vch == NULL )
        {
            bug( "Act: null vch with TO_VICT.", 0 );
            return;
        }

	if (vch->in_room == NULL)
	    return;

        to = vch->in_room->people;
    }
 
    for ( ; to != NULL; to = to->next_in_room )
    {
        if ( to->desc == NULL || to->position < min_pos )
            continue;
 
        if ( type == TO_CHAR && to != ch )
            continue;
        if ( type == TO_VICT && ( to != vch || to == ch ) )
            continue;
        if ( type == TO_ROOM && to == ch )
            continue;
        if ( type == TO_NOTVICT && (to == ch || to == vch) )
            continue;
 
        point   = buf;
        str     = format;
        while ( *str != '\0' )
        {
            if ( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }
            ++str;
 
            if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
            {
	      sprintf (log_buf, "Act: missing arg2 for code %d (format = %s).",
		       *str, format);
	      sbug (log_buf);
	      sprintf (i, " <@@@> ");
            }
            else
            {
                switch ( *str )
                {
                default:  bug( "Act: bad code %d.", *str );
                          sprintf (i, " <@@@> ");                       break;
                /* Thx alex for 't' idea */
                case 't': sprintf (i, "%s", (char *) arg1);             break;
                case 'T': sprintf (i, "%s", (char *) arg2);             break;
                case 'n': sprintf (i, "`A%s``", PERS( ch,  to  ));       break;
                case 'N': sprintf (i, "`A%s``", PERS( vch,  to  ));      break;
                case 'e': sprintf (i, "%s", he_she  [URANGE(0, ch  ->sex, 2)]);
			  break;
                case 'E': sprintf (i, "%s", he_she  [URANGE(0, vch ->sex, 2)]);
			  break;
                case 'm': sprintf (i, "%s", him_her [URANGE(0, ch  ->sex, 2)]);
			  break;
                case 'M': sprintf (i, "%s", him_her [URANGE(0, vch ->sex, 2)]);
			  break;
                case 's': sprintf (i, "%s", his_her [URANGE(0, ch  ->sex, 2)]);
			  break;
                case 'S': sprintf (i, "%s", his_her [URANGE(0, vch ->sex, 2)]);
			  break;
 
                case 'p':
                    sprintf (i,"`O%s``", obj1->short_descr);
                    break;
 
                case 'P':
                    sprintf (i, "`O%s``", obj2->short_descr);
                    break;
 
                case 'd':
                    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
                    {
                        sprintf (i, "door");
                    }
                    else
                    {
                        one_argument( (char *) arg2, fname );
                        sprintf (i, "%s", fname);
                    }
                    break;
                }
            }
 
            ++str;
	    temp_i = i;
            while ( ( *point = *temp_i ) != '\0' )
                ++point, ++temp_i;
        }
 
        *point++ = '\n';
        *point++ = '\r';
	*point++ = '\0';
        buf[0]   = UPPER(buf[0]);
        send_to_char( buf, to );
    }
 
    return;
}
