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
#include "ground0.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_stand		);

sh_int teleport_time = 0;
char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down"
};

const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4
};


/*
 * Local functions.
 */
int	    find_door	args( ( CHAR_DATA *ch, char *arg ) );
int	    numeric_direction args( ( char *arg ) );
bool	    has_key	args( ( CHAR_DATA *ch, int key ) );
int         is_aggro    args( (CHAR_DATA *ch, CHAR_DATA *vict) );

ROOM_DATA *index_room (ROOM_DATA *curr_room, sh_int x, sh_int y)
{
  return &(curr_room[x + curr_room->this_level->x_size * y]);
}

ROOM_DATA *get_to_room (ROOM_DATA *curr_room, sh_int door)
{
  sh_int max_x = curr_room->this_level->x_size - 1;
  sh_int max_y = curr_room->this_level->y_size - 1;
  LEVEL_DATA *other_level;
  sh_int newx, newy;
  switch (door)
    {
    case DIR_NORTH:
      if (curr_room->y + 1 <= max_y)
	return index_room(curr_room,0,1);
      else
	return index_room(curr_room->this_level->rooms_on_level,
			  curr_room->x,0);
      break;
    case DIR_EAST:
      if (curr_room->x + 1 <= max_x)
	return index_room(curr_room,1,0);
      else
	return index_room(curr_room->this_level->rooms_on_level,
			  0,curr_room->y);
      break;
    case DIR_SOUTH:
      if (curr_room->y - 1 >= 0)
	return index_room(curr_room,0,-1);
      else
	return index_room(curr_room->this_level->rooms_on_level,
			  curr_room->x,max_y);
      break;
    case DIR_WEST:
      if (curr_room->x - 1 >= 0)
	return index_room(curr_room,-1,0);
      else
	return index_room(curr_room->this_level->rooms_on_level,
			  max_x,curr_room->y);
      break;
    case DIR_UP:
      other_level = curr_room->this_level->level_up;
      newx = other_level->reference_x - (curr_room->this_level->reference_x -
					 curr_room->x);
      newy = other_level->reference_y - (curr_room->this_level->reference_y -
					 curr_room->y);
      if ((newx > other_level->x_size - 1) || (newx < 0) ||
          (newy > other_level->y_size - 1) || (newy < 0))
	{
	  newx = newy = 0;
	  log_string ("REALBUG: (up) Attempt to enter non_existant square "
		      "on a level from %d %d %d.", curr_room->x, curr_room->y,
		      curr_room->level);
	}
      return index_room (other_level->rooms_on_level, newx,newy);
      break;
    case DIR_DOWN:
      other_level = curr_room->this_level->level_down;
      newx = other_level->reference_x - (curr_room->this_level->reference_x -
					 curr_room->x);
      newy = other_level->reference_y - (curr_room->this_level->reference_y -
					 curr_room->y);
      if ((newx > other_level->x_size - 1) || (newx < 0) ||
          (newy > other_level->y_size - 1) || (newy < 0))
	{
	  newx = newy = 0;
	  log_string ("REALBUG: (down) Attempt to enter non_existant square "
		      "on a level from %d %d %d.", curr_room->x, curr_room->y,
		      curr_room->level);
	}
      return index_room(other_level->rooms_on_level,newx,newy);
      break;
    }
}

void do_enter (CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  if ((obj = get_obj_list (ch, argument, ch->in_room->contents)) == NULL)
    {
      send_to_char ("You don't see that here.\n\r", ch);
      return;
    }
  if (!obj->interior)
    {
      send_to_char ("That does not seem to have an entrance.\n\r", ch);
      return;
    }

  if (ch->in_room->inside_mob)
    {
      send_to_char ("You cannot enter an object while inside a mob.\n\r", ch);
      return;
    }

  act ("You enter $p.", ch, obj, NULL, TO_CHAR);
  act ("$n enters $p.", ch, obj, NULL, TO_ROOM);
  char_from_room (ch);
  char_to_room (ch, obj->interior);
  if (obj->item_type == ITEM_TEAM_VEHICLE)
    SET_BIT (ch->temp_flags, IN_TANK);
  complete_movement (ch);
  return;
}

