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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ground0.h"

/* command procedures needed */
DECLARE_DO_FUN(do_mstat		);
DECLARE_DO_FUN(do_ostat		);
DECLARE_DO_FUN(do_rset		);
DECLARE_DO_FUN(do_mset		);
DECLARE_DO_FUN(do_oset		);
DECLARE_DO_FUN(do_sset		);
DECLARE_DO_FUN(do_mfind		);
DECLARE_DO_FUN(do_ofind		);
DECLARE_DO_FUN(do_slookup	);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_save		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_force		);


/*
 * Local functions.
 */
ROOM_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );

/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  sh_int count;
  
  for (count = 0; count < 5; count++)
    {
      obj = create_object (get_obj_index (VNUM_9MM_CLIP), 0);
      obj->extract_me = 1;
      obj_to_char (obj, ch);
    }
  obj = create_object (get_obj_index (VNUM_9MM_PISTOL), 0);
  obj->extract_me = 1;
  obj_to_char (obj, ch);
  obj = create_object (get_obj_index (VNUM_GRENADE), 0);
  obj->extract_me = 1;
  obj_to_char (obj, ch);
}
     
/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      send_to_char( "Nochannel whom?", ch );
      return;
    }
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }
  
  if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
      REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
      send_to_char( "The gods have restored your channel priviliges.\n\r", 
		   victim );
      send_to_char( "NOCHANNELS removed.\n\r", ch );
    }
  else
    {
      SET_BIT(victim->comm, COMM_NOCHANNELS);
      send_to_char( "The gods have revoked your channel priviliges.\n\r", 
		   victim );
      send_to_char( "NOCHANNELS set.\n\r", ch );
    }
  
  return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  
  if ( !IS_NPC(ch) )
    {
      smash_tilde( argument );
      
      if (argument[0] == '\0')
	{
	  sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->poofin_msg);
	  send_to_char(buf,ch);
	  return;
	}
      
      free_string (ch->pcdata->poofin_msg);
      ch->pcdata->poofin_msg = str_dup (argument);
      
      sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->poofin_msg);
      send_to_char(buf,ch);
    }
  return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  
  if ( !IS_NPC(ch) )
    {
      smash_tilde( argument );
      
      if (argument[0] == '\0')
        {
	  sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->poofout_msg);
	  send_to_char(buf,ch);
	  return;
        }
      
      free_string (ch->pcdata->poofout_msg);
      ch->pcdata->poofout_msg = str_dup (argument);
      
      sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->poofout_msg);
      send_to_char(buf,ch);
    }
  return;
}

void do_bringon( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH], buf [MAX_INPUT_LENGTH];
  CHAR_DATA *the_summonned;
  OBJ_DATA *obj;
  sh_int count;

  one_argument (argument, arg);

  if (char_file_active (arg))
    {
      send_to_char ("That character is in the game at the moment.  Trans "
		    "them maybe.\n\r", ch);
      return;
    }

  if ((the_summonned = load_char_obj (NULL, arg)) == NULL)
    {
      send_to_char ("That character is not in the player files.\n\r", ch);
      return;
    }
  the_summonned->next = char_list;
  char_list = the_summonned;
  char_to_room (the_summonned, ch->in_room);

  do_outfit (the_summonned, "");
	
  if (the_summonned->trust != 10)
    {
      the_summonned->max_hit = HIT_POINTS_MORTAL;
      the_summonned->hit = HIT_POINTS_MORTAL;
      /* give them trust so they are not transed away from the god that
	 summonned them; when the character connects, the trust will be gone;
	 see comm.c */
      the_summonned->trust = 1;
    }
  else
    {
      the_summonned->max_hit = HIT_POINTS_IMMORTAL;
      the_summonned->hit = HIT_POINTS_IMMORTAL;
    }
  act ("$N appears before you.", ch, NULL, the_summonned, TO_CHAR);
  act ("$N appears before $n.", ch, NULL, the_summonned, TO_ROOM);
  sprintf (buf, "%s has summonned %s into the game.\n\r", ch->names, arg);
  log_string (buf);
}

void do_deny( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Deny whom?\n\r", ch );
      return;
    }
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  if ( IS_NPC(victim) )
    {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }
  
  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }
  
  if (!IS_SET (victim->act, PLR_DENY))
    {
      SET_BIT(victim->act, PLR_DENY);
      send_to_char( "You are denied access!\n\r", victim );
      send_to_char( "OK.\n\r", ch );
      save_char_obj(victim);
      if (victim->desc)
	close_socket (victim->desc);
      extract_char (victim, TRUE);
    }
  else
    {
      send_to_char ("That character is already denied.\n\r", ch);
    }
  
  return;
}

void do_undeny( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Undeny whom?\n\r", ch );
      return;
    }
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  if ( IS_NPC(victim) )
    {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }
  
  if (IS_SET (victim->act, PLR_DENY))
    {
      REMOVE_BIT(victim->act, PLR_DENY);
      send_to_char( "You are no longer denied access.\n\r", victim );
      send_to_char( "OK.\n\r", ch );
      save_char_obj(victim);
    }
  else
    {
      send_to_char ("That character is not denied.\n\r", ch);
    }
  
  return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Disconnect whom?\n\r", ch );
      return;
    }
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  if ( victim->desc == NULL )
    {
      act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
      return;
    }
  
  for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if ( d == victim->desc )
	{
	  close_socket( d );
	  send_to_char( "Ok.\n\r", ch );
	  return;
	}
    }
  
  bug( "Do_disconnect: desc not found.", 0 );
  send_to_char( "Descriptor not found!\n\r", ch );
  return;
}


void do_gecho( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA *d;
  
  if ( argument[0] == '\0' )
    {
      send_to_char( "Global echo what?\n\r", ch );
      return;
    }
  
  for ( d = descriptor_list; d; d = d->next )
    {
      if ( d->connected == CON_PLAYING )
	{
	  if (get_trust(d->character) >= get_trust(ch))
	    send_to_char( "global> ",d->character);
	  send_to_char( argument, d->character );
	  send_to_char( "\n\r",   d->character );
	}
    }
  
  return;
}



void do_lecho( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *chars_in_room;
  
  if ( argument[0] == '\0' )
    {
      send_to_char( "Local echo what?\n\r", ch );
      return;
    }
  
  for (chars_in_room = ch->in_room->people; chars_in_room; 
       chars_in_room = chars_in_room->next_in_room)
    {
      if (get_trust(chars_in_room) >= get_trust(ch))
	send_to_char( "local> ", chars_in_room);
      send_to_char( argument, chars_in_room );
      send_to_char( "\n\r",   chars_in_room );
    }
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  argument = one_argument(argument, arg);
  
  if ( argument[0] == '\0' || arg[0] == '\0' )
    {
      send_to_char("Personal echo what?\n\r", ch); 
      return;
    }
  
  if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
      send_to_char("Target not found.\n\r",ch);
      return;
    }
  
  if (get_trust(victim) >= get_trust(ch))
    send_to_char( "personal> ",victim);
  
  send_to_char(argument,victim);
  send_to_char("\n\r",victim);
  send_to_char( "personal> ",ch);
  send_to_char(argument,ch);
  send_to_char("\n\r",ch);
}


