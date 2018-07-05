/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
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


#include "status.h"

#define exit(a) if (fBootDb && ((a) == STATUS_ERROR)) \
                exit (STATUS_BOOT_ERROR); else exit (a)
#define is_digit(a) (((a) >= '0') && ((a) <= '9'))
#define sbug(a) bug (a,0)
/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	void fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#endif

/* system calls */
int unlink();
int system();



/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
/*typedef short   int			sh_int;*/
typedef int			        sh_int;
typedef unsigned char			bool;
#endif
typedef unsigned char			byte;


/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct	char_data		CHAR_DATA;
typedef struct  room_data               ROOM_DATA;
typedef struct  level_data              LEVEL_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct  social_type             SOCIAL_TYPE;
typedef struct  god_type                GOD_TYPE;
typedef struct  account_data		ACCOUNT_DATA;
typedef struct  top_data		TOP_DATA;


/*
 * Function types.
 */
typedef	void DO_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );



/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 4096
#define MAX_INPUT_LENGTH	  256
#define PAGELEN			   22

#define MAX_INT			32768

#define VALID_VALUE                82 /* some exact number to avoid accidental
                                         validation due to memory corruption */

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_SOCIALS		  256
#define MAX_IN_GROUP		   15
#define MAX_TRUST                  10
#define MAX_NUMBER_CARRY           15
#define HIT_POINTS_MORTAL        6666
#define HIT_POINTS_IMMORTAL   1000000
#define NUM_TOP			   10
#define LD_TICKS		   30

#define PULSE_PER_SECOND	    4
#define PULSE_OBJECTS             ( 1 * PULSE_PER_SECOND)
#define PULSE_VIOLENCE		  ( 1 * PULSE_PER_SECOND)
#define PULSE_TICK		  (10 * PULSE_PER_SECOND)
#define PULSE_SAVE                (200 * PULSE_PER_SECOND)

/*
 * Site ban structure.
 */
struct	ban_data
{
    BAN_DATA *	next;
    char 	name[MAX_INPUT_LENGTH];
    char        reason[MAX_INPUT_LENGTH];
};

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
};

/*
 * Connected state for a channel.
 */
#define CON_PLAYING			 0
#define CON_RESOLVE_ADDRESS		 1
#define CON_GET_ANSI                     2
#define CON_GET_NAME			 3
#define CON_GET_OLD_PASSWORD		 4
#define CON_GET_ACCNT_PASS		 5
#define CON_GET_ACCNT_PASS_ADDING	 6
#define CON_GET_NEW_ACCOUNT		 7
#define CON_CONFIRM_NEW_NAME		 8
#define CON_GET_ADMIN_PASS               9
#define CON_GET_NEW_PASSWORD		10
#define CON_CONFIRM_NEW_PASSWORD	11
#define CON_GET_ADMIN_CHAR_NAME         12
#define CON_GET_NEW_SEX			13
#define CON_DEFAULT_CHOICE		15 
#define CON_READ_IMOTD			16
#define CON_READ_MOTD			17
#define CON_BREAK_CONNECT		18

#define COUNT_WEAPONS    0
#define COUNT_ARMOR      1
#define COUNT_EXPLOSIVES 2
#define COUNT_AMMO	 3
#define COUNT_MISC	 4
#define NUM_COUNTS	 5

#define ITEM_MISC                       0
#define ITEM_WEAPON                     1
#define ITEM_ARMOR                      2
#define ITEM_EXPLOSIVE                  3
#define ITEM_AMMO                       4
#define ITEM_TEAM_VEHICLE               5
#define ITEM_TEAM_ENTRANCE              6

/* ammo types */
#define AMMO_9MM                        0
#define AMMO_GREN_LAUNCHER              6
#define AMMO_FLARE                      13
#define AMMO_TANK_GUN                   14