void do_teleport (CHAR_DATA *ch, char *argument)
{
  if ((ch->in_room != safe_area) && !teleport_time && ch->desc)
    if (!argument[0] || str_prefix (argument, "emergency"))
      {
	send_to_char ("A scratchy voice comes through over your comm badge, "
		      "'`tI'm sorry, but\n\rredeployment systems are offline "
		      "at the moment.  We will notify you when they\n\rare "
		      "back on line.  If you require emergency transport, "
		      "then you may request it by typing 'transport emergency'"
		      ", be sure you are in excellent physical condition "
		      "if you do this though.``'\n\r", ch);
	return;
      }
    else
      {
	send_to_char ("A scortching ray of light `1burns`` you as "
		      "transports from above try to pinpoint your "
		      "location.\n\r", ch);
	damage_char (ch, ch->last_hit_by, 5900);
      }
  
  if ((ch->in_room == safe_area) || (ch->in_room->level >= 0) || !ch->desc)
    {
      act ("A helicopter roars in from overhead kicking up dust and "
	   "dirt in a wide radius.\n\rYou are efficiently hustled "
	   "aboard.  On your way to the dropoff, the\n\rSeargant in "
	   "command briefly wishes you luck but you can see from "
	   "the\n\rexpression on his face that he finds your survival "
	   "highly questionable.\n\rThen before you know it, you are "
	   "dropped off and the helicopter is gone once more.", ch, 
	   NULL, NULL, TO_CHAR );
      act("A helicopter roars in from overhead kicking up dust and dirt in a "
	  "wide radius.\n\r$n is hustled aboard as soldiers cover his "
	  "safe boarding with deadly\n\rweapons of a make you have never "
	  "seen.\n\rWith a roar, the helicopter takes to the sky and is "
	  "quickly lost from view.", ch, NULL, NULL, TO_ROOM );
      char_from_room (ch);
      char_to_room( ch, index_room (the_city->rooms_on_level,
				    number_range (0, the_city->x_size - 1),
				    number_range (0, the_city->y_size - 1)));
      act("With a roar of displaced air, a helicopter arrives above you.\n\r"
	  "Dust and dirt sting your face as it lands.\n\r$n is quickly pushed "
	  "out and the helicopter speeds away into the sky once more.", 
	  ch, NULL, NULL, TO_ROOM );
      do_look (ch, "auto");
      stop_manning (ch);
      REMOVE_BIT (ch->temp_flags, IN_TANK);
      return;
    }
  else
    {
      send_to_char ("The transports could not reach your location.\n\r", ch);
      return;
    }
}

void do_leave (CHAR_DATA *ch, char *argument)
{
  ROOM_DATA *temp_room;
  OBJ_DATA *obj;

  if (!ch->in_room->interior_of)
    {
      do_teleport (ch, argument);
      return;
    }
  obj = ch->in_room->interior_of;
  temp_room = obj->in_room;
  act ("You leave $p.", ch, obj, NULL, TO_CHAR);
  act ("$n has left.", ch, obj, NULL, TO_ROOM);
  char_from_room (ch);
  char_to_room (ch, temp_room);
  stop_manning (ch);
  REMOVE_BIT (ch->temp_flags, IN_TANK);
  act ("$n clambers out of $p.", ch, obj, NULL, TO_ROOM);
  complete_movement (ch);
  return;
}

int is_aggro (CHAR_DATA *ch, CHAR_DATA *vict)
{
  if (IS_IMMORTAL(vict))
    return 0;
  if (ch->owner == vict)
    return 0;
  if ((ch->owner == vict->owner) && ch->owner)
    return 0;
  if (IS_SET (ch->act, PLR_AGGRO_ALL))
    return 1;
  if (IS_SET (vict->temp_flags, TRAITOR))
    return 1;
  /* RELEASE: this is where we used to check for the kill_other flag which
     players had on when they typed "kill all" */
  return 0;
}