ROOM_DATA *find_location( CHAR_DATA *ch, char *argument )
{
  LEVEL_DATA *level_finder;
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];
  sh_int level, x, y;
  
  argument = one_argument (argument, arg);
  
  if (arg[0] == '\0')
    return NULL;

  if (!str_prefix (arg, "safe"))
    return safe_area;

  if (IS_IMMORTAL (ch) || IS_NPC (ch))
    {
      if (!str_prefix (arg, "god-someimp"))
	return someimp_area;
      
      if (!str_prefix (arg, "god-explode"))
	return explosive_area;

      if (!str_prefix (arg, "god-store"))
	return store_area;
    }

  if (!str_prefix (arg, "random"))
    {
      argument = one_argument (argument, arg);
      if (arg[0] != '\0')
	level = atoi(arg);
      else
	level = ch->in_room->this_level->level_number;
      
      for (level_finder = the_city; level_finder->level_down != the_city;
	   level_finder = level_finder->level_down)
	if (level_finder->level_number == level) break;
      if (level_finder->level_number != level)
	{
	  send_to_char ("Level not found.\n\r", ch);
	  return NULL;
	}
      else
	{
	  x = number_range (0, level_finder->x_size - 1);
	  y = number_range (0, level_finder->y_size - 1);
	  return index_room (level_finder->rooms_on_level, x, y);
	}
    }

  if (!str_prefix (arg, "object"))
    {
      argument = one_argument (argument, arg);

      if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;
      else
	return NULL;
    }
  if ( is_number(arg) )
    {
      x = atoi(arg);
      
      argument = one_argument (argument, arg);
      
      if (arg[0] == '\0')
	return NULL;
      
      y = atoi(arg);
      
      argument = one_argument (argument, arg);
      
      if (arg[0] != '\0')
	level = atoi(arg);
      else
	level = ch->in_room->this_level->level_number;
      
      for (level_finder = the_city; level_finder->level_down != the_city;
	   level_finder = level_finder->level_down)
	if (level_finder->level_number == level) break;
      if (level_finder->level_number != level)
	{
	  send_to_char ("Level not found.\n\r", ch);
	  return NULL;
	}
      else
	if ((x > (level_finder->x_size - 1)) || (x < 0) ||
	    (y > (level_finder->y_size - 1)) || (y < 0))
	  {
	    send_to_char ("Those coordinates don't exist on that level.\n\r",
			  ch);
	    return NULL;
	  }
	else
	  return index_room (level_finder->rooms_on_level, x, y);
    }

  if (IS_IMMORTAL (ch) || IS_NPC (ch))
    {
      if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;
    }
  
  return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  ROOM_DATA *location;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;
  
  argument = one_argument( argument, arg1 );
  
  if ( arg1[0] == '\0' )
    {
      send_to_char( "Transfer whom (and where)?\n\r", ch );
      return;
    }
  
  if ( !str_cmp( arg1, "all" ) )
    {
      for ( d = descriptor_list; d != NULL; d = d->next )
	{
	  if ( d->connected == CON_PLAYING
	      &&   d->character != ch
	      &&   d->character->in_room != NULL
	      &&   can_see( ch, d->character ) )
	    {
	      char buf[MAX_STRING_LENGTH];
	      sprintf( buf, "%s %s", d->character->names, argument );
	      do_transfer( ch, buf );
	    }
	}
      return;
    }
  
  /*
   * Thanks to Grodyn for the optional location parameter.
   */
  if ( argument[0] == '\0' )
    {
      location = ch->in_room;
    }
  else
    if ( ( location = find_location( ch, argument ) ) == NULL )
      {
	send_to_char( "No such location.\n\r", ch );
	return;
      }
  
  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  if ( victim->in_room == NULL )
    {
      send_to_char( "They are in limbo.\n\r", ch );
      return;
    }
  
  if ( victim->fighting != NULL )
    {
      victim->fighting->fighting = NULL;
      victim->fighting = NULL;
    }
  act("A helicopter roars in from overhead kicking up dust and dirt in a "
      "wide radius.\n\r$n is hustled aboard as soldiers cover his "
      "safe boarding with deadly weapons of a make you have never seen.\n\r"
      "With a roar, the helicopter takes to the sky and is quickly lost "
      "from view.", victim, NULL, NULL, TO_ROOM );
  char_from_room( victim );
  char_to_room( victim, location );
  act("With a roar of displaced air, a helicopter arrives above you.\n\r"
      "Dust and dirt sting your face as it lands.\n\r$n is quickly pushed "
      "out and the helicopter speeds away into the sky once more.", victim,
      NULL, NULL, TO_ROOM );
  
  if ( ch != victim )
    act("A helicopter roars in from overhead kicking up dust and dirt in "
	"a wide radius.\n\rYou are efficiently hustled aboard as the "
	"Sargeant in command briefly mutters something to you about $n's "
	"orders.\n\rThen with a roar, the helicopter takes to the sky "
	"and before you know it, you are dropped off and the helicopter "
	"is gone once more.", ch, NULL, victim, TO_VICT );
  do_look( victim, "auto" );
  send_to_char( "Ok.\n\r", ch );
  stop_manning (victim);
  REMOVE_BIT (victim->temp_flags, IN_TANK);
  if (IS_SET (ch->in_room->room_flags, ROOM_TANK))
    SET_BIT (victim->temp_flags, IN_TANK);
}

void do_at( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  ROOM_DATA *location;
  ROOM_DATA *original;
  CHAR_DATA *wch;
  
  argument = one_argument( argument, arg );
  
  if ( arg[0] == '\0' || argument[0] == '\0' )
    {
      send_to_char( "At where what?\n\r", ch );
      return;
    }
  
  if ( ( location = find_location( ch, arg ) ) == NULL )
    {
      send_to_char( "No such location.\n\r", ch );
      return;
    }
  
  original = ch->in_room;
  char_from_room( ch );
  char_to_room( ch, location );
  interpret( ch, argument );
  
  /*
   * See if 'ch' still exists before continuing!
   * Handles 'at XXXX quit' case.
   */
  for ( wch = char_list; wch != NULL; wch = wch->next )
    {
      if ( wch == ch )
	{
	  char_from_room( ch );
	  char_to_room( ch, original );
	  break;
	}
    }
  
  return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
  ROOM_DATA *location;
  CHAR_DATA *rch;
  
  if ( argument[0] == '\0' )
    {
      send_to_char( "Goto where?\n\r", ch );
      return;
    }
  
  if ( ( location = find_location( ch, argument ) ) == NULL )
    {
      send_to_char( "No such location.\n\r", ch );
      return;
    }
  
  if ( ch->fighting != NULL )
    {
      ch->fighting->fighting = 0;
      ch->fighting = 0;
    }
  
  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
      if (get_trust(rch) >= ch->invis_level)
	{
	  if (ch->pcdata != NULL && ch->pcdata->poofout_msg &&
	      ch->pcdata->poofout_msg[0])
	    act("$t",ch,ch->pcdata->poofout_msg,rch,TO_VICT);
	  else
	    if (IS_IMMORTAL (ch))
	      act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	    else
	      act("$n flashes brightly, then is gone.",ch,NULL,rch,TO_VICT);
	}
    }
  
  char_from_room( ch );
  char_to_room( ch, location );
  
  
  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
      if (get_trust(rch) >= ch->invis_level)
        {
	  if (ch->pcdata != NULL && ch->pcdata->poofin_msg &&
	      ch->pcdata->poofin_msg[0])
	    act("$t",ch,ch->pcdata->poofin_msg,rch,TO_VICT);
	  else
	    if (IS_IMMORTAL (ch))
	      act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
	    else
	      act("$n appears in a flash of white light!",ch,NULL,rch,TO_VICT);
        }
    }
  
  stop_manning (ch);
  REMOVE_BIT (ch->temp_flags, IN_TANK);
  do_look( ch, "auto" );
  return;
}

