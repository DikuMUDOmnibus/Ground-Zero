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
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ground0.h"

extern char *guild_list[];

/* command procedures needed */
DECLARE_DO_FUN(do_quit	);



/*
 * Local functions.
 */
void	note_attach	args( ( CHAR_DATA *ch ) );
void	note_remove	args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void	note_delete	args( ( NOTE_DATA *pnote ) );



bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    if ( !str_cmp( ch->names, pnote->sender ) )
	return TRUE;

    if ( is_name_exact( "all", pnote->to_list ) )
	return TRUE;

    if ( IS_HERO(ch) && (is_name( "immortal", pnote->to_list ) ||
			 is_name_exact ("imm", pnote->to_list)))
	return TRUE;

    if ( is_name_exact ( ch->names, pnote->to_list ) )
	return TRUE;

    if ( ch->trust == MAX_TRUST)
      return TRUE;

    return FALSE;
}



void note_attach( CHAR_DATA *ch )
{
    NOTE_DATA *pnote;

    if ( ch->pnote != NULL )
	return;

    if ( note_free == NULL )
    {
	pnote	  = alloc_perm( sizeof(*ch->pnote) );
    }
    else
    {
	pnote	  = note_free;
	note_free = note_free->next;
    }

    pnote->next		= NULL;
    pnote->sender	= str_dup( ch->names );
    pnote->date		= str_dup( "" );
    pnote->to_list	= str_dup( "" );
    pnote->subject	= str_dup( "" );
    pnote->text		= str_dup( "" );
    ch->pnote		= pnote;
    return;
}



