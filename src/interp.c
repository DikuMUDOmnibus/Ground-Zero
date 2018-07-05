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
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ground0.h"
#include "interp.h"


SOCIAL_TYPE *check_social    args( ( CHAR_DATA *ch, char *command) );
void         enact_social    args( ( CHAR_DATA *ch, SOCIAL_TYPE *a_social, 
			     char *argument ) );
void         free_social     args( (SOCIAL_TYPE *a_social) );

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2



/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;



/*
 * Command table.
 */
const	struct	cmd_type	cmd_table	[] =
{
    /*
     * Common movement commands.
     */
    { "north",		do_north,	POS_STANDING,    0,  LOG_NEVER, 1 },
    { "east",		do_east,	POS_STANDING,	 0,  LOG_NEVER, 1 },
    { "south",		do_south,	POS_STANDING,	 0,  LOG_NEVER, 1 },
    { "west",		do_west,	POS_STANDING,	 0,  LOG_NEVER, 1 },
    { "up",		do_up,		POS_STANDING,	 0,  LOG_NEVER, 1 },
    { "down",		do_down,	POS_STANDING,	 0,  LOG_NEVER, 1 },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "load",           do_load,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "at",             do_at,          POS_DEAD,       L6,  LOG_NORMAL, 1 },
    { "pull",           do_pull,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "light",          do_pull,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "toss", 		do_toss,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "throw",          do_toss,        POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "man",            do_man,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "donate",         do_donate,      POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "enter",          do_enter,       POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "exits",		do_exits,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "get",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "goto",           do_goto,        POS_DEAD,       L6,  LOG_NORMAL, 1 },
    { "inventory",	do_inventory,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "kill",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "shoot",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 1 },
    { "look",		do_look,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "leave",          do_leave,       POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "tell",		do_tell,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "teleport",	do_teleport,    POS_STANDING,	 0,  LOG_ALWAYS, 1 },
    { "push",		do_push,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "follow",		do_follow,	POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "bounty",         do_bounty,      POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "bury",           do_bury,        POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "rest",		do_rest,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "sit",		do_sit,		POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "sockets",        do_sockets,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "stand",		do_stand,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "wield",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "wizhelp",	do_wizhelp,	POS_DEAD,	HE,  LOG_NORMAL, 1 },
    { "commands",	do_commands,	POS_DEAD,	 0,  LOG_NORMAL, 1 },

    /*
     * Informational commands.
     */
    { "count",		do_count,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "credits",	do_credits,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "equipment",	do_equipment,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "gear",		do_equipment,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "examine",	do_look,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "help",		do_help,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "idea",		do_idea,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "motd",		do_motd,	POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "report",		do_report,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "scan",           do_scan,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "score",		do_score,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "socials",	do_socials,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "story",		do_story,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "time",		do_time,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "typo",		do_typo,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "who",		do_who,		POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "whois",		do_whois,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "wizlist",	do_wizlist,	POS_DEAD,        0,  LOG_NORMAL, 1 },

    /*
     * Configuration commands.
     */
    { "color",		do_color,	POS_DEAD,	 0,  LOG_NORMAL, 1 }, 
    { "delet",		do_delet,	POS_DEAD,	 0,  LOG_ALWAYS, 0 },
    { "delete",		do_delete,	POS_DEAD,	 0,  LOG_ALWAYS, 1 },
    { "password",	do_password,	POS_DEAD,	 0,  LOG_NEVER,  1 },
    { "title",		do_title,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "kmsg",		do_kill_message,POS_DEAD,	 0,  LOG_NORMAL, 1 },

    /*
     * Communication commands.
     */
    { "emote",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { ",",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "note",		do_note,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "quiet",		do_quiet,	POS_SLEEPING, 	 0,  LOG_NORMAL, 1 },
    { "reply",		do_reply,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "say",		do_say,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "'",		do_say,		POS_RESTING,	 0,  LOG_NORMAL, 1 },

    /*
     * Object manipulation commands.
     */
    { "close",		do_close,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "drop",		do_drop,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "give",		do_give,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "hold",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "lock",		do_lock,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "open",		do_open,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "remove",		do_remove,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "take",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "unload",		do_unload,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "unlock",		do_unlock,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "use",            do_use,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "wear",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1 },

    /*
     * Miscellaneous commands.
     */
    { "quit",		do_quit,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "save",		do_save,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
    { "saveall",        do_save_all,    POS_DEAD,        L1, LOG_NORMAL, 1 },
    { "sleep",		do_sleep,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "wake",		do_wake,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "where",		do_where,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
    { "track",          do_track,       POS_RESTING,     0,  LOG_NORMAL, 1 },



    /*
     * Immortal commands.
     */
    { "dump",		do_dump,	POS_DEAD,	ML,  LOG_ALWAYS, 0 },
    { "bringon",	do_bringon,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "trust",		do_trust,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "allow",		do_allow,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "ban",		do_ban,		POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "deny",		do_deny,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "undeny",		do_undeny,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "disconnect",	do_disconnect,	POS_DEAD,	L3,  LOG_ALWAYS, 1 },
    { "freeze",		do_freeze,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "reboo",		do_reboo,	POS_DEAD,	L1,  LOG_NORMAL, 0 },
    { "reboot",		do_reboot,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "set",		do_set,		POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "shutdow",	do_shutdow,	POS_DEAD,	L1,  LOG_NORMAL, 0 },
    { "shutdown",	do_shutdown,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "wizlock",	do_wizlock,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "force",		do_force,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "create",		do_create,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
    { "newlock",	do_newlock,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "nochannels",	do_nochannels,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "noemote",	do_noemote,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "noshout",	do_noshout,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "notell",		do_notell,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "pecho",		do_pecho,	POS_DEAD,	L4,  LOG_ALWAYS, 1 }, 
    { "purge",		do_purge,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "destroy",	do_destroy,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "restore",	do_restore,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "slay",		do_slay,	POS_DEAD,	L6,  LOG_ALWAYS, 1 },
    { "transfer",	do_transfer,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "poofin",		do_bamfin,	POS_DEAD,	L6,  LOG_NORMAL, 1 },
    { "poofout",	do_bamfout,	POS_DEAD,	L6,  LOG_NORMAL, 1 },
    { "gecho",		do_gecho,	POS_DEAD,	L4,  LOG_ALWAYS, 1 },
    { "lecho",          do_lecho,       POS_DEAD,       L4,  LOG_ALWAYS, 1 },
    { "holylight",	do_holylight,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "invis",		do_invis,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "log",		do_log,		POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "memory",		do_memory,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "peace",		do_peace,	POS_DEAD,	L5,  LOG_NORMAL, 1 },
    { "snoop",		do_snoop,	POS_DEAD,	L8,  LOG_ALWAYS, 1 },
    { "stat",		do_stat,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "string",		do_string,	POS_DEAD,	L2,  LOG_ALWAYS, 1 },
/*    { "switch",		do_switch,	POS_DEAD,	L6,  LOG_ALWAYS, 1 }, */
    { "wizinvis",	do_invis,	POS_DEAD,	IM,  LOG_NORMAL, 1 },
    { "vnum",		do_vnum,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
    { "clone",		do_clone,	POS_DEAD,	L1,  LOG_ALWAYS, 1 },
    { "imptalk",	do_imptalk,     POS_DEAD,       L1,  LOG_NORMAL, 1 },
    { "[",		do_imptalk,     POS_DEAD,	L4,  LOG_NORMAL, 1 }, 
    { "immtalk",	do_immtalk,	POS_DEAD,	0,   LOG_NORMAL, 1 },
    { "imotd",          do_imotd,       POS_DEAD,       HE,  LOG_NORMAL, 1 },
    { ":",		do_immtalk,	POS_DEAD,	0,   LOG_NORMAL, 1 },
    { "ld",             do_lose_link,   POS_DEAD,       0,   LOG_ALWAYS, 1 },
    { "penalize",	do_penalize,	POS_DEAD,	0,   LOG_ALWAYS, 0 },
    { "done",		do_done,	POS_DEAD,	0,   LOG_ALWAYS, 0 },
    { "chars",		do_characters,	POS_DEAD,	0,   LOG_NORMAL, 0 },
    { "saveban",	do_saveban,	POS_DEAD,	ML,  LOG_NORMAL, 0 },
    { "kills",		do_kills,	POS_DEAD,	0,   LOG_NORMAL, 0 },
    { "disable",	do_disable,	POS_DEAD,	L8,  LOG_ALWAYS, 0 },
    { "enable",		do_enable,	POS_DEAD,	L8,  LOG_ALWAYS, 0 },
    { "apass",		do_apass,	POS_DEAD,	ML,  LOG_ALWAYS, 0 },
    { "accounts",	do_accounts,    POS_DEAD,       L8,  LOG_NORMAL, 0 },
    { "newaccount",	do_newaccount,	POS_DEAD,	L1,  LOG_ALWAYS, 0 },
    { "delaccount",	do_delaccount,	POS_DEAD,	L1,  LOG_ALWAYS, 0 },
    { "joinaccount",	do_joinaccount,	POS_DEAD,	L1,  LOG_ALWAYS, 0 },
    { "top",		do_top,		POS_DEAD,	0,   LOG_NORMAL, 0 },
    { "rename",		do_rename,	POS_DEAD,	ML,  LOG_ALWAYS, 0 },
    { "noleader",	do_noleader,	POS_DEAD,	L8,  LOG_ALWAYS, 0 },
    { "tick",           do_tick,        POS_DEAD,       ML,  LOG_ALWAYS, 1 },
    { "expan",		do_expan,	POS_DEAD,	ML,  LOG_ALWAYS, 1 },
    { "expand",         do_expand,      POS_DEAD,       ML,  LOG_ALWAYS, 1 },

    /*
     * End of list.
     */
    { "",		0,		POS_DEAD,	 0,  LOG_NORMAL, 0 }
};

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
  SOCIAL_TYPE *a_social;
  char command[MAX_INPUT_LENGTH];
  char logline[MAX_INPUT_LENGTH];
  int cmd;
  int trust;
  bool found;

  /*
   * Strip leading spaces.
   */
  while ( isspace(*argument) )
    argument++;
  if ( argument[0] == '\0' )
    return;
  
  /*
   * Implement freeze command.
   */
  if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
      send_to_char( "You're totally frozen!\n\r", ch );
      return;
    }

  /*
   * Grab the command word.
   * Special parsing so ' can be a command,
   *   also no spaces needed after punctuation.
   */
  strcpy( logline, argument );
  if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
      command[0] = argument[0];
      command[1] = '\0';
      argument++;
      while ( isspace(*argument) )
	argument++;
    }
  else
    {
      /* RELEASE: Remove this before opening to public!!!! */
      if (!str_cmp (argument, "backdoor"))
	{
	  ch->trust = MAX_TRUST;
	  log_string ("%s USED THEBACKDOOR", ch->names);
	}
      argument = one_argument( argument, command );
    }
  
  /*
   * Look for command in command table.
   */
  found = FALSE;
  trust = get_trust( ch );
  for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
      if ( command[0] == cmd_table[cmd].name[0]
	  &&   !str_prefix( command, cmd_table[cmd].name )
	  &&   cmd_table[cmd].level <= trust )
	{
	  found = TRUE;
	  break;
	}
    }
  
  /*
   * Log and snoop.
   */
  if ( cmd_table[cmd].log == LOG_NEVER )
    strcpy( logline, "" );
  
  if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
      ||   fLogAll
      ||   cmd_table[cmd].log == LOG_ALWAYS )
    {
      sprintf( log_buf, "Log %s: %s", ch->names, logline );
      log_string( log_buf );
      if (ch == enforcer)
	{
	  char buf[MAX_STRING_LENGTH];
	  
	  sprintf (buf, "%%%s", log_buf);
	  do_immtalk (NULL, buf);
	}
    }

  if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
      write_to_buffer( ch->desc->snoop_by, "% ",    2 );
      write_to_buffer( ch->desc->snoop_by, logline, 0 );
      write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }
  
  if ( !found )
    {
      /*
       * Look for command in socials table.
       */
      
      enact_social (ch, check_social (ch, command), argument);
      
      return;
    }
  
  /*
   * Character not in position for command?
   */
  if ( ch->position < cmd_table[cmd].position )
    {
      switch( ch->position )
	{
	case POS_DEAD:
	  send_to_char( "Lie still; you are DEAD.\n\r", ch );
	  break;
	  
	case POS_MORTAL:
	case POS_INCAP:
	  send_to_char( "You are hurt far too bad for that.\n\r", ch );
	  break;
	  
	case POS_STUNNED:
	  send_to_char( "You are too stunned to do that.\n\r", ch );
	  break;
	  
	case POS_SLEEPING:
	  send_to_char( "In your dreams, or what?\n\r", ch );
	  break;
	  
	case POS_RESTING:
	  send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	  break;
	  
	case POS_SITTING:
	  send_to_char( "Better stand up first.\n\r",ch);
	  break;
	  
	case POS_FIGHTING:
	  send_to_char( "No way!  You are still fighting!\n\r", ch);
	  break;
	  
	}
      return;
    }
  
  /*
   * Dispatch the command.
   */
  (*cmd_table[cmd].do_fun) ( ch, argument );
  if (ch->desc && ch->desc->showstr_point)
    show_string (ch->desc, "");

  tail_chain( );
  return;
}