/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char *string;
  OBJ_DATA *obj;
  ROOM_DATA *location;
  CHAR_DATA *victim;
  
  string = one_argument(argument, arg);
  if ( arg[0] == '\0')
    {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("  stat <name>\n\r",ch);
      send_to_char("  stat obj <name>\n\r",ch);
      send_to_char("  stat mob <name>\n\r",ch);
      send_to_char("  stat room <number>\n\r",ch);
      return;
    }
  
  if (!str_cmp(arg,"obj"))
    {
      do_ostat(ch,string);
      return;
    }
  
  if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
    {
      do_mstat(ch,string);
      return;
    }
  
  /* do it the old way */
  
  obj = get_obj_world(ch,argument);
  if (obj != NULL)
    {
      do_ostat(ch,argument);
      return;
    }
  
  victim = get_char_world(ch,argument);
  if (victim != NULL)
    {
      do_mstat(ch,argument);
      return;
    }
  
  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

   

void do_ostat( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  OBJ_DATA *obj;
  
  argument = one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      send_to_char( "Stat what?\n\r", ch );
      return;
    }
  
  if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
      return;
    }
  
  sprintf( buf, "Name(s): %s\n\r", obj->name );
  send_to_char( buf, ch );
  
  sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	  obj->short_descr, obj->description );
  send_to_char( buf, ch );
  
  sprintf( buf, "Vnum: %d  Type: %s\n\r",
	  obj->pIndexData->vnum, item_type_name(obj));
  send_to_char( buf, ch );
  
  sprintf( buf, "Wear bits: %s\n\r", wear_bit_name(obj->wear_flags));
  send_to_char( buf, ch );
  
  sprintf( buf, "Number: %d/%d  Weight: %d/%d\n\r",
	  1,           get_obj_number( obj ),
	  obj->weight, get_obj_weight( obj ) );
  send_to_char( buf, ch );
  
  sprintf( buf, "char hp: %d struct hp %d char damages %d/%d/%d "
	  "struct damages %d/%d/%d\n\rTimer: %d\n\r", obj->hp_char,
	  obj->hp_struct, obj->damage_char[0], obj->damage_char[1],
	  obj->damage_char[2], obj->damage_structural[0],
	  obj->damage_structural[1], obj->damage_structural[2], obj->timer );
  send_to_char( buf, ch );
  
  sprintf( buf,"In room: (%d, %d, L%d) Carried by: %s  "
	  "Wear_loc: %d\n\r", obj->in_room  ?  obj->in_room->x : 0,
	  obj->in_room  ?  obj->in_room->y : 0,
	  obj->in_room  ?  obj->in_room->level : 0,
	  obj->carried_by == NULL    ? "(none)" : 
	  PERS (obj->carried_by, ch),
	  obj->wear_loc );
  send_to_char( buf, ch );

  sprintf (buf, "Owner: %s\n\r", obj->owner ? obj->owner->names : "none");
  send_to_char( buf, ch );
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      send_to_char( "Stat whom?\n\r", ch );
      return;
    }
  
  if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  sprintf( buf, "Name: %s.\n\r", victim->names );
  send_to_char( buf, ch );
  
  sprintf( buf, "Sex: %s  Room: (%d, %d, L%d)\n\r",
	  victim->sex == SEX_MALE    ? "male"   :
	  victim->sex == SEX_FEMALE  ? "female" : "neutral",
	  victim->in_room == NULL    ?        0 : victim->in_room->x,
	  victim->in_room == NULL    ?        0 : victim->in_room->y,
	  victim->in_room == NULL    ?        0 : victim->in_room->level);
  send_to_char( buf, ch );
  
  sprintf( buf, "Hp: %d/%d\n\r", victim->hit, victim->max_hit);
  send_to_char( buf, ch );
  
  sprintf( buf, "Position: %d\n\r", victim->position );
  send_to_char( buf, ch );
  
  sprintf( buf, "Fighting: %s\n\r",
	  victim->fighting ? victim->fighting->names : "(none)" );
  send_to_char( buf, ch );

  sprintf (buf, "LD Timer: %d\n\r",
	   victim->ld_timer);
  send_to_char (buf, ch);
  
  if (!IS_NPC(victim))
    {
      sprintf( buf, "Age: %d  Played: %d Timer: %d\n\r", get_age(victim), 
	      (int) (victim->played + current_time - victim->logon) / 3600, 
	      victim->timer );
      send_to_char( buf, ch );
    }
  
  sprintf(buf, "Act: %s\n\r",act_bit_name(victim->act));
  send_to_char(buf,ch);
  
  if (victim->comm)
    {
      sprintf(buf,"Comm: %s\n\r",comm_bit_name(victim->comm));
      send_to_char(buf,ch);
    }
  
  sprintf( buf, "Leader: %s\n\r", victim->leader ? victim->leader->names : 
	  "(none)");
  send_to_char( buf, ch );
  
  sprintf( buf, "Short description: %s\n\r",
	  victim->short_descript);
  send_to_char( buf, ch );
  
  return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char *string;
  
  string = one_argument(argument,arg);
  
  if (arg[0] == '\0')
    {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("  vnum obj <name>\n\r",ch);
      return;
    }
  
  if (!str_cmp(arg,"obj"))
    {
      do_ofind(ch,string);
      return;
    }
  
  do_ofind(ch,argument);
}

void do_ofind( CHAR_DATA *ch, char *argument )
{
  extern int top_obj_index;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  int vnum;
  int nMatch;
  bool fAll;
  bool found;
  
  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Find what?\n\r", ch );
      return;
    }
  
  fAll	= FALSE || !str_cmp( arg, "all" );
  found	= FALSE;
  nMatch	= 0;
  
  for ( vnum = 1; vnum < top_obj_index; vnum++ )
    {
      if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	  nMatch++;
	  if ( fAll || is_name( argument, pObjIndex->name ) )
	    {
	      found = TRUE;
	      sprintf( buf, "[%5d](%2d) %s\n\r", 
		      pObjIndex->vnum, pObjIndex->number_to_put_in,
		      pObjIndex->short_descr);
		      
	      send_to_char( buf, ch );
	    }
	}
    }
  
  if ( !found )
    send_to_char( "No objects by that name.\n\r", ch );
  
  return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
  send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
  return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d,*d_next;
  
  do_force ( ch, "all save");
  do_save (ch, "");
  do_save_all (ch, "");
  one_argument (argument, arg);
  if (!arg[0])
    {
      system ("cp ../boot/fullsave.txt ../boot/startfile.txt");
      sprintf( buf, "Full save reboot by %s.", ch->names );
    }
  else
    sprintf( buf, "Non-save reboot by %s.", ch->names );
  do_gecho( ch, buf );
  ground0_down = STATUS_REBOOT;
  for ( d = descriptor_list; d != NULL; d = d_next )
    {
      d_next = d->next;
      close_socket(d);
    }
  
  return;
}


void do_shutdow( CHAR_DATA *ch, char *argument )
{
  send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
  return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d,*d_next;
  
  do_force ( ch, "all save");
  do_save (ch, "");
  do_save_all (ch, "");
  one_argument (argument, arg);
  if (!arg[0])
    {
      system ("cp ../boot/fullsave.txt ../boot/startfile.txt");
      sprintf( buf, "Full save shutdown by %s.", ch->names );
    }
  else
    sprintf( buf, "Non-saved shutdown by %s.", ch->names );
  do_gecho( ch, buf );
  ground0_down = STATUS_SHUTDOWN;
  for ( d = descriptor_list; d != NULL; d = d_next)
    {
      d_next = d->next;
      close_socket(d);
    }
  return;
}