void note_remove( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    char to_new[MAX_INPUT_LENGTH];
    char to_one[MAX_INPUT_LENGTH];
    FILE *fp;
    NOTE_DATA *prev;
    char *to_list;

    /*
     * Build a new to_list.
     * Strip out this recipient.
     */
    to_new[0]	= '\0';
    to_list	= pnote->to_list;
    while ( *to_list != '\0' )
    {
	to_list	= one_argument( to_list, to_one );
	if ( to_one[0] != '\0' && str_cmp( ch->names, to_one ) )
	{
	    strcat( to_new, " " );
	    strcat( to_new, to_one );
	}
    }

    /*
     * Just a simple recipient removal?
     */
    if ( str_cmp( ch->names, pnote->sender ) && to_new[0] != '\0' )
    {
	free_string( pnote->to_list );
	pnote->to_list = str_dup( to_new + 1 );
	return;
    }

    /*
     * Remove note from linked list.
     */
    if ( pnote == note_list )
    {
	note_list = pnote->next;
    }
    else
    {
	for ( prev = note_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pnote )
		break;
	}

	if ( prev == NULL )
	{
	    bug( "Note_remove: pnote not found.", 0 );
	    return;
	}

	prev->next = pnote->next;
    }

    free_string( pnote->text    );
    free_string( pnote->subject );
    free_string( pnote->to_list );
    free_string( pnote->date    );
    free_string( pnote->sender  );
    pnote->next	= note_free;
    note_free	= pnote;

    /*
     * Rewrite entire list.
     */
    fclose( fpReserve );
    if ( ( fp = fopen( NOTE_FILE, "w" ) ) == NULL )
    {
	perror( NOTE_FILE );
    }
    else
    {
	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	{
	    fprintf( fp, "Sender  %s~\n", pnote->sender);
	    fprintf( fp, "Date    %s~\n", pnote->date);
	    fprintf( fp, "Stamp   %d\n",  pnote->date_stamp);
	    fprintf( fp, "To      %s~\n", pnote->to_list);
	    fprintf( fp, "Subject %s~\n", pnote->subject);
	    fprintf( fp, "Text\n%s~\n",   pnote->text);
	}
	fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/* used by imps to nuke a note for good */
void note_delete( NOTE_DATA *pnote )
{
    FILE *fp;
    NOTE_DATA *prev;
 
    /*
     * Remove note from linked list.
     */
    if ( pnote == note_list )
    {
        note_list = pnote->next;
    }
    else
    {
        for ( prev = note_list; prev != NULL; prev = prev->next )
        {
            if ( prev->next == pnote )
                break;
        }
 
        if ( prev == NULL )
        {
            bug( "Note_delete: pnote not found.", 0 );
            return;
        }
 
        prev->next = pnote->next;
    }
 
    free_string( pnote->text    );
    free_string( pnote->subject );
    free_string( pnote->to_list );
    free_string( pnote->date    );
    free_string( pnote->sender  );
    pnote->next = note_free;
    note_free   = pnote;
 
    /*
     * Rewrite entire list.
     */
    fclose( fpReserve );
    if ( ( fp = fopen( NOTE_FILE, "w" ) ) == NULL )
    {
        perror( NOTE_FILE );
    }
    else
    {
        for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
        {
            fprintf( fp, "Sender  %s~\n", pnote->sender);
            fprintf( fp, "Date    %s~\n", pnote->date);
            fprintf( fp, "Stamp   %d\n",  pnote->date_stamp);
            fprintf( fp, "To      %s~\n", pnote->to_list);
            fprintf( fp, "Subject %s~\n", pnote->subject);
            fprintf( fp, "Text\n%s~\n",   pnote->text);
        }
        fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


void do_note( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    NOTE_DATA *pnote;
    int vnum;
    int anum;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg );
    smash_tilde( argument );

    if (arg[0] == '\0')
    {
	do_note(ch,"read");
	return;
    }

    if ((!ch->pcdata->account || !ch->pcdata->account[0]) && !IS_IMMORTAL (ch)
	&& str_prefix( arg, "list" )  && str_prefix (arg, "read"))
      {
	send_to_char ("You must be a registerred player to write notes.", ch);
	return;
      }

    if ( !str_prefix( arg, "list" ) )
    {
	vnum = 0;
	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) )
	    {
		sprintf( buf, "[%3d%s] %s: %s\n\r",
		    vnum, 
		    (pnote->date_stamp > ch->last_note 
		     && str_cmp(pnote->sender,ch->names)) ? "N" : " ",
		     pnote->sender, pnote->subject );
		send_to_char( buf, ch );
		vnum++;
	    }
	}
	return;
    }

    if ( !str_prefix( arg, "read" ) )
    {
	bool fAll;

	if ( !str_cmp( argument, "all" ) )
	{
	    fAll = TRUE;
	    anum = 0;
	}
	
	else if ( argument[0] == '\0' || !str_prefix(argument, "next"))
	/* read next unread note */
	{
	    vnum = 0;
	    for ( pnote = note_list; pnote != NULL; pnote = pnote->next) 
	    {
		if (is_note_to(ch,pnote) && str_cmp(ch->names,pnote->sender)
		&&  ch->last_note < pnote->date_stamp)
            	{
                    sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
                    	vnum,
                    	pnote->sender,
                    	pnote->subject,
                    	pnote->date,
                    	pnote->to_list);
                    send_to_char( buf, ch );
                    page_to_char( pnote->text, ch );
                    ch->last_note = UMAX(ch->last_note,pnote->date_stamp);
                    return;
		}
		else if (is_note_to(ch,pnote))
		    vnum++;
            }
	    send_to_char("You have no unread notes.\n\r",ch);	
	    return;
	}

	else if ( is_number( argument ) )
	{
	    fAll = FALSE;
	    anum = atoi( argument );
	}
	else
	{
	    send_to_char( "Note read which number?\n\r", ch );
	    return;
	}

	vnum = 0;
	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
	    {
		sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
		    vnum - 1,
		    pnote->sender,
		    pnote->subject,
		    pnote->date,
		    pnote->to_list
		    );
		send_to_char( buf, ch );
		send_to_char( pnote->text, ch );
		ch->last_note = UMAX(ch->last_note,pnote->date_stamp);
		return;
	    }
	}

	send_to_char( "No such note.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "+" ) )
    {
	note_attach( ch );
	strcpy( buf, ch->pnote->text );
	if ( strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 200 )
	{
	    send_to_char( "Note too long.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
	free_string( ch->pnote->text );
	ch->pnote->text = str_dup( buf );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "subject" ) )
    {
	note_attach( ch );
	free_string( ch->pnote->subject );
	ch->pnote->subject = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "to" ) )
    {
	note_attach( ch );
	free_string( ch->pnote->to_list );
	ch->pnote->to_list = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "clear" ) )
    {
	if ( ch->pnote != NULL )
	{
	    free_string( ch->pnote->text );
	    free_string( ch->pnote->subject );
	    free_string( ch->pnote->to_list );
	    free_string( ch->pnote->date );
	    free_string( ch->pnote->sender );
	    ch->pnote->next	= note_free;
	    note_free		= ch->pnote;
	    ch->pnote		= NULL;
	}

	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "show" ) )
    {
	if ( ch->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

	sprintf( buf, "%s: %s\n\rTo: %s\n\r",
	    ch->pnote->sender,
	    ch->pnote->subject,
	    ch->pnote->to_list
	    );
	send_to_char( buf, ch );
	send_to_char( ch->pnote->text, ch );
	return;
    }

    if ( !str_prefix( arg, "post" ) || !str_prefix(arg, "send"))
    {
	FILE *fp;
	char *strtime;

	if ( ch->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

	if (!str_cmp(ch->pnote->to_list,""))
	{
	    send_to_char(
		"You need to provide a recipient (name, all, or immortal).\n\r",
		ch);
	    return;
	}

	if (!str_cmp(ch->pnote->subject,""))
	{
	    send_to_char("You need to provide a subject.\n\r",ch);
	    return;
	}

	ch->pnote->next			= NULL;
	strtime				= ctime( &current_time );
	strtime[strlen(strtime)-1]	= '\0';
	ch->pnote->date			= str_dup( strtime );
	ch->pnote->date_stamp		= current_time;

	if ( note_list == NULL )
	{
	    note_list	= ch->pnote;
	}
	else
	{
	    for ( pnote = note_list; pnote->next != NULL; pnote = pnote->next )
		;
	    pnote->next	= ch->pnote;
	}
	pnote		= ch->pnote;
	ch->pnote	= NULL;

	fclose( fpReserve );
	if ( ( fp = fopen( NOTE_FILE, "a" ) ) == NULL )
	{
	    perror( NOTE_FILE );
	}
	else
	{
	    fprintf( fp, "Sender  %s~\n",	pnote->sender);
	    fprintf( fp, "Date    %s~\n", pnote->date);
	    fprintf( fp, "Stamp   %d\n", pnote->date_stamp);
	    fprintf( fp, "To	  %s~\n", pnote->to_list);
	    fprintf( fp, "Subject %s~\n", pnote->subject);
	    fprintf( fp, "Text\n%s~\n", pnote->text);
	    fclose( fp );
	}
	fpReserve = fopen( NULL_FILE, "r" );

	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "remove" ) )
    {
	if ( !is_number( argument ) )
	{
	    send_to_char( "Note remove which number?\n\r", ch );
	    return;
	}

	anum = atoi( argument );
	vnum = 0;
	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) && vnum++ == anum )
	    {
		note_remove( ch, pnote );
		send_to_char( "Ok.\n\r", ch );
		return;
	    }
	}

	send_to_char( "No such note.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "delete" ) && get_trust(ch))
    {
        if ( !is_number( argument ) )
        {
            send_to_char( "Note delete which number?\n\r", ch );
            return;
        }
 
        anum = atoi( argument );
        vnum = 0;
        for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
                note_delete( pnote );
                send_to_char( "Ok.\n\r", ch );
                return;
            }
        }
 
        send_to_char( "No such note.\n\r", ch );
        return;
    }

    send_to_char( "Huh?  Type 'help note' for usage.\n\r", ch );
    return;
}