/* general flags */
#define GEN_CAN_BURY                   (B)
#define GEN_BURNS_ROOM                 (D)
#define GEN_DETECT_MINE                (G)
#define GEN_ANTI_BLIND                 (H)
#define GEN_DARKS_ROOM		       (J)
#define GEN_EXTRACT_ON_IMPACT	       (K)
#define GEN_SEE_IN_DARK		       (L)

/* extract flags */
#define EXTRACT_EXPLODE_ON_EXTRACT     (A)
#define EXTRACT_BLIND_ON_EXTRACT       (B)
#define EXTRACT_BURN_ON_EXTRACT        (C)
#define EXTRACT_STUN_ON_EXTRACT        (D)
#define EXTRACT_DARK_ON_EXTRACT	       (E)

/* usage flags */
#define USE_HEAL                       (A)
#define USE_EVAC                       (D)
#define USE_MAKE_DROID                 (E)
#define USE_MAKE_SEEKER_DROID          (F)

/* search types */
#define SEARCH_ITEM_TYPE    0
#define SEARCH_GEN_FLAG     1
#define SEARCH_AMMO_TYPE    2

/* some common objects */
#define VNUM_GRENADE                   1
#define VNUM_9MM_CLIP                  2
#define VNUM_9MM_PISTOL                3
#define VNUM_FIRE                      5
#define VNUM_NAPALM                    6
#define VNUM_HQ			       7
#define VNUM_DARK		       24

/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    GOD_TYPE           *adm_data;
    unsigned int	time;
    pid_t		host_lookup_pid;
    char		host[80];
    sh_int		descriptor;
    sh_int		connected;
    int                 wait;
    bool		fcommand;
    char		inbuf		[4 * MAX_INPUT_LENGTH];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int			repeat;
    char *		outbuf;
    int			outsize;
    int			outtop;
    int			max_out;
    char *		showstr_head;
    char *		showstr_point;
};

/* allocation validation structure */
struct allocations_type
{
  void *ptr;
  int size;
  char *filename;
  int line_num;
  struct allocations_type *next;
};

struct god_type
{
  char                 *rl_name;
  sh_int                trust;
  char                 *game_names;
  char                 *password;
  char                 *room_name;
  int			honorary;
};

struct account_data
{
  char *login;
  char *password;
  char *character;
  struct account_data *next;
};

struct top_data
{
  char *name;
  int kills;
};

/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3

/* raw proportion (out of 1k) of kp to be transfered before tax */
#define KILL_TRANSFER_PROPORTION 2

#define KP_BADASS 1500
/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    sh_int	level;
    char *	keyword;
    char *	text;
};

/*
 * Data structure for notes.
 */
struct	note_data
{
    NOTE_DATA *	next;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t  	date_stamp;
};

/* RT ASCII conversions -- used so we can have letters in this file */

#define A		  	1
#define B			2
#define C			4
#define D			8
#define E			16
#define F			32
#define G			64
#define H			128

#define I			256
#define J			512
#define K		        1024
#define L		 	2048
#define M			4096
#define N		 	8192
#define O			16384
#define P			32768

#define Q			65536
#define R			131072
#define S			262144
#define T			524288
#define U			1048576
#define V			2097152
#define W			4194304
#define X			8388608

#define Y			16777216
#define Z			33554432
#define aa			67108864 	/* doubled due to conflicts */
#define bb			134217728
#define cc			268435456    
#define dd			536870912
#define ee			1073741824

/* mob behaviors */
#define BEHAVIOR_LD 0
#define BEHAVIOR_GUARD 1
#define BEHAVIOR_SEEK 2
#define BEHAVIOR_WANDER 3
#define BEHAVIOR_PILLBOX 4
#define BEHAVIOR_SEEKING_PILLBOX 5
#define BEHAVIOR_PLAYER_CONTROL 6

#define LIFETIME_SMOKE 100

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		(A)
#define AFF_DAZE		(B)

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2