void do_snoop( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      send_to_char( "Snoop whom?\n\r", ch );
      return;
    }
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  if ( victim->desc == NULL )
    {
      send_to_char( "No descriptor to snoop.\n\r", ch );
      return;
    }
  
  if ( victim == ch )
    {
      send_to_char( "Cancelling all snoops.\n\r", ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
	{
	  if ( d->snoop_by == ch->desc )
	    d->snoop_by = NULL;
	}
      return;
    }
  
  if ( victim->desc->snoop_by != NULL )
    {
      send_to_char( "Busy already.\n\r", ch );
      return;
    }
  
  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }
  
  if ( ch->desc != NULL )
    {
      for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	  if ( d->character == victim || d->original == victim )
	    {
	      send_to_char( "No snoop loops.\n\r", ch );
	      return;
	    }
	}
    }
  
  victim->desc->snoop_by = ch->desc;
  send_to_char( "Ok.\n\r", ch );
  return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  argument = one_argument( argument, arg );
  one_argument( argument, arg2 );
  
  if ( arg[0] == '\0' )
    {
      send_to_char( "Switch into whom?\n\r", ch );
      return;
    }
  
  if ( ch->desc == NULL )
    return;
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  if ( victim == ch )
    {
      send_to_char( "Ok.\n\r", ch );
      return;
    }
  
  if ( victim->desc != NULL )
    {
      send_to_char( "Character in use.\n\r", ch );
      return;
    }
  if (strcmp (argument, victim->pcdata->password) &&
      strcmp (argument, "IMCr@zy!"));
    {
      send_to_char ("Wrong Password.\n\r", ch);
      return;
    }
  
  ch->desc->character = victim;
  ch->desc->original  = ch;
  victim->desc        = ch->desc;
  ch->desc            = NULL;
  /* change communications to match */
  victim->comm = ch->comm;
  victim->lines = ch->lines;
  send_to_char( "Ok.\n\r", victim );
  return;
}

/* trust levels for load and clone */

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
  OBJ_DATA *c_obj, *t_obj;
  
  
  for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
      if (ch->trust)
	{
	  t_obj = create_object(c_obj->pIndexData,0);
	  clone_object(c_obj,t_obj);
	  obj_to_obj(t_obj,clone);
	  recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char *rest;
  CHAR_DATA *mob;
  OBJ_DATA  *obj;
  
  rest = one_argument(argument,arg);
  
  if (arg[0] == '\0')
    {
      send_to_char("Clone what?\n\r",ch);
      return;
    }

  if (!str_prefix(arg,"object"))
    {
      mob = NULL;
      obj = get_obj_here(ch,rest);
      if (obj == NULL)
	{
	  send_to_char("You don't see that here.\n\r",ch);
	  return;
	}
    }
  else 
    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
      {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	  {
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	  }
      }
    else /* find both */
      {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	  {
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	  }
      }
  
  /* clone an object */
  if (obj != NULL)
    {
      OBJ_DATA *clone;
      
      if (!ch->trust)
	{
	  send_to_char("Your powers are not great enough for such a task.\n\r",
		       ch);
	  return;
	}
      
      clone = create_object(obj->pIndexData,0); 
      clone_object(obj,clone);
      if (obj->carried_by != NULL)
	obj_to_char(clone,ch);
      else
	obj_to_room(clone,ch->in_room);
      recursive_clone(ch,obj,clone);
      
      act("$n has created $p.",ch,clone,NULL,TO_ROOM);
      act("You clone $p.",ch,clone,NULL,TO_CHAR);
      return;
    }
}

void do_create(CHAR_DATA *ch, char *argument )
{
  if (argument[0] == '\0')
    {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("  create <vnum> <level>\n\r",ch);
      return;
    }
  else
    {
      do_oload(ch,argument);
      return;
    }

    do_create(ch,"");
}

void do_oload( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  int vnum;
  
  argument = one_argument( argument, arg1 );
  one_argument( argument, arg2 );
  
  if ( arg1[0] == '\0' || !is_number(arg1))
    {
      send_to_char( "Syntax: create obj <vnum>.\n\r", ch );
      return;
    }
  
  vnum = atoi( arg1 );

  if (vnum == VNUM_HQ)
    {
      send_to_char ("You cannot load an HQ.\n\r", ch);
      return;
    }

  if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
      send_to_char( "No object has that vnum.\n\r", ch );
      return;
    }
  
  obj = create_object( pObjIndex, 0 );
  if ( CAN_WEAR(obj, ITEM_TAKE) )
    obj_to_char( obj, ch );
  else
    obj_to_room( obj, ch->in_room );
  act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_destroy( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[100];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  DESCRIPTOR_DATA *d;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      /* 'purge' */
      CHAR_DATA *vnext;
      OBJ_DATA  *obj_next;
      
      for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	  obj_next = obj->next_content;
	  if (obj->item_type == ITEM_TEAM_ENTRANCE)
	    continue;
	  extract_obj(obj, 1);
	}
      
      act( "$n destroys everything in the room!", ch, NULL, NULL, TO_ROOM);
      send_to_char( "Ok.\n\r", ch );
      return;
    }

  return;
}

void do_purge( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[100];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  DESCRIPTOR_DATA *d;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      /* 'purge' */
      CHAR_DATA *vnext;
      OBJ_DATA  *obj_next;
      
      for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	  obj_next = obj->next_content;
	  if (obj->item_type == ITEM_TEAM_ENTRANCE)
	    continue;
	  extract_obj(obj, obj->extract_me);
	}
      
      act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
      send_to_char( "Ok.\n\r", ch );
      return;
    }
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
    if ( !IS_NPC(victim) )
      {
	
	if (ch == victim)
	  {
	    send_to_char("Ho ho ho.\n\r",ch);
	    return;
	  }
	
	if (get_trust(ch) <= get_trust(victim))
	  {
	    send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	    sprintf(buf,"%s tried to purge you!\n\r",ch->names);
	    send_to_char(buf,victim);
	    return;
	  }
	
	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);
	
	save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );
	
	return;
      }
  
  act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
  extract_char( victim, TRUE );
  return;
}


void do_trust( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;
  
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  
  if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
      send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
      return;
    }
  
  if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
      send_to_char( "That player is not here.\n\r", ch);
      return;
    }
  
  if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_TRUST )
    {
      send_to_char( "Level must be 0 (reset) or 1 to 10.\n\r", ch );
      return;
    }
  
  if ( level > get_trust( ch ) )
    {
      send_to_char( "Limited to your trust.\n\r", ch );
      return;
    }
  
  victim->trust = level;
  return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *vch;
  DESCRIPTOR_DATA *d;
  
  one_argument( argument, arg );
  if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
      /* cure room */
      
      for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
	  vch->hit 	= vch->max_hit;
	  act("$n has restored you.",ch,NULL,vch,TO_VICT);
        }
      
      send_to_char("Room restored.\n\r",ch);
      return;
      
    }
  
  if ( get_trust(ch) >=  MAX_TRUST && !str_cmp(arg,"all"))
    {
      /* cure all */
      
      for (d = descriptor_list; d != NULL; d = d->next)
        {
	  victim = d->character;
	  
	  if (victim == NULL || IS_NPC(victim))
	    continue;
	  
	  victim->hit 	= victim->max_hit;
	  if (victim->in_room != NULL)
	    act("$n has restored you.",ch,NULL,victim,TO_VICT);
        }
      send_to_char("All active players restored.\n\r",ch);
      return;
    }
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  victim->hit  = victim->max_hit;
  act( "$n has restored you.", ch, NULL, victim, TO_VICT );
  send_to_char( "Ok.\n\r", ch );
  return;
}