/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];

   if (IS_NPC(ch))
	return;
  
   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
    	    sprintf( strsave, "mv -f %sPS%s %sPS%s", PLAYER_DIR, 
		    capitalize( ch->names ), BAK_PLAYER_DIR, 
		    capitalize( ch->names ) );
	    send_to_char ("See ya here next time you get the urge to kill.  "
			  "Grin.\n\r", ch);
	    if (ch->desc)
	      close_socket (ch->desc);
	    extract_char (ch, TRUE);
	    system(strsave);
	    do_save_all (NULL, "");
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
}

/* RT quiet blocks out all communication */
void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}

void do_kills( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  
  if (ch)
    if (IS_SET(ch->comm,COMM_NOKILLS))
      {
	send_to_char("You will now be notified of kills.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOKILLS);
      }
    else
      {
	send_to_char("You will no longer be notified of kills.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOKILLS);
      }
  else
    {
      for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	  if (!ground0_down && (IS_SET(victim->comm,COMM_NOKILLS) || 
				IS_SET(victim->comm,COMM_QUIET)))
	    continue;
	  send_to_char(argument, victim);
	}
    }
}

void do_bounty( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  
  if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOBOUNTY))
	{
	  send_to_char("`0Bounty`` Hunter channel is now ON.\n\r",ch);
	  REMOVE_BIT(ch->comm,COMM_NOBOUNTY);
	}
      else
	{
	  send_to_char("`0Bounty`` Hunter channel is now OFF.\n\r",ch);
	  SET_BIT(ch->comm,COMM_NOBOUNTY);
	}
    }
  else
    {
      if (ch)
	{
	  if (IS_SET(ch->comm,COMM_QUIET))
	    {
	      send_to_char("You must turn off quiet mode first.\n\r",ch);
	      return;
	    }
	  
	  if (IS_SET(ch->comm,COMM_NOCHANNELS))
	    {
	      send_to_char("The gods have revoked your channel priviliges."
			   "\n\r", ch);
	      return;
	    }
 
	  REMOVE_BIT(ch->comm,COMM_NOBOUNTY);
      
	  sprintf( buf, "You `0BOUNTY``: '`0%s``'\n\r", argument );
	  send_to_char( buf, ch );
	  sprintf( buf, "$n `0BOUNTY``: '`0%s``'", argument );
	}
      if (ch)
	if (ch->chan_delay)
	  {
	    WAIT_STATE (ch, ch->chan_delay);
	    ch->chan_delay *= 2;
	  }
	else
	  ch->chan_delay = PULSE_PER_SECOND;
      for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	  if (IS_SET(victim->comm,COMM_NOBOUNTY) || 
	      IS_SET(victim->comm,COMM_QUIET))
	    continue;
	  if (((argument[0] == '%') && ch && IS_IMMORTAL(ch)) || !ch)
	    act_new ("`AMR SELF-DESTRUCT`` `0BOUNTY``: '`0$t``'", victim,
		     &(argument[1]), NULL, TO_CHAR, POS_DEAD);
	  else
	    act_new("$n `0BOUNTY``: '`0$t``'", ch, argument, victim, TO_VICT,
		    POS_DEAD);
	}
    }
}