/* dice */
#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		(A)
#define ITEM_WEAR_FINGER	(B)
#define ITEM_WEAR_NECK		(C)
#define ITEM_WEAR_BODY		(D)
#define ITEM_WEAR_HEAD		(E)
#define ITEM_WEAR_LEGS		(F)
#define ITEM_WEAR_FEET		(G)
#define ITEM_WEAR_HANDS		(H)
#define ITEM_WEAR_ARMS		(I)
#define ITEM_WEAR_SHIELD	(J)
#define ITEM_WEAR_ABOUT		(K)
#define ITEM_WEAR_WAIST		(L)
#define ITEM_WEAR_WRIST		(M)
#define ITEM_WIELD		(N)
#define ITEM_HOLD		(O)
#define ITEM_TWO_HANDS		(P)


/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		(A)
#define ROOM_TANK               (B)
#define ROOM_CONNECTED          (C)/* used at boot to guarantee connectivity */
#define ROOM_PRIVATE		(J)
#define ROOM_SAFE		(K)
#define ROOM_SOLITARY		(L)
#define ROOM_GODS_ONLY		(P)
#define ROOM_NEWBIES_ONLY	(R)


/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      (A)
#define EX_CLOSED		      (B)
#define EX_LOCKED		      (C)
#define EX_ISWALL                     (D)
#define EX_ISNOBREAKWALL              (E)
#define EX_ISNOBREAKDOOR              (F)
#define EX_ISNOLOCKDOOR               (G)
#define EX_ISNOUNLOCKDOOR             (H)

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_LEGS		      7
#define WEAR_FEET		      8
#define WEAR_HANDS		      9
#define WEAR_ARMS		     10
#define WEAR_SHIELD		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD		     16
#define WEAR_HOLD		     17
#define MAX_WEAR		     18



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_RESTING		      5
#define POS_SITTING		      6
#define POS_FIGHTING		      7
#define POS_STANDING		      8



/*
 * ACT bits for players.
 */
#define PLR_AGGRO_ALL           (L)
#define PLR_HOLYLIGHT		(N)
#define PLR_WIZINVIS		(O)
#define PLR_NOLEADER		(P)
#define PLR_LOG			(W)
#define PLR_DENY		(X)
#define PLR_FREEZE		(Y)

/*temp flags*/
/*tank flags*/
#define IN_TANK                 (A)
#define MAN_SHIELD              (B)
#define MAN_TURRET              (C)
#define MAN_DRIVE               (D)
#define TRAITOR                 (E)

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (A)
#define COMM_NOWIZ              (B)
#define COMM_NOIMP		(C)
#define COMM_NOBOUNTY           (F)
#define COMM_NOKILLS            (G)

/* 4 channels reserved, H-K */

/* display flags */
#define COMM_COMPACT		(L)
#define COMM_BRIEF		(M)
#define COMM_PROMPT		(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)
/* 3 flags reserved, Q-S */

/* penalties */
#define COMM_NOEMOTE		(T)
#define COMM_NOSHOUT		(U)
#define COMM_NOTELL		(V)
#define COMM_NOCHANNELS		(W) 

struct level_data
{
  sh_int                num_levels;
  sh_int                level_number;
  sh_int                x_size;
  sh_int                y_size;
  sh_int                reference_x;/* x,y coordinates of room directly below*/
  sh_int                reference_y;/* the reference from the previous level */
  ROOM_DATA *           rooms_on_level;
  LEVEL_DATA *          level_down;
  LEVEL_DATA *          level_up;
};

struct room_data
{
  CHAR_DATA *           people;
  OBJ_DATA *            contents;
  OBJ_DATA *            mine;
  LEVEL_DATA *          this_level;
  OBJ_DATA *            interior_of;
  CHAR_DATA *		inside_mob;
  long                  exit[6];
  long                  room_flags;
  int                   x, y, level;
  char *                name;
  char *                description;
};