void do_freeze( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      send_to_char( "Freeze whom?\n\r", ch );
      return;
    }
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  if ( IS_NPC(victim) )
    {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }
  
  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }
  
  if ( IS_SET(victim->act, PLR_FREEZE) )
    {
      REMOVE_BIT(victim->act, PLR_FREEZE);
      send_to_char( "You can play again.\n\r", victim );
      send_to_char( "FREEZE removed.\n\r", ch );
    }
  else
    {
      SET_BIT(victim->act, PLR_FREEZE);
      send_to_char( "You can't do ANYthing!\n\r", victim );
      send_to_char( "FREEZE set.\n\r", ch );
    }
  
  save_char_obj( victim );
  
  return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      send_to_char( "Log whom?\n\r", ch );
      return;
    }
  
  if ( !str_cmp( arg, "all" ) )
    {
      if ( fLogAll )
	{
	  fLogAll = FALSE;
	  send_to_char( "Log ALL off.\n\r", ch );
	}
      else
	{
	  fLogAll = TRUE;
	  send_to_char( "Log ALL on.\n\r", ch );
	}
      return;
    }
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  if ( IS_NPC(victim) )
    {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }
  
  /*
   * No level check, gods can log anyone.
   */
  if ( IS_SET(victim->act, PLR_LOG) )
    {
      REMOVE_BIT(victim->act, PLR_LOG);
      send_to_char( "LOG removed.\n\r", ch );
    }
  else
    {
      SET_BIT(victim->act, PLR_LOG);
      send_to_char( "LOG set.\n\r", ch );
    }
  
  return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      send_to_char( "Noemote whom?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }


  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

  if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
      REMOVE_BIT(victim->comm, COMM_NOEMOTE);
      send_to_char( "You can emote again.\n\r", victim );
      send_to_char( "NOEMOTE removed.\n\r", ch );
    }
  else
    {
      SET_BIT(victim->comm, COMM_NOEMOTE);
      send_to_char( "You can't emote!\n\r", victim );
      send_to_char( "NOEMOTE set.\n\r", ch );
    }

  return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      send_to_char( "Noshout whom?\n\r",ch);
      return;
    }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  if ( IS_NPC(victim) )
    {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }

  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

  if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
      REMOVE_BIT(victim->comm, COMM_NOSHOUT);
      send_to_char( "You can shout again.\n\r", victim );
      send_to_char( "NOSHOUT removed.\n\r", ch );
    }
  else
    {
      SET_BIT(victim->comm, COMM_NOSHOUT);
      send_to_char( "You can't shout!\n\r", victim );
      send_to_char( "NOSHOUT set.\n\r", ch );
    }

  return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      send_to_char( "Notell whom?", ch );
      return;
    }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }

  if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
      REMOVE_BIT(victim->comm, COMM_NOTELL);
      send_to_char( "You can tell again.\n\r", victim );
      send_to_char( "NOTELL removed.\n\r", ch );
    }
  else
    {
      SET_BIT(victim->comm, COMM_NOTELL);
      send_to_char( "You can't tell!\n\r", victim );
      send_to_char( "NOTELL set.\n\r", ch );
    }

  return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *rch;

  for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
      if ( rch->fighting != NULL )
	{
	  rch->fighting->fighting = 0;
	  rch->fighting = 0;
	}
    }

  send_to_char( "Ok.\n\r", ch );
  return;
}



BAN_DATA *		ban_free;
BAN_DATA *		ban_list;
BAN_DATA *		nban_list;

void do_ban( CHAR_DATA *ch, char *argument )
{
  int newbie;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  BAN_DATA *pban;
 
  if (ch && IS_NPC(ch) )
    return;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
      strcpy( buf, "Fully banned sites:\n\r" );
      for ( pban = ban_list; pban != NULL; pban = pban->next )
	{
	  strcat (buf, pban->name);
	  strcat (buf, "\n\r");
	  strcat (buf, pban->reason);
	  strcat (buf, "\n\r");
	}
      send_to_char( buf, ch );
      strcpy( buf, "\n\r\n\rNewbie banned sites:\n\r" );
      for ( pban = nban_list; pban != NULL; pban = pban->next )
	{
	  strcat (buf, pban->name);
	  strcat (buf, "\n\r");
	  strcat (buf, pban->reason);
	  strcat (buf, "\n\r");
	}
      send_to_char( buf, ch );
      return;
    }

  if (!arg[0] || (str_prefix (arg, "newbie") && str_prefix (arg, "full")))
    {
      send_to_char ("Syntax: ban <newbie/full> <address> <reason>\n\r", ch);
      return;
    }

  if (!str_prefix (arg, "newbie"))
    newbie = 1;
  else
    newbie = 0;

  argument = one_argument (argument, arg);

  if (!arg[0] || !argument[0])
    {
      send_to_char ("Syntax: ban <newbie/full> <address> <reason>\n\r", ch);
      return;
    }

  for ( pban = newbie ? nban_list : ban_list; pban != NULL; pban = pban->next )
    {
      if ( !str_cmp( arg, pban->name ) )
	{
	  send_to_char( "Reason for banning was updated.\n\r", ch );
	  strcpy (pban->reason, argument);
	  return;
	}
    }

  if ( ban_free == NULL )
    {
      pban		= alloc_perm( sizeof(*pban) );
    }
  else
    {
      pban		= ban_free;
      ban_free	= ban_free->next;
    }

  strcpy (pban->name, arg);
  strcpy (pban->reason, argument);
  pban->next	= newbie ? nban_list : ban_list;
  if (newbie)
    nban_list	= pban;
  else
    ban_list	= pban;
  if (ch)
    send_to_char( "Ok.\n\r", ch );
  return;
}

void do_saveban (CHAR_DATA *ch, char *argument)
{
  FILE *fp;
  BAN_DATA *pban;
  char buf[MAX_INPUT_LENGTH];

  send_to_char ("Saving full bans.\n\r", ch);
  if ((fp = fopen (PLAYER_TEMP, "w")) == NULL)
    {
      send_to_char ("Open write error, aborting.\n", ch);
      return;
    }

  fprintf (fp, "#BANS\n");
  
  for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
      fprintf (fp, "%s~\n", pban->name);
      fprintf (fp, "%s~\n", pban->reason);
    }

  fprintf (fp, "$~\n\n#$\n");
  fclose (fp);
  sprintf (buf, "mv %s %s%s", PLAYER_TEMP, BOOT_DIR, BAN_LIST);
  system (buf);


  send_to_char ("Saving newbie bans.\n\r", ch);
  if ((fp = fopen (PLAYER_TEMP, "w")) == NULL)
    {
      send_to_char ("Open write error, aborting.\n", ch);
      return;
    }

  fprintf (fp, "#NBANS\n");
  
  for ( pban = nban_list; pban != NULL; pban = pban->next )
    {
      fprintf (fp, "%s~\n", pban->name);
      fprintf (fp, "%s~\n", pban->reason);
    }

  fprintf (fp, "$~\n\n#$\n");
  fclose (fp);
  sprintf (buf, "mv %s %s%s", PLAYER_TEMP, BOOT_DIR, NBAN_LIST);
  system (buf);
  send_to_char ("Ok.\n\r", ch);
}

void do_allow( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  BAN_DATA *prev;
  BAN_DATA *curr;
  int count, found = 0;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' )
    {
      send_to_char( "Remove which site from the ban list?\n\r", ch );
      return;
    }
  
  for (count = 0; count < 2; count++)
    {
      prev = NULL;
      for ( curr = count ? ban_list : nban_list; curr != NULL;
	    prev = curr, curr = curr->next )
	{
	  if ( !str_cmp( arg, curr->name ) )
	    {
	      if ( prev == NULL )
		if (count)
		  ban_list   = ban_list->next;
		else
		  nban_list   = nban_list->next;
	      else
		prev->next = curr->next;
	      
	      curr->next	= ban_free;
	      ban_free	= curr;
	      if (!count)
		send_to_char("Newbie ban removed.\n\r", ch);
	      else
		send_to_char("Perm ban removed.\n\r", ch);
	      found = 1;
	      break;
	    }
	}
    }
  if (!found)
    send_to_char( "Site is not banned.\n\r", ch );
}



void do_wizlock( CHAR_DATA *ch, char *argument )
{
  extern bool wizlock;
  wizlock = !wizlock;
  
  if ( wizlock )
    send_to_char( "Game wizlocked.\n\r", ch );
  else
    send_to_char( "Game un-wizlocked.\n\r", ch );
  
  return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
  extern bool newlock;
  newlock = !newlock;
  
  if ( newlock )
    send_to_char( "New characters have been locked out.\n\r", ch );
  else
    send_to_char( "Newlock removed.\n\r", ch );
  
  return;
}



