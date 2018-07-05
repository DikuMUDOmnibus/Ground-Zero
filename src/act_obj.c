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
DECLARE_DO_FUN(do_split		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_say		);



/*
 * Local functions.
 */
bool	      remove_obj       args( ( CHAR_DATA *ch, int iWear, 
				      bool fReplace, int wearing ) );
void	      wear_obj	       args( ( CHAR_DATA *ch, OBJ_DATA *obj, 
				      bool fReplace ) );
CHAR_DATA *   find_keeper      args( ( CHAR_DATA *ch ) );
int	      get_cost	       args( ( CHAR_DATA *keeper, OBJ_DATA *obj, 
				      bool fBuy ) );
int           evac_char        args( ( CHAR_DATA *ch, OBJ_DATA *obj,
				      char *argument) );
int           air_strike       args( ( CHAR_DATA *ch, char *argument) );

int evac_char (CHAR_DATA *ch, OBJ_DATA *obj, char *argument)
{
  ROOM_DATA *dest;

  /* To prevent evac booming by low rep chars, but now you cannot evac to a
     specific person, so it is extraneous
  if (ch->reputation < 450)
    {
      act ("You talk into $p, requesting transport.\n\rUnfortunately, the "
	   "officer on the other side informs you that you are of "
	   "insufficient rank for your request to be honorred.", ch, obj,
	   NULL, TO_CHAR);
      return 0;
    }
    */
  if ((ch->in_room->interior_of &&
       (ch->in_room->interior_of->item_type == ITEM_TEAM_ENTRANCE)) ||
      (ch->in_room == safe_area))
    {
      send_to_char ("Radio signal is too weak here.\n\r", ch);
      return 0;
    }
  if (dest = find_location (ch, argument))
    if (!dest->level)
      {
	act ("You talk into $p, requesting transport.\n\rAlmost "
	     "instantly, a bright light surrounds you and you are at "
	     "your destination.\n\r", ch, obj, NULL, TO_CHAR);
	do_goto (ch, argument);
	complete_movement (ch);
	return 1;
      }
    else
      {
	send_to_char ("Transports cannot reach the saferoom or lower "
		      "levels.\n\r",ch);
	return 0;
      }
  send_to_char ("You must specify where you want to go in the form x y "
		"level.\n\r", ch);
  return 0;
}

int air_strike (CHAR_DATA *ch, char *argument)
{
  ROOM_DATA *start_room, *temp_room;
  OBJ_DATA *the_napalm;
  sh_int start_x, start_y, end_x, end_y, x, y;
  int radius = 1;

  if ((ch->in_room->interior_of &&
       (ch->in_room->interior_of->item_type == ITEM_TEAM_ENTRANCE)) ||
      (ch->in_room == safe_area))
    {
      send_to_char ("Radio signal is too weak here.\n\r", ch);
      return 0;
    }
  start_room = find_location (ch, argument);
  if (!start_room)
    {
      send_to_char ("You must specify which coordinates you want bombed in "
		    "the form x y level.\n\r", ch);
      return 0;
    }
  if (start_room->level)
    {
      send_to_char ("Lower levels cannot be reached by bombers.\n\r", ch);
      return 0;
    }
  if ((start_x = start_room->x - radius) < 0)
    start_x = 0;
  if ((start_y = start_room->y - radius) < 0)
    start_y = 0;
  if ((end_x = start_room->x + radius) > start_room->this_level->x_size - 1)
    end_x = start_room->this_level->x_size - 1;
  if ((end_y = start_room->y + radius) > start_room->this_level->y_size - 1)
    end_y = start_room->this_level->y_size - 1;

  for (x = start_x; x <= end_x; x++)
    for (y = start_y; y <= end_y; y++)
      {
	temp_room = index_room (start_room->this_level->rooms_on_level, x, y);
	obj_to_room (the_napalm=create_object (get_obj_index(VNUM_NAPALM), 0), 
		     temp_room);
	the_napalm->owner = ch;
	if (temp_room->people)
	  {
	    act ("A bomber roars overhead dropping $p which lands everwhere!",
		 temp_room->people, the_napalm, NULL, TO_CHAR);
	    act ("A bomber roars overhead dropping $p which lands everwhere!",
		 temp_room->people, the_napalm, NULL, TO_ROOM);
	  }
	bang_obj (the_napalm, 1);
      }
  return 1;
}