/*
 * One character (PC or NPC).
 */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *         next_extract;
    CHAR_DATA *		leader;
    CHAR_DATA *		fighting;
    CHAR_DATA *		chasing;
    CHAR_DATA *		reply;
    CHAR_DATA *		last_hit_by;
    CHAR_DATA *		owner;
    SPEC_FUN *		spec_fun;
    DESCRIPTOR_DATA *	desc;
    NOTE_DATA *		pnote;
    OBJ_DATA *		carrying;
    ROOM_DATA *	        in_room;
    ROOM_DATA *		interior;
    PC_DATA *		pcdata;
    int			allows_followers;
    char		*kill_msg;
    char        	*names;
    char                *where_start;
    sh_int              valid; /* added for debugging purposes */
    sh_int              move_delay;
    sh_int              carry_number;
    sh_int              carry_weight;
    sh_int              ld_behavior;
    char 		*short_descript;
    sh_int		sex;
    sh_int		trust;
    byte 		color; 
    int			played;
    int			lines;  /* for the pager */
    time_t		logon;
    time_t		last_note;
    time_t		last_fight; 
    sh_int		timer;
    sh_int		wait;
    int                 kills, deaths;
    int			hit;
    int			max_hit;
    long		act, temp_flags;
    long		comm;   /* RT added to pad the vector */
    sh_int		invis_level;
    sh_int              armor;
    int			affected_by;
    sh_int		position;
    sh_int              start_pos;
    sh_int		donate_num;
    int			chan_delay;
    int			report;
    int			ld_timer;
    int			turret_dir, shield_dir; /* used only for tanks */
};

/*
 * Data which only PC's have.
 */
struct	pc_data
{
    PC_DATA *		next;
    char 		*password;
    char 		*poofin_msg;
    char 		*poofout_msg;
    char		*title_line;
    char                *account;
    int			solo_hit;
    bool              	confirm_delete;

    byte 		color_action;
    byte		color_combat_condition_s;
    byte		color_combat_condition_o;
    byte 		color_xy;
    byte 		color_wizi;
    byte 		color_hp;
    byte 		color_combat_o;
    byte 		color_level;
    byte 		color_exits;
    byte 		color_desc;
    byte		color_obj;
    byte		color_say;
    byte		color_tell;
    byte		color_reply;


};

struct	obj_index_data
{
/*    OBJ_INDEX_DATA *	next;*/
    char *		name;
    char *		short_descr;
    char *		description;
    char *              explode_desc;
    sh_int		vnum;
    int                 hp_struct;
    int                 hp_char;
    sh_int		item_type;
    sh_int              count;
    sh_int              prob_in, number_to_put_in;
    int			extract_flags, general_flags, usage_flags;
    sh_int		wear_flags;
    sh_int              ammo_type;
    sh_int              burn_time;
    sh_int              ammo;
    sh_int		weight;
    sh_int              range;
    sh_int              rounds_per_second;
    sh_int              armor;
    sh_int              damage_char[3]; /* 0. direct 1. in room 2. 1 away */
    sh_int              damage_structural[3];
};



/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		next_content;
    OBJ_DATA *		contains;
    OBJ_DATA *          in_obj;
    CHAR_DATA *		carried_by;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_DATA *	        in_room;
    CHAR_DATA *	        owner;
    char *		name;
    char *		short_descr;
    char *		description;
    char *              explode_desc;
    sh_int              valid; /* added for debugging purposes */
    sh_int		item_type;
    sh_int              vnum;
    int			extract_flags, general_flags, usage_flags;
    int                 hp_struct;
    int                 hp_char;
    sh_int              extract_me;
    sh_int		wear_flags;
    sh_int		wear_loc;
    sh_int		weight;
    sh_int		timer;
    sh_int              range;
    sh_int              rounds_per_second;
    sh_int              armor;
    sh_int              arrival_time;
    sh_int              ammo;
    sh_int              ammo_type;
    sh_int              wait_time;
    sh_int              dir_facing;
    sh_int              burn_time;
    ROOM_DATA *         destination;
    ROOM_DATA *         interior; /* tank or plane interior */
    sh_int              damage_char[3]; /* 0. direct 1. in room 2. 1 away */
    sh_int              damage_structural[3];
};

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))

/*
 * Character macros.
 */