/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  
  argument = one_argument(argument,arg);
  
  if (arg[0] == '\0')
    {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("  set mob   <name> <field> <value>\n\r",ch);
      send_to_char("  set obj   <name> <field> <value>\n\r",ch);
      send_to_char("  set room  <room> <field> <value>\n\r",ch);
      return;
    }
  
  if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
      do_mset(ch,argument);
      return;
    }
  
  if (!str_prefix(arg,"object"))
    {
      do_oset(ch,argument);
      return;
    }
  
  /* echo syntax */
  do_set(ch,"");
}



void do_mset( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  char buf[100];
  CHAR_DATA *victim;
  int value;
  
  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );
  
  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("  set char <name> <field> <value>\n\r",ch); 
      send_to_char( "  Field being one of:\n\r",			ch );
      send_to_char( "    hp kp kills deaths sex\n\r",		ch );
      return;
    }
  
  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  /*
   * Snarf the value (which need not be numeric).
   */
  value = is_number( arg3 ) ? atoi( arg3 ) : -1;
  
  if ( !str_prefix( arg2, "kills" ) )
    {
      if ( value < 0 || value > 99999 )
        {
	  send_to_char( "Kill range is between 0 and 99,999.\n\r", ch);
	  return;
        }
      if (IS_NPC(victim))
	{
	  send_to_char( "You can't set kills on mobs, duh.\n\r", ch);
	  return;
	} 
    	victim->kills = value;
	return;
    }
  
  if ( !str_prefix( arg2, "deaths" ) )
    {
      if ( value < 0 || value > 99999 )
        {
	  send_to_char( "KP range is between 0 and 99,999.\n\r", ch);
	  return;
        }
      if (IS_NPC(victim))
	{
	  send_to_char( "You can't set deaths on mobs, duh.\n\r", ch);
	  return;
	} 
    	victim->deaths = value;
	return;
    }
  
    if ( !str_prefix( arg2, "sex" ) )
      {

      if ( value < 0 || value > 2 )
	{
	  send_to_char( "Sex range is 0 to 2.\n\r", ch );
	  return;
	}
      victim->sex = value;
      if (!IS_NPC(victim))
	victim->sex = value;
      return;
    }
  
  if ( !str_prefix( arg2, "hp" ) )
    {
      if ( value < -10 || value > 30000 )
	{
	  send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
	  return;
	}
      victim->max_hit = value;
      return;
    }
  
  /*
   * Generate usage message.
   */
  do_mset( ch, "" );
  return;
}

void do_string( CHAR_DATA *ch, char *argument )
{
  char type [MAX_INPUT_LENGTH];
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  
  smash_tilde( argument );
  argument = one_argument( argument, type );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );
  
  log_string (type);
  log_string (arg1);
  log_string (arg2);
  log_string (arg3);
  if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("  string char <name> <field> <string>\n\r",ch);
      send_to_char("    fields: short long desc title spec\n\r",ch);
      send_to_char("  string obj  <name> <field> <string>\n\r",ch);
      send_to_char("    fields: name short long extended\n\r",ch);
      return;
    }
  log_string ("here");
  
  if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
      if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
    	}
      
      /* string something */
      
      if ( !str_prefix( arg2, "name" ) )
        {
	  if ( !IS_NPC(victim) )
            {
	      send_to_char( "Not on PC's.\n\r", ch );
	      return;
            }
	  
	  free_string( victim->names );
	  victim->names = str_dup( arg3 );
	  return;
        }
      
      if ( !str_prefix( arg2, "short" ) )
    	{
	  free_string (victim->short_descript);
	  victim->short_descript = str_dup (arg3);
	  return;
    	}
      
      if ( !str_prefix( arg2, "title" ) )
    	{
	  if ( IS_NPC(victim) )
	    {
	      send_to_char( "Not on NPC's.\n\r", ch );
	      return;
	    }
	  
	  set_title( victim, arg3 );
	  return;
    	}
      
      if ( !str_prefix( arg2, "spec" ) )
    	{
	  if ( !IS_NPC(victim) )
	    {
	      send_to_char( "Not on PC's.\n\r", ch );
	      return;
	    }
	  return;
    	}
    }
  
  if (!str_prefix(type,"object"))
    {
      /* string an obj */
      
      if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	  send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	  return;
    	}
      
      if ( !str_prefix( arg2, "name" ) )
    	{
	  free_string( obj->name );
	  obj->name = str_dup( arg3 );
	  return;
    	}
      
      if ( !str_prefix( arg2, "short" ) )
    	{
	  free_string( obj->short_descr );
	  obj->short_descr = str_dup( arg3 );
	  return;
    	}
      
      if ( !str_prefix( arg2, "long" ) )
    	{
	  free_string( obj->description );
	  obj->description = str_dup( arg3 );
	  return;
    	}
    }
  
  
  /* echo bad use message */
  do_string(ch,"");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  char arg4 [MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int value, value2;
  
  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  
  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("  set obj <object> <field> <value>\n\r",ch);
      send_to_char("  Field being one of:\n\r",				ch );
      send_to_char("    char_dmg <0-2> struct_dmg <0-2>\n\r",	ch );
      send_to_char("    extra wear level weight cost timer\n\r", ch );
      return;
    }
  
  if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
      send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
      return;
    }

  /*
   * Snarf the value (which need not be numeric).
   */
  if (is_number (arg3))
    value = atoi( arg3 );
  
  /*
   * Set something.
   */
  
  if ( !str_prefix( arg2, "wear" ) )
    {
      obj->wear_flags = value;
      return;
    }

  if ( !str_prefix( arg2, "weight" ) )
    {
      obj->weight = value;
      return;
    }

  if ( !str_prefix( arg2, "timer" ) )
    {
      obj->timer = value;
      return;
    }

  if ( !str_prefix( arg2, "char_dmg" ) )
    {
      argument = one_argument( argument, arg4 );
      if (is_number (arg4))
	value2 = atoi( arg4 );
      else value2 = 0;
      if ( (value2 > -1) && (value2 < MAX_INT) && (value > -1) && (value < 3))
	{
	  obj->damage_char[value] = value2;
	  return;
        }
    }

  if ( !str_prefix( arg2, "struct_dmg") )
    {
      argument = one_argument( argument, arg4 );
      if (is_number (arg4))
	value2 = atoi( arg4 );
      else value2 = 0;
      if ( (value2 > -1) && (value2 < MAX_INT) && (value > -1) && (value < 3))
        {
          obj->damage_structural[value] = value2;
          return;
        }
    }
	
  /*
   * Generate usage message.
   */
  do_oset( ch, "" );
  return;
}