int do_use (CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *droid, *vict;
  OBJ_DATA *obj;
  char *rest_of_line, arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];

  rest_of_line = one_argument (argument, arg);
  if ((obj = get_obj_carry (ch, arg)) == NULL)
    {
      send_to_char ("You don't have that.\n\r", ch);
      return;
    }

  if (IS_SET (obj->usage_flags, USE_EVAC))
    {
      act ("$n talks quietly into $p.", ch, obj, NULL, TO_ROOM);
      if (evac_char (ch, obj, rest_of_line))
	extract_obj (obj, obj->extract_me);
    }
  else
    if (IS_SET (obj->usage_flags, USE_HEAL))
      {
	if (ch->hit < ch->max_hit)
	  act ("You use $p.  Ahhhh . . . `!MUCH`` better!", ch, obj, NULL,
	       TO_CHAR);
	else
	  act ("What a waste of good medicine.", ch, obj, NULL, TO_CHAR);
	act ("$n uses $p.", ch, obj, NULL, TO_ROOM);
	heal_char (ch, obj->damage_char[0]);
	extract_obj (obj, obj->extract_me);
      }
    else
      if (IS_SET (obj->usage_flags, USE_MAKE_DROID))
	{
	  droid = clone_mobile (pill_box);
	  char_from_room (droid);
	  char_to_room (droid, ch->in_room);
	  act ("$n uses $p to assemble a guard droid.", ch, obj, NULL,
	       TO_ROOM);
	  act ("You use $p to assemble a guard droid.", ch, obj, NULL,
	       TO_CHAR);
	  act ("You carefully program it to attack anything it comes in "
	       "contact with.", ch, NULL, NULL, TO_CHAR);
	  droid->owner = ch;
	  extract_obj (obj, obj->extract_me);
	}
      else
	if (IS_SET (obj->usage_flags, USE_MAKE_SEEKER_DROID))
	  {
	    one_argument (rest_of_line, arg);
	    if (!arg[0])
	      {
		send_to_char ("You must specify a target for the droid to "
			      "attempt to destroy.", ch);
		return;
	      }
	    if ((vict = get_char_world (ch, arg)) == NULL)
	      {
		send_to_char ("They aren't here.", ch);
		return;
	      }
	    droid = clone_mobile (pill_box);
	    char_from_room (droid);
	    char_to_room (droid, ch->in_room);
	    act ("$n uses $p to assemble a seeker droid.", ch, obj, NULL,
		 TO_ROOM);
	    act ("You use $p to assemble a seeker droid.", ch, obj, NULL,
		 TO_CHAR);
	    act ("You carefully program it to hunt down $N.", ch, NULL,
		 vict, TO_CHAR);
	    droid->ld_behavior = BEHAVIOR_SEEKING_PILLBOX;
	    droid->chasing = droid->fighting = vict;
	    droid->owner = ch;
	    extract_obj (obj, obj->extract_me);
	  }
	else
	  do_wear (ch, argument);
}