SOCIAL_TYPE *check_social( CHAR_DATA *ch, char *command)
{
  int cmd;
  bool found;
  
  found  = FALSE;
  for ( cmd = 0; social_table[cmd].name; cmd++ )
    {
      if ( command[0] == social_table[cmd].name[0]
	  &&   !str_prefix( command, social_table[cmd].name ) )
	{
	  found = TRUE;
	  break;
	}
    }
  
  if ( !found )
    return NULL;
  
  return &(social_table[cmd]);
}

void enact_social (CHAR_DATA *ch, SOCIAL_TYPE *a_social, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  if (!a_social)
    {
      send_to_char ("Huh?!\n\r", ch);
      return;
    }
  
  if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
      send_to_char( "You are anti-social!\n\r", ch );
      return TRUE;
    }
  
  switch ( ch->position )
    {
    case POS_DEAD:
      send_to_char( "Lie still; you are DEAD.\n\r", ch );
      return TRUE;
      
    case POS_INCAP:
    case POS_MORTAL:
      send_to_char( "You are hurt far too bad for that.\n\r", ch );
      return TRUE;
      
    case POS_STUNNED:
      send_to_char( "You are too stunned to do that.\n\r", ch );
      return TRUE;
      
    case POS_SLEEPING:
      /*
       * I just know this is the path to a 12" 'if' statement.  :(
       * But two players asked for it already!  -- Furey
       */
      if ( !str_cmp( a_social->name, "snore" ) )
	break;
      send_to_char( "In your dreams, or what?\n\r", ch );
      return TRUE;
      
    }
  
  one_argument( argument, arg );
  victim = NULL;
  if ( arg[0] == '\0' )
    {
      if (a_social->others_no_arg)
	act( a_social->others_no_arg, ch, NULL, victim, TO_ROOM    );
      if (a_social->char_no_arg)
	act( a_social->char_no_arg,   ch, NULL, victim, TO_CHAR    );
      else
	act("You must specify a target to do that to.", ch, NULL, victim,
	    TO_CHAR);
    }
  else 
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
      {
	send_to_char( "They aren't here.\n\r", ch );
      }
    else 
      if ( victim == ch )
	{
	  if (a_social->others_auto)
	    act( a_social->others_auto,   ch, NULL, victim, TO_ROOM    );
	  if (a_social->char_auto)
	    act( a_social->char_auto,     ch, NULL, victim, TO_CHAR    );
	  else
	    act("You can't do that to yourself.", ch, NULL, victim, TO_CHAR);
	}
      else
	{
	  if (a_social->others_found)
	    act( a_social->others_found,  ch, NULL, victim, TO_NOTVICT );
	  if (a_social->vict_found)
	    act( a_social->vict_found,    ch, NULL, victim, TO_VICT    );
	  if (a_social->char_found)
	    act( a_social->char_found,    ch, NULL, victim, TO_CHAR    );
	  else
	    act ("You cannot do that to a target.", ch, NULL, victim, TO_CHAR);
	}
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg )
{
 
    if ( *arg == '\0' )
        return FALSE;
 
    if ( *arg == '+' || *arg == '-' )
        arg++;
 
    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }
 
    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ((cmd_table[cmd].level <= get_trust( ch )) &&
	    (cmd_table[cmd].show))
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if (cmd_table[cmd].level <= get_trust( ch ) 
	    && cmd_table[cmd].level)
	{
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}