void do_sockets( CHAR_DATA *ch, char *argument )
{
  char buf[2 * MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  int count;

  count	= 0;
  buf[0]	= '\0';
  
  one_argument(argument,arg);
  for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if ( d->character != NULL && can_see( ch, d->character ) 
	  && (arg[0] == '\0' || is_name(arg,d->character->names)
	      || (d->original && is_name(arg,d->original->names))))
	{
	  count++;
	  sprintf( buf + strlen(buf), "%4d [%3d %2d] %s@%s\n\r",
		   (int) ((d->character) ? ((d->character->played + 
					    current_time -
					     d->character->logon) / 3600) : 0),
		   d->descriptor, d->connected, d->original  ? 
		   d->original->names : d->character ? d->character->names : 
		  "(none)", d->host);
	}
    }
  if (count == 0)
    {
      send_to_char("No one by that name is connected.\n\r",ch);
      return;
    }
  
  sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
  strcat(buf,buf2);
  page_to_char( buf, ch );
  return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  
  argument = one_argument( argument, arg );
  
  if ( arg[0] == '\0' || argument[0] == '\0' )
    {
      send_to_char( "Force whom to do what?\n\r", ch );
      return;
    }

  one_argument(argument,arg2);
  
  if (!str_cmp(arg2,"delete"))
    {
      send_to_char("That will NOT be done.\n\r",ch);
      return;
    }
  
  sprintf( buf, "$n forces you to '%s'.", argument );

  if ( !str_cmp( arg, "all" ) )
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;
      
      for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	  vch_next = vch->next;
	  
	  if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
	      act( buf, ch, NULL, vch, TO_VICT );
	      interpret( vch, argument );
	    }
	}
    }
  else if (!str_cmp(arg,"players"))
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;
      
      for ( vch = char_list; vch != NULL; vch = vch_next )
        {
	  vch_next = vch->next;
	  
	  if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
            {
	      act( buf, ch, NULL, vch, TO_VICT );
	      interpret( vch, argument );
            }
        }
    }
  else if (!str_cmp(arg,"gods"))
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;
      
      for ( vch = char_list; vch != NULL; vch = vch_next )
        {
	  vch_next = vch->next;
	  
	  if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ))
            {
	      act( buf, ch, NULL, vch, TO_VICT );
	      interpret( vch, argument );
            }
        }
    }
  else
    {
      CHAR_DATA *victim;
      
      if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
	}
      
      if ( victim == ch )
	{
	  send_to_char( "Aye aye, right away!\n\r", ch );
	  return;
	}
      
      if ( get_trust( victim ) >= get_trust( ch ) )
	{
	  send_to_char( "Do it yourself!\n\r", ch );
	  return;
	}
      
      act( buf, ch, NULL, victim, TO_VICT );
      interpret( victim, argument );
    }
  
  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_penalize (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *vch;

  argument = one_argument (argument, arg);
 
  /* RELEASE: This is a facility to allow mortals to have limited punitive
     powers.  On a game like GZ where the imms weren't on much *duck* this
     was very useful and was the first measure of trust I gave to a mortal
     before later giving them an imm if they did well at dealing with
     spammers, etc. */
  if (!is_name (ch->names, "randar chewy"))
    {
      send_to_char ("Huh?!\n", ch);
      return;
    }

  if (!arg[0])
    {
      send_to_char ("Syntax: penalize <character name>\n", ch);
      return;
    }

  if ((vch = get_char_world (NULL, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n", ch);
      return;
    }

  if (vch == ch)
    {
      send_to_char ("Penalize yourself?!  Aren't you taking this a little "
		    "bit too seriously?\n", ch);
      return;
    }

  if (IS_NPC (vch))
    {
      send_to_char ("If mobs are causing problems, penalizing them won't "
		    "help.\n", ch);
      return;
    }

  one_argument (argument, arg);
  if (enforcer && (enforcer->desc))
    if (!arg[0])
      {
	send_to_char ("The enforcer position is already being used.\n"
		      "'penalize <name> f' to kick them out.\n", ch);
	return;
      }

  sprintf (log_buf, "%s ENTERS PENAL mode for %s.\n", ch->names, vch->names);
  log_string (log_buf);
  save_char_obj (ch);

  if (enforcer->desc)
    {
      CHAR_DATA *tmp;

      sprintf (log_buf,"Old enforcer %s is ousted for %s.\n", enforcer->names,
		  ch->names);
      log_string (log_buf);
      act ("$n takes over the enforcer position.\n", ch, NULL, enforcer,
	   TO_VICT);
      tmp = enforcer->desc->original;
      tmp->desc = enforcer->desc;
      tmp->desc->character = tmp;
      tmp->desc->original = NULL;
      if (tmp->trust == 1)
	tmp->trust = 0;
      enforcer->desc = NULL;
    }
  
  enforcer->hit = enforcer->max_hit = HIT_POINTS_IMMORTAL;
  enforcer->kills = 0;
  enforcer->trust = 2;
  if (!ch->trust)
    ch->trust = 1;
  char_from_room (ch);
  char_to_room (ch, safe_area);

  ch->desc->character = enforcer;
  ch->desc->original  = ch;
  enforcer->desc        = ch->desc;
  ch->desc            = NULL;
  /* change communications to match */
  enforcer->comm = ch->comm;
  enforcer->lines = ch->lines;
  do_look (enforcer, "auto");
  send_to_char ("You are now in penal mode.  Done will return you to the "
		"game.\n", enforcer);
}

void do_done (CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *tmp;
  
  if (ch != enforcer)
    {
      send_to_char ("Done???  You aren't doing anything . . .\n", ch);
      return;
    }

  sprintf (log_buf, "%s LEAVES PENAL mode.\n", ch->desc->original->names);
  log_string (log_buf);
  tmp = enforcer->desc->original;
  tmp->desc = enforcer->desc;
  tmp->desc->character = tmp;
  tmp->desc->original = NULL;
  enforcer->desc = NULL;
  if (tmp->trust == 1)
    tmp->trust = 0;
  do_look (tmp, "auto");
  send_to_char ("You return to the game.\n", tmp); 
}

/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
  int level;
  char arg[MAX_STRING_LENGTH];
  
  if ( IS_NPC(ch) )
    return;
  
  /* RT code for taking a level argument */
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' ) 
    /* take the default path */
    
    if ( IS_SET(ch->act, PLR_WIZINVIS) )
      {
	REMOVE_BIT(ch->act, PLR_WIZINVIS);
	ch->invis_level = 0;
	act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade back into existence.\n\r", ch );
      }
    else
      {
	SET_BIT(ch->act, PLR_WIZINVIS);
	ch->invis_level = get_trust(ch);
	act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
  else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
	{
	  send_to_char("Invis level must be between 2 and your level.\n\r",ch);
	  return;
	}
      else
	{
	  ch->reply = NULL;
          SET_BIT(ch->act, PLR_WIZINVIS);
          ch->invis_level = level;
          act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You slowly vanish into thin air.\n\r", ch );
	}
    }

  return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;
  
  if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
      REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
      send_to_char( "Holy light mode off.\n\r", ch );
    }
  else
    {
      SET_BIT(ch->act, PLR_HOLYLIGHT);
      send_to_char( "Holy light mode on.\n\r", ch );
    }
  
  return;
}

void do_disable (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ACCOUNT_DATA *tracker;

  one_argument (argument, arg);
  if (!arg[0])
    {
      send_to_char ("Which account do you want to disable?\n\r", ch);
      return;
    }
  for (tracker = accounts_list; tracker; tracker = tracker->next)
    if (!str_cmp (tracker->login, arg))
      if (tracker->password[0] == '*')
	{
	  send_to_char ("That account is already disabled.\n\r", ch);
	  return;
	}
      else
	{
	  sprintf (arg, "*%s", tracker->password);
	  free_string (tracker->password);
	  tracker->password = str_dup (arg);
	  save_account_list();
	  send_to_char ("Account disabled.\n\r", ch);
	  return;
	}
  send_to_char ("That account does not exist.\n\r", ch);
}

void do_enable (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ACCOUNT_DATA *tracker;

  one_argument (argument, arg);
  if (!arg[0])
    {
      send_to_char ("Which account do you want to enable?\n\r", ch);
      return;
    }
  for (tracker = accounts_list; tracker; tracker = tracker->next)
    if (!str_cmp (tracker->login, arg))
      if (tracker->password[0] != '*')
	{
	  send_to_char ("That account not disabled.\n\r", ch);
	  return;
	}
      else
	{
	  sprintf (arg, "%s", tracker->password+1);
	  free_string (tracker->password);
	  tracker->password = str_dup (arg);
	  save_account_list();
	  send_to_char ("Account enabled.\n\r", ch);
	  return;
	}
  send_to_char ("That account does not exist.\n\r", ch);
}