int do_toss (CHAR_DATA *ch, char *argument)
{
  char *	const	dir_name	[]		=
    {
      "north", "east", "south", "west", "up", "down"
    };

  char arg [MAX_INPUT_LENGTH], buf [MAX_INPUT_LENGTH];
  sh_int n_dir, num_squares, count, number;
  CHAR_DATA *thrown_at;
  ROOM_DATA *dest_room;
  OBJ_DATA *obj;

  thrown_at = NULL;
  argument = one_argument (argument, arg);
  
  if (!arg[0])
    {
      send_to_char ("Toss what?", ch);
      return;
    }
  obj = get_obj_carry (ch, arg);
  if (obj == NULL)
    {
      send_to_char ("You don't have that.", ch);
      return;
    } 
  if (ch->in_room->level < 0)
    {
      send_to_char ("No where to throw it to.", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if (!arg[0])
    {
      send_to_char ("Toss it where?", ch);
      return;
    }

  num_squares = 0;
  if (is_number (arg))
    {
      num_squares = atoi (arg);
      if (num_squares < 1)
	{
	  send_to_char ("The number must be positive.", ch);
	  return;
	}
      one_argument (argument, arg);
      if (!arg[0])
	{
	  send_to_char ("Yes, but in what direction?", ch);
	  return;
	}
    }
  else
    num_squares = 1;

  if ((n_dir = numeric_direction (arg)) == -1)
    {
      thrown_at = get_char_world (ch, arg);
      if (!thrown_at)
	{
	  send_to_char ("That is not a valid direction or player's name.", ch);
	  return;
	}
      if (!can_see_linear_char (ch, thrown_at) || 
	  (thrown_at->in_room->x - ch->in_room->x > 2) ||
	  (thrown_at->in_room->x - ch->in_room->x < -2) || 
	  (thrown_at->in_room->y - ch->in_room->y > 2) ||
	  (thrown_at->in_room->y - ch->in_room->y < -2))
	{
	  send_to_char ("They are not in range to throw it at them.", ch);
	  return;
	}
      if (ch->in_room->x - thrown_at->in_room->x > 0)
	n_dir = 1;
      else
	if (ch->in_room->x - thrown_at->in_room->x < 0)
	  n_dir = 3;
        else
	  if (ch->in_room->y - thrown_at->in_room->y > 0)
	    n_dir = 2;
	  else
	    n_dir = 0;
    }
  else
    {
      if ((n_dir == DIR_UP) || (n_dir == DIR_DOWN))
	{
	  send_to_char ("You cannot throw an item up or down.\n\r", ch);
	  return;
	}
      if (num_squares >= 2)
	{
	  send_to_char ("You `athrow`` it as hard as you can!\n\r", ch);
	  num_squares = 2;
	}
    }

  if (num_squares == 0)  num_squares = 1;

  if (!thrown_at)
    switch (n_dir)
      {
      case 0:
	if (ch->in_room->y + num_squares > ch->in_room->this_level->y_size - 1)
	  num_squares = ch->in_room->this_level->y_size - ch->in_room->y - 1;
	dest_room = index_room (ch->in_room, 0, num_squares);
	break;
      case 1:
	if (ch->in_room->x + num_squares > ch->in_room->this_level->x_size - 1)
	  num_squares = ch->in_room->this_level->x_size - ch->in_room->x - 1;
	dest_room = index_room (ch->in_room, num_squares, 0);
	break;
      case 2:
	if (ch->in_room->y - num_squares < 0)
	  num_squares = ch->in_room->y;
	dest_room = index_room (ch->in_room, 0, -num_squares);
	break;
      case 3:
	if (ch->in_room->x - num_squares < 0)
	  num_squares = ch->in_room->x;
	dest_room = index_room (ch->in_room, -num_squares, 0);
	break;
      }
  else
    dest_room = thrown_at->in_room;

  if ((ch->in_room->level > 0) && !can_see_linear_room (ch, dest_room))
    {
      send_to_char ("You aren't on the top level of the city, so cannot throw "
		    "objects over walls.", ch);
      return;
    }

  sprintf (buf, "$n `athrows`` $p `E%s``.", dir_name[n_dir]);
  act(buf,ch, obj, NULL, TO_ROOM);
  sprintf (buf, "You `athrow`` $p `E%s``.", dir_name[n_dir]);
  act (buf, ch, obj, NULL, TO_CHAR);

  send_object (obj, dest_room, num_squares);
  if(!obj->owner || (obj->owner != ch))
    obj->owner = ch;
}

int do_unload (CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *the_weapon, *the_ammo;
  if ((the_weapon = get_eq_char (ch, WEAR_WIELD)) == NULL)
    {
      send_to_char ("You aren't wielding anything to unload.\n\r", ch);
      return;
    }
  if (!the_weapon->contains)
    {
      send_to_char ("The weapon is already empty.\n\r", ch);
      return;
    }
  if (ch->carry_number+1 == MAX_NUMBER_CARRY)
    {
      send_to_char ("You can't carry that many items.\n\r", ch);
      return;
    }
  act ("You `aremove`` $P from $p.", ch, the_weapon, the_weapon->contains, 
       TO_CHAR);
  the_ammo = the_weapon->contains;
  obj_from_obj (the_ammo);
  obj_to_char (the_ammo, ch);
}

int do_load (CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *the_weapon, *the_ammo;
  sh_int count;
  char arg[MAX_INPUT_LENGTH];

  if ((the_weapon = get_eq_char (ch, WEAR_WIELD)) == NULL)
    {
      send_to_char ("You aren't wielding anything to load.\n\r", ch);
      return;
    }
#if 0
  if (the_weapon->item_type != ITEM_WEAPON)
    {
      send_to_char ("That weapon does not take any ammo.\n\r", ch);
      return;
    }
#endif
  if (the_weapon->contains)
    if (the_weapon->contains->ammo > 0)
      {
	sprintf (arg, "There are still %d rounds in the weapon.\n\r", 
		 the_weapon->contains->ammo);
	send_to_char (arg, ch);
	return;
      }
    else
      {
	extract_obj (the_weapon->contains, the_weapon->contains->extract_me);
	the_weapon->contains = NULL;
      }

  one_argument (argument, arg);
  if (!arg[0])
    {
      if ((the_ammo = find_eq_char (ch, SEARCH_AMMO_TYPE,
				    the_weapon->ammo_type)) == NULL)
	{
	  send_to_char ("You don't have any ammo for that weapon.\n\r", ch);
	  return;
	}
    }
  else
    {
      if ((the_ammo = get_obj_carry (ch, arg)) == NULL)
	{
	  send_to_char ("You don't have that item.\n\r", ch);
	  return;
	}
      if ((the_ammo->item_type != ITEM_AMMO) ||
	  (the_ammo->ammo_type != the_weapon->ammo_type))
	{
	  act ("$p is not ammo for $P", ch, the_ammo, the_weapon, TO_CHAR);
	  return;
	}
    }

  obj_from_char (the_ammo);
  if (the_weapon->contains)
    extract_obj (the_weapon->contains, the_weapon->contains->extract_me);
  the_ammo->owner = ch;
  obj_to_obj (the_ammo, the_weapon);
  act ("You `aload`` $p.\n\r`!CHCK-CHCK . . . CLUNK``\n\r`1Grrrrrrrrr . . .``",
       ch, the_weapon, NULL, TO_CHAR);
  act ("$n `aloads`` $P into $p.\n\r`!CHCK-CHCK . . . CLUNK``", ch, the_weapon,
       the_ammo, TO_ROOM);
}

void send_object (OBJ_DATA *obj, ROOM_DATA *dest_room, sh_int travel_time)
{
  if (obj->carried_by)
    obj_from_char (obj);
  else
    if (obj->in_room)
      obj_from_room (obj);
    else
      {
	log_string ("REALBUG in send_object");
	return;
      }
  obj_to_room (obj, explosive_area);
  obj->arrival_time = travel_time;
  obj->destination = dest_room;
}

void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	send_to_char( "You can't take that.\n\r", ch );
	return;
    }
   
    if (ch->carry_number+1 == MAX_NUMBER_CARRY)
      {
	send_to_char ("You can't carry that many items.\n\r", ch);
	return;
      }

    act( "You `aget`` $p.", ch, obj, NULL, TO_CHAR );
    act( "$n `agets`` $p.", ch, obj, NULL, TO_ROOM );
    if (!obj->in_room)
      {
	send_to_char ("This is a bug, please report.\n\r", ch);
	sbug ("get_obj: not in a room");
	bug_object_state (obj);
	return;
      }
    obj_from_room( obj );

    obj_to_char( obj, ch );

    return;
}