#define IS_NPC(ch)		(((ch)->pcdata == NULL))
#define IS_IMMORTAL(ch)		(get_trust(ch))
#define IS_HERO(ch)		(get_trust(ch))
#define IS_IMP(ch)		(get_trust(ch) == MAX_TRUST)
#define IS_TRUSTED(ch,level)	(get_trust(ch))

#define GET_AGE(ch)		((int) (17 + ((ch)->played \
				    + current_time - (ch)->logon )/72000))

#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)

#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DESC_WAIT_STATE(d, npulse) ((d)->wait = UMAX((d)->wait, (npulse)))


/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))



/*
 * Description macros.
 */
#define PERS(ch, looker)	( can_see( looker, (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descript	\
				: (ch)->names ) : "someone" )

/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    char *    name;
    char *    char_no_arg;
    char *    others_no_arg;
    char *    char_found;
    char *    others_found;
    char *    vict_found;
    char *    char_not_found;
    char *      char_auto;
    char *      others_auto;
};

/*
 * Global constants.
 */
extern          struct social_type      social_table[MAX_SOCIALS];
extern  const   struct god_type         imp_table[];

/*
 * Global variables.
 */
extern		HELP_DATA	  *	help_first;
extern		BAN_DATA	  *	ban_list;
extern		BAN_DATA	  *	nban_list;
extern		CHAR_DATA	  *	char_list;
extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		NOTE_DATA	  *	note_list;
extern		OBJ_DATA	  *	object_list;

extern          OBJ_DATA          *     object_list;
extern          OBJ_INDEX_DATA    *     all_objects;
extern          LEVEL_DATA        *     the_city;
extern		BAN_DATA	  *	ban_free;
extern		CHAR_DATA	  *	char_free;
extern		DESCRIPTOR_DATA	  *	descriptor_free;
extern		NOTE_DATA	  *	note_free;
extern		OBJ_DATA	  *	obj_free;
extern		PC_DATA		  *	pcdata_free;

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		sh_int      		team_choice;
extern          sh_int                  teleport_time;
extern          int                     tick_counter;
extern          int                     expansions;
extern          int                    ground0_down;
extern		ROOM_DATA	   *ammo_repop[3];
extern          ROOM_DATA              *safe_area, *someimp_area;
extern          ROOM_DATA              *god_general_area;
extern		ROOM_DATA	       *explosive_area;
extern		ROOM_DATA	       *store_area;
extern          int                     game_seed;
extern          int                     boot_seed;
extern          GOD_TYPE               *god_table;
extern          int                     iteration;
extern          CHAR_DATA              *next_violence;
extern		int		        max_on;
extern		CHAR_DATA	       *enforcer;
extern		CHAR_DATA	       *pill_box;
extern		CHAR_DATA	       *guardian;
extern		CHAR_DATA	       *tank_mob;
extern		ACCOUNT_DATA	       *accounts_list;
extern		TOP_DATA	        top_players_kills[NUM_TOP];
extern		CHAR_DATA	       *extract_list;
extern		int			expand_event;
extern		bool			fBootDb;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#define NOCRYPT
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(linux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(macintosh)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MSDOS)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(sun)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
#if	defined(SYSV)
siz_t	fread		args( ( void *ptr, size_t size, size_t n, 
			    FILE *stream) );
#else
/* int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );*/
#endif
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(unix)
#define PLAYER_DIR	"../player/"	 /* Player files		*/
#define BAK_PLAYER_DIR	"../player/bak/" /* deleted player files       	*/
#define PLAYER_TEMP	"../player/temp"
#define GOD_DIR		"../gods/"	/* list of gods			*/
#define NULL_FILE	"/dev/null"	/* To reserve one stream	*/
#define BOOT_DIR	"../boot/"
#define ACCOUNTS_FILE   "../boot/accounts.boot" /* account list */
#define TOP_FILE        "../boot/top.boot" /* account list */
#endif

#define BOOT_LIST	"files.boot"	/* List of boot files		*/
#define BAN_LIST	"ban.boot"	/* List of banned sites		*/
#define NBAN_LIST	"nban.boot"	/* List of banned sites		*/