void control_mob (CHAR_DATA *ch, CHAR_DATA *mob, int door)
{
  if (IS_SET (ch->temp_flags, MAN_TURRET))
    {
      if (door > DIR_WEST)
	{
	  send_to_char ("The turret can't be turned in that "
			"direction.\n\r", ch);
	  return;
	}
      act ("You turn the turret to face $t.\n\r", ch, dir_name[door], NULL,
	   TO_CHAR);
      mob->turret_dir = door;
      return;
    }
  if (IS_SET (ch->temp_flags, MAN_DRIVE))
    {
      if (door > DIR_WEST)
	{
	  send_to_char ("Tanks cannot move in that direction.\n\r", ch);
	  return;
	}
      act ("You drive the tank $t.\n\r", ch, dir_name[door], NULL,
	   TO_CHAR);
      move_char (mob, door, FALSE);
      return;
    }
  if (IS_SET (ch->temp_flags, MAN_SHIELD))
    {
      act ("You now have shielding in the $t direction.\n\r", ch,
	   dir_name[door], NULL, TO_CHAR);
      mob->shield_dir = door;
      return;
    }
  sbug ("control_mob: not manning anything.");
}

void move_char( CHAR_DATA *ch, int door, bool follow )
{
  ROOM_DATA *in_room;
  ROOM_DATA *to_room;
  long       pexit;
  CHAR_DATA *fch;
  CHAR_DATA *fch_next;

  if ( door < 0 || door > 5 )
    {
      bug( "Do_move: bad door %d.", door );
      return;
    }
  
  if (ch->in_room->inside_mob)
    {
      control_mob (ch, ch->in_room->inside_mob, door);
      return;
    }

  in_room = ch->in_room;
  pexit = in_room->exit[door];
  if ( IS_SET (pexit, EX_CLOSED) || IS_SET (pexit, EX_ISWALL))
    {
      send_to_char( "Can't go that way pal.  There's a wall.\n\r", ch );
      return;
    }
  
  if (IS_SET (ch->affected_by, AFF_DAZE))
    {
      send_to_char ("You are too dazed to move.\n\r", ch);
      return;
    }
  
  to_room = get_to_room (in_room, door);
  
  if ( room_is_private( to_room ) )
    {
      send_to_char( "That room is private right now.\n\r", ch );
      return;
    }
  
  if (ch->position < POS_STANDING)
    do_stand (ch, "");
  
  char_from_room( ch );
  char_to_room( ch, to_room );
  do_look( ch, "auto" );
  
  if (in_room == to_room) /* no circular follows */
    return;
  
  for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
      fch_next = fch->next_in_room;
      
      if ( fch->leader == ch && fch->position == POS_STANDING )
	{
	  if (fch->position < POS_STANDING)
	    do_stand(fch,"");
	  act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	  move_char( fch, door, TRUE );
	}
    }
  complete_movement (ch);
  /*
  if ((to_room->level == 2) && !IS_IMMORTAL(ch))
    {
      send_to_char ("Pataki's energy field slows you . . .\n\r", ch);
      WAIT_STATE (ch, 5);
    }
  else*/
    WAIT_STATE( ch, 1 );
  
}

void complete_movement (CHAR_DATA *ch)
{
  sh_int distance;
  char *the_dir, buf [MAX_INPUT_LENGTH];
  CHAR_DATA *fch;
  CHAR_DATA *fch_next;
  ROOM_DATA *temp_room;

  if (!IS_SET(ch->act, PLR_WIZINVIS))
    for (fch = char_list; fch; fch = fch->next)
      {
	if (fch == ch)
	  continue;
	if (!can_see_linear_char (fch, ch))
	  continue;
	buf[0] = NULL;
	if (ch->in_room->x - fch->in_room->x > 0)
	  {
	    distance = ch->in_room->x - fch->in_room->x;
	    the_dir = "east";
	  }
	else
	  if (ch->in_room->x - fch->in_room->x < 0)
	    {
	      distance = fch->in_room->x - ch->in_room->x;
	      the_dir = "west";
	    }
	  else
	    if (ch->in_room->y - fch->in_room->y > 0)
	      {
		distance = ch->in_room->y - fch->in_room->y;
		the_dir = "north";
	      }
	    else
	      if (ch->in_room->y - fch->in_room->y < 0)
		{
		  distance = fch->in_room->y - ch->in_room->y;
		  the_dir = "south";
		}
	      else
		sprintf (buf, "$N has arrived.");
	if (!buf[0])
	  sprintf (buf, "$N has arrived %d %s.", distance, the_dir);
	act (buf, fch, NULL, ch, TO_CHAR);
	if (fch->interior)
	  {
	    log_string ("sending to interior");
	    act (buf, fch->interior->people, NULL, ch, TO_CHAR);
	    act (buf, fch->interior->people, NULL, ch, TO_ROOM);
	  }
	if (is_aggro (fch, ch))
	  {
	    fch->fighting = ch;
	    if (IS_NPC (fch) && (fch->ld_behavior == BEHAVIOR_GUARD) &&
		(fch->in_room == ch->in_room))
	      {
		int door;

		do
		  door = number_range (0, 3);
		while (ch->in_room->exit[door]);
		sprintf (buf, "%s %s", ch->names, dir_name[door]);
		do_push (fch, buf);
	      }
	  }
	if (is_aggro (ch, fch))
	  ch->fighting = fch;
      }
  
  temp_room = ch->in_room;

  if (temp_room->mine)
    if (!find_eq_char (ch, SEARCH_GEN_FLAG, GEN_DETECT_MINE))
      {
	temp_room->mine->destination = NULL;
	obj_to_char (temp_room->mine, ch);
	bang_obj (temp_room->mine, 0);
	temp_room->mine = NULL;
      }
    else
      send_to_char ("Your metal detector beeps wildly around a certain "
		    "area of the ground and you carefully avoid the "
		    "mine.\n\r", ch);
  return;
}