void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Get what?\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );
	    if ( obj == NULL )
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    get_obj( ch, obj, NULL );
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) ))
		{
		    found = TRUE;
		    get_obj( ch, obj, NULL );
		}
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\n\r", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
#if 0
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	if (( container->item_type != ITEM_CONTAINER) &&
	    (container->item_type != ITEM_CORPSE) )
	{
	  send_to_char( "That's not a container.\n\r", ch );
	  return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if ( obj == NULL )
	    {
		act( "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) ))
		{
		    found = TRUE;

		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in the $T.",
			ch, NULL, arg2, TO_CHAR );
		else
		    act( "I see nothing like that in the $T.",
			ch, NULL, arg2, TO_CHAR );
	    }
	}
    }
#endif

    return;
}

void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }


    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	act( "$n `adrops`` $p.", ch, obj, NULL, TO_ROOM );
	act( "You `adrop`` $p.", ch, obj, NULL, TO_CHAR );
    }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (obj->carried_by != ch)
	      sbug ("do_drop: carried_by is not ch");
	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   obj->wear_loc == WEAR_NONE)
	    {
		found = TRUE;
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		act( "$n `adrops`` $p.", ch, obj, NULL, TO_ROOM );
		act( "You `adrop`` $p.", ch, obj, NULL, TO_CHAR );
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if (obj->timer > 0)
      {
	send_to_char ("It's live, toss it or drop it.\n\r", ch);
	return;
      }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim) && !IS_IMMORTAL(ch))
      {
	send_to_char ("They don't seem to want it and you can't find anywhere "
		      "to put it on them.", ch);
	return;
      }

    if (victim->carry_number+1 == MAX_NUMBER_CARRY)
      {
	send_to_char ("They can't carry that many things.\n\r", ch);
	return;
      }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    act( "$n `agives`` $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n `agives`` you $p.",   ch, obj, victim, TO_VICT    );
    act( "You `agive`` $p to $N.", ch, obj, victim, TO_CHAR    );
    if ((obj->timer > 0) && (obj->item_type == ITEM_EXPLOSIVE))
      {
	send_to_char ("Watch out!  It's LIVE!!", victim);
	send_to_char ("A live one.", ch);
      }
    return;
}