#define BUG_FILE	"../boot/bugs.txt"      /* For 'bug' and bug( )	*/
#define IDEA_FILE	"../boot/ideas.txt"	/* For 'idea'		*/
#define TYPO_FILE	"../boot/typos.txt"     /* For 'typo'		*/
#define NOTE_FILE	"../boot/notes.txt"	/* For 'notes'		*/
#define SHUTDOWN_FILE	"../boot/shutdown.txt"	/* For 'shutdown'	*/
#define REBOOT_FILE	"../boot/reboot.txt"	/* For 'reboot'	*/



/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RD	ROOM_DATA
#define SF	SPEC_FUN

/* act_comm.c */
bool    is_note_to      args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void  	check_sex	args( ( CHAR_DATA *ch) );
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void 	nuke_pets	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );

/* act_info.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
int get_num_rank (CHAR_DATA *ch);
char *get_rank (CHAR_DATA *ch);
int is_leader (CHAR_DATA *ch);

/* act_move.c */
void	move_char	args( ( CHAR_DATA *ch, int door, bool follow ) );
ROOM_DATA*  get_to_room args( ( ROOM_DATA *curr_room, sh_int door));
ROOM_DATA*  index_room  args( ( ROOM_DATA *curr_room, sh_int x, sh_int y));
void    complete_movement (CHAR_DATA *ch);

/* act_obj.c */
bool can_loot		args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
                            OBJ_DATA *container ) );
CHAR_DATA *owner_objective (int num);

/* act_wiz.c */