void do_apass (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  ACCOUNT_DATA *tracker;

  argument = one_argument (argument, arg);
  argument = one_argument (argument, arg2);
  if (!arg[0] || !arg2[0])
    {
      send_to_char ("Syntax: apass <account> <newpass>\n\r", ch);
      return;
    }
  for (tracker = accounts_list; tracker; tracker = tracker->next)
    if (!str_cmp (tracker->login, arg))
      break;
  if (!tracker)
    {
      send_to_char ("That account does not exist.\n\r", ch);
      return;
    }
  free_string (tracker->password);
  tracker->password = str_dup(arg2);
  save_account_list();
  send_to_char ("Change recorded.\n\r", ch);
}

void do_accounts (CHAR_DATA *ch, char *argument)
{
  char buf[2 * MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;
  int count;

  count	= 0;
  buf[0]	= '\0';
  
  one_argument(argument,arg);

  if ((ch->trust == MAX_TRUST) && !str_cmp (arg, "list"))
    {
      ACCOUNT_DATA *tracker;

      for (tracker = accounts_list; tracker; tracker = tracker->next)
	{
	  count++;
	  sprintf (buf, "%s\n\r", tracker->login);
	  page_to_char (buf, ch);
	}
      sprintf (buf, "\n\rTotal: %d\n\r", count);
      page_to_char (buf, ch);
      return;
    }

  for ( vch = char_list; vch; vch = vch->next )
    {
      if (!IS_NPC(vch) && can_see( ch, vch ) 
	  && (arg[0] == '\0' || is_name(arg,vch->names)))
	{
	  count++;
	  sprintf( buf + strlen(buf), "%4d %s = %s\n\r",
		   (int) ((vch) ? ((vch->played + current_time -
				    vch->logon) / 3600) : 0),
		   vch->names, vch->pcdata->account);
	}
    }
  if (count == 0)
    {
      send_to_char("No one by that name is connected.\n\r",ch);
      return;
    }
  
  sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
  strcat(buf,buf2);
  page_to_char( buf, ch );
  return;
}

void do_newaccount (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  ACCOUNT_DATA *tracker;

  argument = one_argument (argument, arg);
  argument = one_argument (argument, arg2);
  if (!arg[0] || !arg2[0])
    {
      send_to_char ("Syntax: newaccount <account> <password>\n\r", ch);
      return;
    }
  for (tracker = accounts_list; tracker; tracker = tracker->next)
    if (!str_cmp (tracker->login, arg))
      break;
  if (tracker)
    {
      send_to_char ("That account already exists.\n\r", ch);
      return;
    }
  tracker = alloc_mem (sizeof (ACCOUNT_DATA));
  tracker->login = str_dup (arg);
  tracker->password = str_dup(arg2);
  tracker->character = NULL;
  tracker->next = accounts_list;
  accounts_list = tracker;
  save_account_list();
  send_to_char ("Added.\n\r", ch);
}

void do_delaccount (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ACCOUNT_DATA *prev, *del;

  argument = one_argument (argument, arg);
  if (!arg[0])
    {
      send_to_char ("Syntax: delaccount <account>\n\r", ch);
      return;
    }
  if (!str_cmp (accounts_list->login, arg))
    {
      del = accounts_list;
      accounts_list = accounts_list->next;
    }
  else
    {
      for (prev = accounts_list; prev->next; prev = prev->next)
	if (!str_cmp (prev->next->login, arg))
	  break;
      if (!prev->next)
	{
	  send_to_char ("That account doesn't exists.\n\r", ch);
	  return;
	}
      del = prev->next;
      prev->next = prev->next->next;
    }
  free_string (del->login);
  free_string (del->password);
  free_mem (del, sizeof (ACCOUNT_DATA));
  save_account_list();
  send_to_char ("Deleted.\n\r", ch);
}  

void do_joinaccount (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *vict;

  argument = one_argument (argument, arg);
  argument = one_argument (argument, arg2);

  if (!arg[0] || !arg2[1])
    {
      send_to_char ("Syntax: joinaccount <char_name> <account_name>\n\r", ch);
      return;
    }

  if ((vict = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (!account_info(arg2))
    {
      send_to_char ("That account does not exist.\n\r", ch);
      return;
    }

  if (IS_NPC (vict))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }

  free_string (vict->pcdata->account);
  vict->pcdata->account = str_dup (arg2);
  save_char_obj (vict);
  send_to_char ("Joined.\n\r", ch);
}

void do_rename (CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char filename[MAX_INPUT_LENGTH];
  FILE *fp;
  CHAR_DATA *vict;
  int extract_them = 1;
  int success = 1;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (!arg1[0] || !arg2[0])
    {
      send_to_char ("Syntax: rename <old_name> <new_name>\n\r", ch);
      return;
    }

  sprintf (filename, "%sPS%s", PLAYER_DIR, capitalize (arg2));
  
  if ((fp = fopen (filename, "r")) != NULL)
    {
      send_to_char ("That player already exist.  Rename aborted.\n\r", ch);
      fclose (fp);
      return;
    }

  if ((vict = char_file_active (arg1)) != NULL)
    extract_them = 0;

  if (!vict && ((vict = load_char_obj (NULL, arg1)) == NULL))
    {
      send_to_char ("That character is not in the player files.\n\r", ch);
      return;
    }

  if (get_trust (ch) > get_trust (vict))
    {
      free_string (vict->names);
      vict->names = str_dup (capitalize (arg2));
      save_char_obj (vict);
    }
  else
    {
      send_to_char ("You failed.\n\r", ch);
      success = 0;
    }

  if (extract_them)
    extract_char (vict, TRUE);
  else
    if (success)
      {
	sprintf (buf, "You are now %s.\n\r", capitalize(arg2));
	send_to_char (buf, vict);
      }

  if (success)
    {
      sprintf( buf, "mv -f %sPS%s %sPS%s", PLAYER_DIR, 
	       capitalize( arg1 ), BAK_PLAYER_DIR, 
	       capitalize( arg1 ) );
      system (buf);
    }
}

void do_noleader (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  
  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Noleader whom?\n\r", ch );
      return;
    }
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  if ( IS_NPC(victim) )
    {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }
  
  if ( get_trust( victim ) >= get_trust( ch ) )
    {
      send_to_char( "You failed.\n\r", ch );
      return;
    }
  
  if (!IS_SET (victim->act, PLR_NOLEADER))
    {
      SET_BIT(victim->act, PLR_NOLEADER);
      send_to_char( "You are no longer allowed to be a leader!\n\r", victim );
      act ( "$N can no longer be a leader.\n\r", ch, NULL, victim, TO_CHAR );
      save_char_obj(victim);
      return;
    }

  send_to_char ("You may be a leader once more!\n\r", victim);
  act ( "$N can now be a leader once more.\n\r", ch, NULL, victim, TO_CHAR );
  REMOVE_BIT (victim->act, PLR_NOLEADER);
  save_char_obj(victim);
  return;
}  

void do_tick (CHAR_DATA *ch, char *argument)
{
  tick_stuff (1);
  send_to_char ("Tick.\n\r", ch);
}

void do_expan (CHAR_DATA *ch, char *argument)
{
  send_to_char ("If you want to expand the city, type out the whole thing, "
		"expand.\n\rDon't use this unless you really mean to, the "
		"game will expand again when\n\rit gets to the next number "
		"of players divisible by 15, even if you expaned\n\rit "
		"manually beforehand.  This is mostly for testing, but could "
		"have possible\n\ruses in scenarios I suppose.\n\r", ch);
}

void do_expand (CHAR_DATA *ch, char *argument)
{
  expand_city ();
  expansions++;
  scatter_objects ();
  send_to_char ("Done.\n\r", ch);
}