void do_imptalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
	send_to_char("You cannot turn off IMP talk.\n\r", ch );
	return;
    }

    sprintf( buf, "$n: %s", argument );
    act_new("IMP] $n: $t",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING && 
	     IS_IMP(d->character) ) 
	{
	    act_new("IMP] $n: $t",ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }

}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *listenners;

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOWIZ))
      {
	send_to_char("Immortal channel is now ON\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOWIZ);
      }
      else
      {
	send_to_char("Immortal channel is now OFF\n\r",ch);
	SET_BIT(ch->comm,COMM_NOWIZ);
      } 
      return;
    }

    if (ch)
      REMOVE_BIT(ch->comm,COMM_NOWIZ);

    sprintf( buf, "$n: %s", argument );
    if (ch)
      act_new("$n: $t",ch,argument,NULL,TO_CHAR,POS_DEAD);
    if (ch)
      if (ch->chan_delay)
	{
	  WAIT_STATE (ch, ch->chan_delay);
	  ch->chan_delay *= 2;
	}
      else
	ch->chan_delay = PULSE_PER_SECOND;
    for (listenners = char_list; listenners; listenners = listenners->next)
      {
	if (IS_HERO(listenners) && 
	    !IS_SET(listenners->comm,COMM_NOWIZ) )
	  if (ch)
	    act_new("$n: $t",ch,argument,listenners,TO_VICT,POS_DEAD);
	  else
	    act_new ("`AMR SELF-DESTRUCT``: $t", listenners, &(argument[1]), 
		     NULL, TO_CHAR, POS_DEAD);
      }

    return;
}

void do_say( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    act( "$n says '`s$T``'", ch, NULL, argument, TO_ROOM );
    act( "You say '`s$T``'", ch, NULL, argument, TO_CHAR );
    return;
}

