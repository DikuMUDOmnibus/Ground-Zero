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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ground0.h"

/* command procedures needed */
DECLARE_DO_FUN(do_quit		);

/*
 * Local functions.
 */
int	hit_gain	args( ( CHAR_DATA *ch ) );
int	mana_gain	args( ( CHAR_DATA *ch ) );
int	move_gain	args( ( CHAR_DATA *ch ) );
void	mobile_update	args( ( void ) );
void	weather_update	args( ( void ) );
void	char_update	args( ( void ) );
void	obj_update	args( ( void ) );
void	aggr_update	args( ( void ) );

/* used for saving */

int	save_number = 0;
int     tick_counter = 0;

/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    gain = number_range (125, 325);
    switch ( ch->position )
      {
      case POS_SLEEPING: gain *= 2; break;
      }

    return UMIN(gain, ch->max_hit - ch->hit);
}

/*
 * Update all chars, including mobs.
*/
void char_update( void )
{   
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;

    /* update save counter */
    save_number++;

    if (save_number > 29)
	save_number = 0;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if (ch->ld_timer == 1)
	  {
	    ch->last_hit_by = NULL;
	    char_death (ch);
	  }
	if (ch->ld_timer)
	  ch->ld_timer--;
	if (!number_range (0, 5) && IS_SET (ch->affected_by, AFF_BLIND))
	  {
	    REMOVE_BIT (ch->affected_by, AFF_BLIND);
	    send_to_char ("You can see again!\n\r", ch);
	  }

	if (!number_range (0, 2) && IS_SET (ch->affected_by, AFF_DAZE))
	  {
	    REMOVE_BIT (ch->affected_by, AFF_DAZE);
	    send_to_char ("You are no longer dazed!\n\r", ch);
	  }

	if ( ch->position >= POS_STUNNED )
	{
	    if ( ch->hit  < ch->max_hit )
		ch->hit  += hit_gain(ch);
	    else
		ch->hit = ch->max_hit;
	}

	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

	if ( !IS_NPC(ch) && !ch->trust)
	  if (IS_IMMORTAL(ch))
	    ch->timer = 0;
	ch->donate_num = UMAX (0, ch->donate_num - 2);
	ch->chan_delay = UMAX(0, ch->chan_delay - 4*PULSE_PER_SECOND);

    }

    /*
     * Autosave.
     * Check that these chars still exist.
     */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

	if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
	    save_char_obj(ch);

    }

    return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;
    CHAR_DATA *all_chars;

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message;
        char buf[MAX_STRING_LENGTH];
        DESCRIPTOR_DATA *d;

	obj_next = obj->next;

	if (obj->arrival_time >= 0)
	  {
	    obj->arrival_time--;
	    if (obj->arrival_time == -1)
	      {
		ROOM_DATA *temp_room;

		if (!obj->in_room)
		  sbug ("obj_update: object not in room");
		obj_from_room (obj);
		temp_room = obj->destination;
		obj->destination = NULL;
		obj_to_room (obj, temp_room);
		if (obj->in_room->people)
		  {
		    act ("$p lands at your feet.", obj->in_room->people,
			 obj, NULL, TO_ROOM);
		    act ("$p lands at your feet.", obj->in_room->people,
			 obj, NULL, TO_CHAR);
		  }
		if (IS_SET (obj->general_flags, GEN_EXTRACT_ON_IMPACT))
		  bang_obj (obj, 0);
	      }
	  }

	/* for clarity, this means that we either have a timer of -1 or 0
	   (a stopped timer) from the first half of the clause, or we have a
	   timer that is positive int after subtracting 1 from it, the else
	   clause means the timer must have just expired */
	if ( obj->timer <= 0 || --obj->timer > 0 )
	  {
	    if (obj->carried_by && obj->timer > 0)
	      if (IS_SET (obj->extract_flags, EXTRACT_BURN_ON_EXTRACT))
		act ("$p burns quietly in your hand.", obj->carried_by, obj,
		     NULL, TO_CHAR);
	      else
		act ("$p ticks very quietly.", obj->carried_by, obj, NULL, 
		     TO_CHAR);

	    if (IS_SET (obj->general_flags, GEN_BURNS_ROOM))
	      if (obj->in_room && obj->in_room->people)
		burn_obj (obj);

	    continue;
	  }
	else
	  {
	    if (obj->arrival_time >= 0)
	      extract_obj (obj, 0);
	    else
	      {
		if (obj->extract_flags)
		  bang_obj (obj, 0);
		else
		  if (IS_SET (obj->general_flags, GEN_BURNS_ROOM))
		    {
		      if (obj->in_room->people)
			{
			  act ("$p burns down and dies leaving nothing but "
			       "ashes.", obj->in_room->people, obj, NULL,
			       TO_CHAR);
			  act ("$p burns down and dies leaving nothing but "
			       "ashes.", obj->in_room->people, obj, NULL,
			       TO_ROOM);
			}
		      extract_obj (obj, 1);
		    }
		  else
		    if (IS_SET (obj->general_flags, GEN_DARKS_ROOM))
		      {
			if (obj->in_room->people)
			  {
			    act ("This room is no longer dark.",
				 obj->in_room->people, obj, NULL, TO_CHAR);
			    act ("This room is no longer dark.",
				 obj->in_room->people, obj, NULL, TO_ROOM);
			  }
			extract_obj (obj, 1);
		      }
	      }
	  }
    }
}

void tick_stuff (int imm_generated)
{
  static  int     pulse_point = PULSE_TICK;
  char buf[100];
  CHAR_DATA *all_chars;
  int count;

  if ((--pulse_point > 0) && !imm_generated)
    return;

  sprintf(buf,"TICK %d\n\r", tick_counter);
  log_string(buf);
  tick_counter++;

  if (!teleport_time)
    {
      if (!number_range (0, 99))
	{
	  for (all_chars = char_list; all_chars; 
	       all_chars = all_chars->next)
	    send_to_char ("A scratchy voice comes in over your comm "
			  "badge '`tRedeployment systems are\n\r"
			  "operational for the moment, but may go "
			  "offline at any second.  Should you\n\rneed "
			  "redeployment in the near future, please do "
			  "so now.``'\n\r", all_chars);
	  teleport_time = 1;
	}
    }
  else
    {
      if (!number_range (0, 11))
	{
	  for (all_chars = char_list; all_chars; 
	       all_chars = all_chars->next)
	    send_to_char ("A scratchy voice comes in over your comm "
			  "badge '`tTransportation systems have\n\rjust "
			  "gone off line.  Redeployment services are "
			  "temporarily unavailable.``'\n\r", all_chars);
	  teleport_time =0;
	}
    }
  pulse_point     = PULSE_TICK;
  char_update	( );
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler()
{
    static  int     pulse_violence = PULSE_VIOLENCE;
    static  int     pulse_objects = PULSE_OBJECTS;
    static  int     pulse_save = PULSE_SAVE;

    tick_stuff (0);

    if ( --pulse_violence <= 0 )
    {
	pulse_violence	= PULSE_VIOLENCE;
	violence_update	( );
    }

    if ( --pulse_objects <= 0 )
      {
	pulse_objects = PULSE_OBJECTS;
	obj_update ( );
      }

    if ( --pulse_save <= 0 )
      {
	pulse_save = PULSE_SAVE;
	do_save_all (NULL, "");
      }

    /* get rid of dead folk */
    while (extract_list)
      {
	CHAR_DATA *ch;

	ch = extract_list;
	extract_list = extract_list->next_extract;
	extract_char (ch, TRUE);
      }

    tail_chain( );
}