/* comm.c */
void	show_string	args( ( struct descriptor_data *d, char *input) );
void	close_socket	args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt,
			    int length ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	page_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	act		args( ( const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
void	act_new		args( ( const char *format, CHAR_DATA *ch, 
			    const void *arg1, const void *arg2, int type,
			    int min_pos) );

/* db.c */
void	boot_db		args( ( void ) );
void	area_update	args( ( void ) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void	clone_object	args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
void	free_char	args( ( CHAR_DATA *ch ) );
OID *	get_obj_index	args( ( int vnum ) );
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
long 	fread_flag	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
char *  fread_string_eol args(( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
long	flag_convert	args( ( char letter) );
void mark_alloc (void *ptr, int size, struct allocations_type **first_alloc,
		 char *filename, int line_num);
void mark_free (void *ptr, struct allocations_type **first_alloc, 
		char *filename, int line_num);
struct allocations_type *get_mem_reference (void *ptr, 
				      struct allocations_type **first_alloc);
int size_correct (void *ptr, int size, struct allocations_type **first_alloc);
#define alloc_mem(sMem)      f_alloc_mem (sMem, __FILE__, __LINE__)
#define alloc_perm(sMem)     f_alloc_perm (sMem, __FILE__, __LINE__)
#define free_mem(pMem, sMem) f_free_mem (pMem, sMem, __FILE__, __LINE__)
#define str_dup(str)         f_str_dup (str, __FILE__, __LINE__)
#define really_str_dup(str)  f_really_str_dup (str, __FILE__, __LINE__)
#define free_string(pstr)    f_free_string (pstr, __FILE__, __LINE__)
void *	f_alloc_mem	args( ( int sMem, char *filename, int line_num ) );
void *	f_alloc_perm	args( ( int sMem, char *filename, int line_num ) );
void	f_free_mem	args( ( void *pMem, int sMem, char *filename, 
			       int line_num ) );
char *	f_str_dup		args( ( const char *str, char *filename,
				       int line_num) );
char *	f_really_str_dup	args( ( const char *str, char *filename,
				       int line_num) );
void	f_free_string	args( ( char *pstr, char *filename, int line_num) );
int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
int     number_mm       args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	bug		args( ( const char *str, int param ) );
void	log_string	(char *str, ...);
void	tail_chain	args( ( void ) );
void    expand_city ();  
void load_accounts (FILE *fp);

/* fight.c */
bool 	is_safe		args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool 	is_safe_spell	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
bool	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, int class ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );

/* handler.c */
int 	check_immune	args( (CHAR_DATA *ch, int dam_type) );
int 	material_lookup args( ( const char *name) );
int	race_lookup	args( ( const char *name) );
int	class_lookup	args( ( const char *name) );
bool	is_old_mob	args ( (CHAR_DATA *ch) );
int	get_skill	args( ( CHAR_DATA *ch, int sn ) );
int	get_weapon_sn	args( ( CHAR_DATA *ch ) );
int	get_weapon_skill args(( CHAR_DATA *ch, int sn ) );
int     get_age         args( ( CHAR_DATA *ch ) );
void	reset_char	args( ( CHAR_DATA *ch )  );
int	get_trust	args( ( CHAR_DATA *ch ) );
int	get_curr_stat	args( ( CHAR_DATA *ch, int stat ) );
int 	get_max_train	args( ( CHAR_DATA *ch, int stat ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( char *str, char *namelist ) );
bool    is_name_exact ( char *str, char *namelist );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_obj	args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove_obj args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, int sn ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear, int type ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj, int perm_extract) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	create_money	args( ( int amount ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_DATA *pRoomIndex ) );
bool	room_is_private	args( ( ROOM_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_see_room	args( ( CHAR_DATA *ch, ROOM_DATA *pRoomIndex) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	item_type_name	args( ( OBJ_DATA *obj ) );
char *	affect_loc_name	args( ( int location ) );
char *	affect_bit_name	args( ( int vector ) );
char *	extra_bit_name	args( ( int extra_flags ) );
char * 	wear_bit_name	args( ( int wear_flags ) );
char *	act_bit_name	args( ( int act_flags ) );
char *	off_bit_name	args( ( int off_flags ) );
char *  imm_bit_name	args( ( int imm_flags ) );
char * 	form_bit_name	args( ( int form_flags ) );
char *	part_bit_name	args( ( int part_flags ) );
char *	weapon_bit_name	args( ( int weapon_flags ) );
char *  comm_bit_name	args( ( int comm_flags ) );
int rem_from_top (TOP_DATA *top_array, CHAR_DATA *ch);
int better_than (CHAR_DATA *ch, TOP_DATA *top_array, int num);
void add_to_top (TOP_DATA *top_array, CHAR_DATA *ch, int num);
void show_top_list (CHAR_DATA *ch, TOP_DATA *top_array);
void insert_top (CHAR_DATA *ch, TOP_DATA *top_array);
void top_stuff (CHAR_DATA *ch);

/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
char *	one_argument	args( ( char *argument, char *arg_first ) );

/* magic.c */
int 	mana_cost 	(CHAR_DATA *ch, int min_mana, int level);
int	skill_lookup	args( ( const char *name ) );
int	slot_lookup	args( ( int slot ) );
bool	saves_spell	args( ( int level, CHAR_DATA *victim ) );
void	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch,
				    CHAR_DATA *victim, OBJ_DATA *obj ) );

/* save.c */
void	   save_char_obj	args( ( CHAR_DATA *ch ) );
CHAR_DATA *load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name ) );
void save_account_list ();

/* skills.c */
bool 	parse_gen_groups args( ( CHAR_DATA *ch,char *argument ) );
void 	list_group_costs args( ( CHAR_DATA *ch ) );
void    list_group_known args( ( CHAR_DATA *ch ) );
void 	check_improve	args( ( CHAR_DATA *ch, int sn, bool success, 
				    int multiplier ) );
int 	group_lookup	args( (const char *name) );
void	gn_add		args( ( CHAR_DATA *ch, int gn) );
void 	gn_remove	args( ( CHAR_DATA *ch, int gn) );
void 	group_add	args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void	group_remove	args( ( CHAR_DATA *ch, const char *name) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );

/* update.c */
void	advance_level	args( ( CHAR_DATA *ch ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );

#undef	CD
#undef	OD
#undef	OID
#undef	RID
#undef	SF