void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
  
    if ( IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    if (ch->chan_delay)
      {
	WAIT_STATE (ch, ch->chan_delay);
	ch->chan_delay *= 2;
      }
    else
      ch->chan_delay = PULSE_PER_SECOND;
    act( "You tell $N '`t$t``'", ch, argument, victim, TO_CHAR );
    act_new("$n tells you '`t$t``'",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    return;
}

void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        act("$N seems to have misplaced $S link...try again later.",
            ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch))
    {
        act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
        return;
    }

    if (ch->chan_delay)
      {
	WAIT_STATE (ch, ch->chan_delay);
	ch->chan_delay *= 2;
      }
    else
      ch->chan_delay = PULSE_PER_SECOND;

    act("You reply to $N '`t$t``'",ch,argument,victim,TO_CHAR);
    act_new("$n replies '`t$t``'",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    return;
}

void do_emote( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You can't show your emotions.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Emote what?\n\r", ch );
	return;
    }

    act( "$n $T", ch, NULL, argument, TO_ROOM );
    act( "$n $T", ch, NULL, argument, TO_CHAR );
    return;
}

void do_idea( CHAR_DATA *ch, char *argument )
{
    append_file( ch, IDEA_FILE, argument );
    send_to_char( "Idea logged.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}

void do_quit ( CHAR_DATA *ch, char *argument)
{
  if (!IS_IMMORTAL (ch))
    {
      send_to_char ("No quitting here pal.  If you need to lose link, the "
		    "command is 'ld'.\n\r", ch);
      return;
    }
  save_char_obj (ch);
  if (ch->desc)
    close_socket (ch->desc);
  extract_char (ch, TRUE);
}

void do_lose_link (CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  if (ch->desc)
    for (d = descriptor_list; d; d = d->next)
      if (d == ch->desc)
	{
	  close_socket (d);
	  return;
	}
}

void do_save( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;
  
  save_char_obj( ch );
  send_to_char("You have been saved.\n\r", ch );
  WAIT_STATE(ch,5 * PULSE_VIOLENCE);
  return;
}

void die_follower( CHAR_DATA *ch )
{
  CHAR_DATA *fch;
  
  if (ch->leader)
    stop_follower( ch );
  
  for ( fch = char_list; fch != NULL; fch = fch->next )
    if ( fch->leader == ch )
      stop_follower( fch );
}

void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
      if (ch->allows_followers)
	{
	  send_to_char ("You no longer allow any followers.\n\r", ch);
	  ch->allows_followers = 0;
	  die_follower (ch);
	  return;
	}
      else
	{
	  send_to_char ("Members of your team can follow you now.\n\r", ch);
	  ch->allows_followers = 1;
	  return;
	}

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->leader == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if ( ch->leader != NULL )
	stop_follower( ch );

    if (victim->leader)
      {
	send_to_char ("Follow someone who isn't already following someone "
		      "else.\n\r", ch);
	return;
      }

    if (!victim->allows_followers)
      {
	act ("$N doesn't look like $E wants you to follow $M.", ch,
	     NULL, victim, TO_CHAR);
	act ("$n tries to follow you, but you aren't allowing followers.",
	     ch, NULL, victim, TO_VICT);
	return;
      }
      
    add_follower( ch, victim );
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *leader )
{
    if ( ch->leader != NULL )
    {
	bug( "Add_follower: non-null leader.", 0 );
	return;
    }

    ch->leader        = leader;

    if ( can_see( leader, ch ) )
	act( "$n now follows you.", ch, NULL, leader, TO_VICT );

    act( "You now follow $N.",  ch, NULL, leader, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( ch->leader == NULL )
    {
	bug( "Stop_follower: null leader.", 0 );
	return;
    }

    if ( can_see( ch->leader, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->leader, TO_VICT    );
    	act( "You stop following $N.",      ch, NULL, ch->leader, TO_CHAR    );
    }

    ch->leader = NULL;
    return;
}

void do_characters (CHAR_DATA *ch, char *argument)
{
  int num;

  if (IS_NPC(ch))
    return;

  if (!is_number (argument))
    {
      send_to_char ("Syntax: chars <number>\n\r", ch);
      return;
    }

  if (((num = atoi(argument)) < 500) || (num > 32000))
    {
      send_to_char ("The number must be in the range 500 - 32000.\n\r", ch);
      return;
    }

  ch->desc->max_out = num;
}