void do_push( CHAR_DATA *ch, char *argument )
{
  ROOM_DATA *temp_room;
  CHAR_DATA *victim;
  OBJ_DATA *the_barrel;
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char message[MAX_INPUT_LENGTH];
  sh_int x_mod, y_mod, dir;

  argument = one_argument (argument, arg);
  one_argument (argument, arg2);
  
  if ( arg[0] == '\0' )
    {
	send_to_char( "Who do you want to push?\n\r", ch ); 
	return; 
    }

  if (is_name (arg, "barrel gasoline"))
    {
      argument = one_argument (argument, arg);
      x_mod = y_mod = 0;
      if (!str_prefix (arg, "north"))
	{
	  y_mod = 1;
	  dir = 0;
	  sprintf (message, "north");
	}
      else
	if (!str_prefix (arg, "south"))
	  {
	    y_mod = -1;
	    dir = 2;
	    sprintf (message, "south");
	  }
	else
	  if (!str_prefix (arg, "east"))
	    {
	      x_mod = 1;
	      dir = 1;
	      sprintf (message, "east");
	    }
	  else
	    if (!str_prefix (arg, "west"))
	      {
		x_mod = -1;
		dir = 3;
		sprintf (message, "west");
	      }
	    else
	      {
		send_to_char ("What dirction do you want to push it?", 
				   ch);
		return;
	      }
      if ((the_barrel = get_obj_list (ch, "barrel", 
				      ch->in_room->contents)) == NULL)
	{
	  send_to_char ("You don't see that here.", ch);
	  return;
	}
      if (the_barrel->in_room->exit[dir])
	{
	  send_to_char ("A wall blocks your way.", ch);
	  return;
	}
      temp_room = index_room (the_barrel->in_room, x_mod, y_mod);
      obj_from_room (the_barrel);
      obj_to_room (the_barrel, temp_room);
      act ("You push $P $t.", ch, message, the_barrel, TO_CHAR);
      act ("$n pushes $P $t.", ch, message, the_barrel, TO_ROOM);
      return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
      {
	send_to_char( "They aren't here.\n\r", ch ); 
	return;
      }

    if ( !IS_AWAKE(ch) )
      {
	send_to_char( "Try waking up first!\n\r", ch ); 
	return; 
      }

  if (IS_NPC (victim))
    {
      send_to_char ("They won't budge.\n\r", ch);
      return;
    }

  if ( victim == ch )
    {
	send_to_char( "Why would you want to push yourself, bully?\n\r",ch);
	return;
    }

  if (!str_prefix (arg2, "north") || !str_prefix (arg2, "south") ||
      !str_prefix (arg2, "east") || !str_prefix (arg2, "west"))
    {
      act ("$n pushes $N $t.", ch, arg2, victim, TO_NOTVICT);
      act ("You pushes $N $t.", ch, arg2, victim, TO_CHAR);
      act ("\n\r$n pushes you $t.\n\r", ch, arg2, victim, TO_VICT);
      interpret (victim, argument);
    }

  return;
}

void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTH, FALSE );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_EAST, FALSE );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTH, FALSE );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_WEST, FALSE );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_UP, FALSE );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_DOWN, FALSE );
    return;
}