/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace, int wearing )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if (!wearing && (ch->carry_number+1 == MAX_NUMBER_CARRY))
      {
	send_to_char ("You can't carry that many items.\n\r", ch);
	return FALSE;
      }

    unequip_char( ch, obj );
    act( "$n `astops using`` $p.", ch, obj, NULL, TO_ROOM );
    act( "You `astop using`` $p.", ch, obj, NULL, TO_CHAR );
    return TRUE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    char buf[MAX_STRING_LENGTH];

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace, 1 )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace, 1 ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    act( "$n `awears`` $p on $s left finger.",    ch, obj, NULL, 
		TO_ROOM );
	    act( "You `awear`` $p on your left finger.",  ch, obj, NULL, 
		TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    act( "$n `awears`` $p on $s right finger.",   ch, obj, NULL, 
		TO_ROOM );
	    act( "You `awear`` $p on your right finger.", ch, obj, NULL, 
		TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "You already wear two rings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace, 1 )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace, 1 ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( "$n `awears`` $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You `awear`` $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( "$n `awears`` $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You `awear`` $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace, 1 ) )
	    return;
	act( "$n `awears`` $p on $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You `awear`` $p on your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace, 1 ) )
	    return;
	act( "$n `awears`` $p on $s head.",   ch, obj, NULL, TO_ROOM );
	act( "You `awear`` $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace, 1 ) )
	    return;
	act( "$n `awears`` $p on $s legs.",   ch, obj, NULL, TO_ROOM );
	act( "You `awear`` $p on your legs.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace, 1 ) )
	    return;
	act( "$n `awears`` $p on $s feet.",   ch, obj, NULL, TO_ROOM );
	act( "You `awear`` $p on your feet.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace, 1 ) )
	    return;
	act( "$n `awears`` $p on $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You `awear`` $p on your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace, 1 ) )
	    return;
	act( "$n `awears`` $p on $s arms.",   ch, obj, NULL, TO_ROOM );
	act( "You `awear`` $p on your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace, 1 ) )
	    return;
	act( "$n `awears`` $p about $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You `awear`` $p about your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace, 1 ) )
	    return;
	act( "$n `awears`` $p about $s waist.",   ch, obj, NULL, TO_ROOM );
	act( "You `awear`` $p about your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace, 1 )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace, 1 ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( "$n `awears`` $p around $s left wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You `awear`` $p around your left wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( "$n `awears`` $p around $s right wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You `awear`` $p around your right wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "You already wear two wrist items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	OBJ_DATA *weapon;

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace, 1 ) )
	    return;

	weapon = get_eq_char(ch,WEAR_WIELD);

	act( "$n `awears`` $p as a shield.", ch, obj, NULL, TO_ROOM );
	act( "You `awear`` $p as a shield.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn,skill;

	if ( !remove_obj( ch, WEAR_WIELD, fReplace, 1 ) )
	    return;

	act( "$n `awields`` $p.", ch, obj, NULL, TO_ROOM );
	act( "You `awield`` $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WIELD );
	obj->owner = ch;

	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
	if ( !remove_obj( ch, WEAR_HOLD, fReplace, 1 ) )
	    return;
	act( "$n `aholds`` $p in $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You `ahold`` $p in your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOLD );
	return;
    }

    if ( fReplace )
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE)
		wear_obj( ch, obj, FALSE );
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	wear_obj( ch, obj, TRUE );
    }
    
    return;
}

void donate (CHAR_DATA *ch, OBJ_DATA *obj)
{
  if (obj->timer > 0)
    {
      send_to_char ("You can't donate that.  It's live!\n\r", ch);
      return;
    }
  if ((ch->donate_num == 4) && (!IS_IMMORTAL(ch)))
    {
      send_to_char ("Your donation device needs to recharge first.\n", ch);
      return;
    }
  act ("$p flashes for a moment, then is gone.", ch, obj, NULL,
       TO_ROOM);
  act ("$p flashes for a moment, then is gone.", ch, obj, NULL,
       TO_CHAR);
  if (!obj->in_room)
    {
      send_to_char ("This is a bug, please report.\n\r", ch);
      sbug ("donate: not in a room");
      bug_object_state (obj);
      return;
    }
  obj_from_room (obj);
  /* since teams are out, I just send them all to the safe area for now,
     you probably want to change this */
  obj_to_room (obj, safe_area);
  if (obj->in_room->people)
    {
      act ("$p appears in a `!bright`` flash!", 
	   obj->in_room->people, obj, NULL, TO_ROOM);
      act ("$p appears in a `!bright`` flash!", 
	   obj->in_room->people, obj, NULL, TO_CHAR);
    }
  ch->donate_num++;
}

void do_donate( CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  OBJ_DATA *obj_next;
  bool found;
  int max_don = 10;
  int count;
  
  one_argument( argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char("What do you want to donate?\n\r", ch);
      return;
    }

  if (ch->in_room == safe_area)
      {
	send_to_char ("You cannot donate anything from the safe area.\n\r",
		      ch);
	return;
      }

  if ( strcasecmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
      obj = get_obj_list( ch, arg, ch->in_room->contents );
      if ( obj == NULL )
	{
	  send_to_char("You don't see that.\n\r", ch);
	  return;
	}
      donate (ch, obj);
    }
  else
    {
      found = FALSE;
      for (obj = ch->in_room->contents; (obj != NULL) && (max_don > 0); 
	   obj = obj_next )
	{
	  obj_next = obj->next_content;
	  if (( arg[3] == '\0') || is_name(&arg[4], obj->name))
	    {
	      found = TRUE;
	      donate (ch, obj);
	      max_don --;
	    }
	}
      if ( !found )
	send_to_char( "I don't see that here.\n\r", ch );
    }
}

void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }

    
    if ( !strcmp( arg, "all"))
    {
	int x;
	for (x=1;x<MAX_WEAR;x++)
	{
        	remove_obj( ch, x, TRUE, 0 );
	}
        return; 

	} 


    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    remove_obj( ch, obj->wear_loc, TRUE, 0 );
    return;
}