int numeric_direction (char *arg)
{
  if ( !str_prefix( arg, "north" ) ) return DIR_NORTH;
  else if ( !str_prefix( arg, "east"  ) ) return DIR_EAST;
  else if ( !str_prefix( arg, "south" ) ) return DIR_SOUTH;
  else if ( !str_prefix( arg, "west"  ) ) return DIR_WEST;
  else if ( !str_prefix( arg, "up"    ) ) return DIR_UP;
  else if ( !str_prefix( arg, "down"  ) ) return DIR_DOWN;
  else return -1;
}

int find_door( CHAR_DATA *ch, char *arg )
{
    long pexit;
    int door;

	 if ( str_prefix( arg, "north" ) ) door = 0;
    else if ( str_prefix( arg, "east"  ) ) door = 1;
    else if ( str_prefix( arg, "south" ) ) door = 2;
    else if ( str_prefix( arg, "west"  ) ) door = 3;
    else if ( str_prefix( arg, "up"    ) ) door = 4;
    else if ( str_prefix( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	  if (IS_SET (ch->in_room->exit[door], EX_ISDOOR))
	    return door;
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(ch->in_room->exit[door], EX_ISDOOR) )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    return door;
}



void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_DATA *to_room;
	long pexit;
	long pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(pexit, EX_CLOSED);
	act( "$n opens the door.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* open the other side */
	if ( ( to_room   = get_to_room (ch->in_room, door) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL))
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The door opens.", rch, NULL, NULL, TO_CHAR );
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_DATA *to_room;
	long pexit;
	long pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	SET_BIT(pexit, EX_CLOSED);
	act( "$n closes the door.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* close the other side */
	if ( ( to_room   = get_to_room (ch->in_room, door) ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0)
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The door closes.", rch, NULL, NULL, TO_CHAR );
	}
    }

    return;
}



bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_DATA *to_room;
	long pexit;
	long pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( IS_SET (pexit, EX_ISNOLOCKDOOR))
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( IS_SET(pexit, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the door.", ch, NULL, NULL, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = get_to_room (ch->in_room, door)) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0)
	{
	    SET_BIT( pexit_rev, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_DATA *to_room;
	long pexit;
	long pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if (IS_SET (pexit, EX_ISNOUNLOCKDOOR))
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(pexit, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the door.", ch, NULL, NULL, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = get_to_room (ch->in_room, door)) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL)
	{
	    REMOVE_BIT( pexit_rev, EX_LOCKED );
	}
    }

    return;
}

void do_stand( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You wake and stand up.\n\r", ch );
	act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_RESTING: case POS_SITTING:
	send_to_char( "You stand up.\n\r", ch );
	act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You wake up and start resting.\n\r", ch );
	act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
	ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	send_to_char( "You rest.\n\r", ch );
	act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	send_to_char("You rest.\n\r",ch);
	act("$n rests.",ch,NULL,NULL,TO_ROOM);
	ch->position = POS_RESTING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }


    return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
    switch (ch->position)
    {
	case POS_SLEEPING:
	    send_to_char("You wake up.\n\r",ch);
	    act("$n wakes and sits up.\n\r",ch,NULL,NULL,TO_ROOM);
	    ch->position = POS_SITTING;
	    break;
 	case POS_RESTING:
	    send_to_char("You stop resting.\n\r",ch);
	    ch->position = POS_SITTING;
	    break;
	case POS_SITTING:
	    send_to_char("You are already sitting down.\n\r",ch);
	    break;
	case POS_FIGHTING:
	    send_to_char("Maybe you should finish this fight first?\n\r",ch);
	    break;
	case POS_STANDING:
	    send_to_char("You sit down.\n\r",ch);
	    act("$n sits down on the ground.\n\r",ch,NULL,NULL,TO_ROOM);
	    ch->position = POS_SITTING;
	    break;
    }
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING: 
	send_to_char( "You go to sleep.\n\r", ch );
	act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SLEEPING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    victim->position = POS_STANDING;
    act( "You wake $M.", ch, NULL, victim, TO_CHAR );
    act( "$n wakes you.", ch, NULL, victim, TO_VICT );
    return;
}
