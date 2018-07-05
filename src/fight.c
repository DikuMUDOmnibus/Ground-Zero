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

int  check_char_alive   args  ( ( CHAR_DATA *ch));
int  test_char_alive   args  ( ( CHAR_DATA *ch));
void  bang_obj           args  ( ( OBJ_DATA  *obj, int perm_extract));
int  explode_obj        args  ( ( OBJ_DATA  *obj));
int  burn_obj           args  ( ( OBJ_DATA  *obj));
int  flash_obj          args  ( ( OBJ_DATA  *obj));
int  sound_obj          args  ( ( OBJ_DATA  *obj));
int  dark_obj 		args  ( (OBJ_DATA *obj, ROOM_DATA *room, int recurs_depth));
void char_death         args  ( ( CHAR_DATA *ch));
void report_condition   args  ( ( CHAR_DATA *ch, CHAR_DATA *victim));
int  damage_char        args  ( ( CHAR_DATA *ch, CHAR_DATA *attacker,
				 sh_int damage));
int  shoot_damage       args  ( ( CHAR_DATA *ch, CHAR_DATA *attacker,
				 sh_int damage));
int  damage_obj         args  ( ( OBJ_DATA *obj, sh_int damage_ch, 
				 sh_int damage_struct, 
				 CHAR_DATA *damage_from));
int  mod_hp_obj         args  ( ( OBJ_DATA *obj, sh_int damage_ch, 
			         sh_int damage_struct, 
				 CHAR_DATA *damage_from));
int  check_obj_alive    args  ( (OBJ_DATA *obj) );
int  damage_room        args  ( ( ROOM_DATA *a_room, sh_int damage_struct));
int  falling_wall       args  ( ( ROOM_DATA *a_room, sh_int dir));
void npc_act            args  ( ( CHAR_DATA *ch));
void move_random        args  ( ( CHAR_DATA *ch));

int been_pushed = 0;
int button_divisor = 6;
CHAR_DATA *extract_list = NULL;
int red_on = 0, blue_on = 0, green_on = 0, yellow_on = 0;
CHAR_DATA *tank_mob = NULL;

/* RELEASE: obviously the logic here was a little bit different when there
   were teams */
void adjust_kp (CHAR_DATA *killer, CHAR_DATA *victim)
{
  int transfer_amnt;

  if (killer == victim)
    killer = NULL;

  if (!IS_NPC (victim))
    victim->deaths++;

  if (killer)
    if (IS_NPC (victim))
      if (IS_NPC (killer))
	return;
      else
	{
	  if (!IS_IMMORTAL (killer))
	    killer->kills++;
	  return;
	}
    else
      killer->kills++;
}

int do_kill (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  if (ch->in_room == safe_area)
    {
      send_to_char ("There will be no hostilities in the safe room.  "
		    "Take it outside.\n\r", ch);
      return;
    }
  if (ch->in_room->inside_mob)
    {
      send_to_char ("There is not enough space in here to be violent.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if (!arg[0])
    {
      send_to_char ("You will now not attack anyone unless they attack "
		    "you.\n\r", ch);
      ch->fighting = NULL;
      return;
    }
  if ((victim = get_char_world (ch, arg)) != NULL)
    {
      if (victim == ch)
	{
	  send_to_char ("I'm sure you can find a more creative way to kill "
			"yourself.\n\n", ch);
	  return;
	}
      
      if (can_see_linear_char (ch, victim))
	if (ch->fighting == victim)
	  send_to_char ("You are doing your best!\n\r", ch);
	else
	  {
	    ch->fighting = victim;
	    if (!shoot_char (ch, victim, 1))
	      act ("You will shoot $M as soon as you have $t working "
		   "properly.\n\r", ch, (obj = get_eq_char (ch, WEAR_WIELD))
		   ? obj->short_descr : "your weapon", victim, TO_CHAR);
	  }
      else
	{
	  ch->fighting = victim;
	  act ("You will now attack $N on sight.\n\r", ch, NULL, victim, 
	       TO_CHAR);
	}
      return;
    }
  send_to_char ("They aren't here", ch);
}

int door_dir (CHAR_DATA *ch, CHAR_DATA *victim)
{
  int diff_x = ch->in_room->x - victim->in_room->x;
  int diff_y = ch->in_room->y - victim->in_room->y;

  if (diff_x)
    if (diff_x > 0)
      return DIR_WEST;
    else
      return DIR_EAST;
  else
    if (diff_y)
      if (diff_y > 0)
	return DIR_SOUTH;
      else
	return DIR_NORTH;
    else
      return DIR_DOWN;
}

/* out of necessity, some of this code is duplicated in shoot_damage */
int damage_char (CHAR_DATA *ch, CHAR_DATA *attacker, sh_int damage)
{
  /* let's find out who is REALLY responsible for this =) */
  if (attacker)
    while (attacker->owner)
      attacker = attacker->owner;
  if (attacker && (attacker != ch))
    ch->last_hit_by = attacker;
  ch->hit -= UMAX (1, damage - ch->armor);
  if (ch->desc)
    ch->desc->fcommand = TRUE;
  return check_char_alive (ch);
}

int shoot_damage (CHAR_DATA *ch, CHAR_DATA *attacker, sh_int damage)
{
  /* let's find out who is REALLY responsible for this =) */
  if (attacker)
    while (attacker->owner)
      attacker = attacker->owner;
  if (attacker && (attacker != ch))
    ch->last_hit_by = attacker;
  if (ch->interior && (ch->shield_dir == door_dir (ch, attacker)))
    ch->hit -= UMAX (1, damage - 450);
  else
    ch->hit -= UMAX (1, damage - ch->armor);
  if (ch->desc)
    ch->desc->fcommand = TRUE;
  return test_char_alive (ch);
}

int heal_char (CHAR_DATA *ch, sh_int bonus)
{
  ch->hit = UMIN (ch->max_hit, ch->hit + bonus);
  return 1;
}

int mod_hp_obj (OBJ_DATA *obj, sh_int damage_ch, sh_int damage_struct,
		CHAR_DATA *damage_from)
{
  if ((obj->hp_char == 30000) || (obj->hp_struct == 30000))
    return;
  if (damage_from)
    obj->owner = damage_from;
  obj->hp_char -= UMAX (1, damage_ch);
  obj->hp_struct -= UMAX (1, damage_struct);
}

int check_obj_alive (OBJ_DATA *obj)
{
  return ((obj->hp_char > 0) || (obj->hp_struct > 0));
}
int damage_obj ( OBJ_DATA *obj, sh_int damage_ch, sh_int damage_struct,
		CHAR_DATA *damage_from)
{
  mod_hp_obj (obj, damage_ch, damage_struct, damage_from);
  if (!check_obj_alive (obj))
    bang_obj (obj, obj->extract_me);
  return;
}

int  damage_room ( ROOM_DATA *a_room, sh_int damage_struct)
{
  const	sh_int	rev_dir		[]		=
    {
      2, 3, 0, 1, 5, 4
    };

  int wall_shock_constant = 60000;
  sh_int check_wall;
  ROOM_DATA *temp_room;

  for (check_wall = 0; check_wall < 6; check_wall++)
    {
      if (!IS_SET (a_room->exit[check_wall], EX_ISNOBREAKWALL) && 
	  IS_SET (a_room->exit[check_wall], EX_ISWALL))
	if (number_range (0, wall_shock_constant) < damage_struct)
	  {
	    temp_room = get_to_room(a_room, check_wall);
	    a_room->exit[check_wall] = 0;
	    temp_room->exit[rev_dir[check_wall]] = 0;
	    falling_wall (a_room, check_wall);
	    falling_wall (temp_room, rev_dir[check_wall]);
	    return 1;
	  }
    }
  return 0;
}

int falling_wall (ROOM_DATA *a_room, sh_int dir)
{
  char *	const	dir_name	[]		=
    {
      "north", "east", "south", "west", "up", "down"
    };

  CHAR_DATA *those_in_room, *next_char;
  if ((those_in_room = a_room->people) != NULL)
    {
      act ("The wall to the $t `aexplodes`` in a shower of rubble and debris.",
	   a_room->people, dir_name[dir], NULL, TO_CHAR);
      act ("The wall to the $t `aexplodes`` in a shower of rubble and debris.",
	   a_room->people, dir_name[dir], NULL, TO_ROOM);
      for (those_in_room = a_room->people; those_in_room; 
	   those_in_room = next_char)
	{
	  next_char = those_in_room->next_in_room;
	  send_to_char ("You are `1hit`` by falling rock!\n\r", those_in_room);
	  act ("$n is `1hit`` by falling rock!\n\r", those_in_room, NULL, NULL,
	       TO_ROOM);
	  damage_char (those_in_room, NULL, 4000);
	}
    }
}

void shoot_messages (int shots, CHAR_DATA *ch, CHAR_DATA *victim,
		     OBJ_DATA *weapon)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

  if (shots == 0)
    return;
  sprintf (buf2, "%d round%s", shots, (shots == 1) ? "" : "s");
  sprintf (buf, "$n `1shoots`` %s from $p at $N.", buf2);
  act (buf, ch, weapon, victim, TO_ROOM);
  sprintf (buf, "You `1shoot`` %s from $p at $N.", buf2);
  act (buf, ch, weapon, victim, TO_CHAR);
  if (ch->interior)
    {
      act ("You are momentarily deafened as the tank `1fires`` at $N.",
	   ch->interior->people, NULL, victim, TO_CHAR);
      act ("You are momentarily deafened as the tank `1fires`` at $N.",
	   ch->interior->people, NULL, victim, TO_ROOM);
    }
      
  sprintf (buf, "`!OUCH!`` $N `1shot`` you with %s%s$p!",
	   (shots != 1) ? buf2 : "", (shots != 1) ? " from " : "");
  act (buf, victim, weapon, ch, TO_CHAR);
  if (shots == 1)
    sprintf (buf, "$n `astaggers`` as $e is `1hit``!");
  else
    sprintf (buf, "$n `astaggers`` as $e is `1hit`` %d times!", shots);
  act (buf, victim, NULL, ch, TO_ROOM);
  if (victim->interior)
    {
      int diff_x = ch->in_room->x - victim->in_room->x;
      int diff_y = ch->in_room->y - victim->in_room->y;
      CHAR_DATA *occupants;

      if (diff_x)
	sprintf (buf, "You feel an `1impact`` from the %s.",
		 (diff_x > 0) ? "east" : "west");
      else
	if (diff_y)
	  sprintf (buf, "You feel an `1impact`` from the %s.",
		   (diff_y > 0) ? "north" : "south");
	else
	  sprintf (buf, "You feel an `1impact`` from below.");
      act (buf, victim->interior->people, victim, NULL, TO_ROOM);
      act (buf, victim->interior->people, victim, NULL, TO_CHAR);
    }
}

int shoot_char (CHAR_DATA *ch, CHAR_DATA *victim, int verify)
{
  sh_int dx, dy, start_x, start_y, pos_neg, count, shots;
  ROOM_DATA *current_room;
  OBJ_DATA *weapon, *clone;
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *real_victim;
  int real_shots = 0;

  weapon = get_eq_char (ch, WEAR_WIELD);

  if (!weapon)
    return 0;

  if (!weapon->contains)
    return 0;

  if (verify)
    return 1;

  if (weapon->wait_time++ < 0)
    return 1;

  if (weapon->rounds_per_second < 0)
    weapon->wait_time = weapon->rounds_per_second;

  /* RELEASE: the covetted question of WHEN does someone DI - here is the
     answer, for all the good it does you now =) */
#if 0
  if (ch->fighting && (ch->fighting != ch) &&
      (ch->in_room == ch->fighting->in_room) &&
      (get_num_rank (ch) == RANK_BADASS) &&
      (number_range (0, 50*KP_BADASS) < ch->kill_points) &&
      (ch->hit < 3000))
    {
      CHAR_DATA *next_ch;

      sprintf (log_buf, "%s goes ballistic.", ch->names);
      log_string (log_buf);
      send_to_char ("Your vision takes on a red tinge as blind rage wells up "
		    "within you.  When it\n\rpasses, your memory of what "
		    "happenned is foggy, but your enemies lie dead \n\raround "
		    "you and your hands are coated with their blood.\n\r", ch);
      act ("$n's eyes roll back into $s head.\n\r$s weapon drops from limp "
	   "fingers.\n\rA darkness seems to surround $m and swollow up the "
	   "room.\n\rSuddenly $s eyes snap open.  They are calm, deadly, and "
	   "devoid of humanity.\n\r$s movements are a blur.  Faster than "
	   "thought you find yourself face down on \n\rthe ground, gasping "
	   "for air through lungs that you no longer have.\n\r$n has "
	   "`!BECOME`` `1DEATH``!", ch, NULL, NULL, TO_ROOM);
      for (real_victim = ch->in_room->people; real_victim;
	   real_victim = next_ch)
	{
	  next_ch = real_victim->next_in_room;
	  if (real_victim == ch)
	    continue;
	  real_victim->last_hit_by = ch;
	  char_death (real_victim);
	}
      return 1;
    }
#endif

  for (shots = 0; shots < ((weapon->rounds_per_second > 0) ? 
       weapon->rounds_per_second : 1); shots++)
    {
      if (weapon->contains->ammo <= 0)
	{
	  send_to_char ("`aCLICK!``\n\r", ch);
	  act ("`aCLICK!``  What a pity, $n seems to have run out of ammo.", 
	       ch, NULL, NULL, TO_ROOM);
	  extract_obj (weapon->contains, weapon->contains->extract_me);
	  weapon->contains = NULL;
	  shoot_messages (real_shots, ch, victim, weapon);
	  return 0;
	}
      if (!IS_NPC (ch))
	weapon->contains->ammo--;
      
      start_x = ch->in_room->x;
      start_y = ch->in_room->y;
      dx = start_x - victim->in_room->x;
      dy = start_y - victim->in_room->y;
      if ((dx > 0) || (dy > 0))
	pos_neg = -1;
      else
	pos_neg = 1;
      
      real_victim = victim;
      if (dx || dy)
	for (dx ? (count = start_x + pos_neg) : (count = start_y + pos_neg);
	     dx ? (count != victim->in_room->x) :(count != victim->in_room->y);
	     count = count + pos_neg)
	  {
	    current_room = index_room (ch->in_room->this_level->rooms_on_level,
				       dx ? count : start_x, dy ? count : 
				       start_y);
	    if (current_room->people)
	      if (number_range (0, 10) + weapon->range - 4 > 0)
		{
		  act ("A bullet whizzes overhead.", current_room->people, 
		       NULL, NULL, TO_ROOM);
		  act ("A bullet whizzes overhead.", current_room->people, 
		       NULL, NULL, TO_CHAR);
		}
	      else
		{
		  act ("`!Oops.``  You `1hit`` $N by accident.  `!Doh!``",
		       ch, NULL, current_room->people, TO_CHAR);
		  real_victim = current_room->people;
		  break;
		}
	  }
      
      if (real_victim != victim)
	{
	  act ("`!OUCH!``  $N `1shot`` you with $p!", real_victim, weapon, ch, 
	       TO_CHAR);
	  act ("$n `astaggers`` as $e is `1hit``!", real_victim, NULL, ch, 
	       TO_ROOM);
	}
      else
	real_shots++;
      real_victim->fighting = ch;
      if (weapon->contains->extract_flags)
	{
	  clone = create_object(weapon->contains->pIndexData,0);
	  clone_object(weapon->contains,clone);
	  obj_to_char (clone, real_victim);
	  clone->owner = ch;
	  bang_obj (clone, 1);
	}
      else
	{
	  if (shoot_damage (real_victim, ch,
			    weapon->contains->damage_char[0]))
	    {
	      shoot_messages (real_shots, ch, victim, weapon);
	      check_char_alive (real_victim);
	      return 1; /* ie was shot to death */
	    }
	}
    }
  shoot_messages (real_shots, ch, victim, weapon);
  return 1;
}

void pull_obj (OBJ_DATA *obj, CHAR_DATA *ch, int silent)
{
  if (!obj->extract_flags)
    {
      if (!silent)
	send_to_char ("There is no pin on that.\n\r", ch);
      return;
    }
  if (obj->timer > 0)
    if (IS_SET (obj->extract_flags, EXTRACT_BURN_ON_EXTRACT))
      {
	if (!silent)
	  send_to_char ("It has already been lit.\n\r", ch);
	return;
      }
    else
      {
	if (!silent)
	  send_to_char ("It has already been pulled.\n\r", ch);
	return;
      }
  obj->timer = 10;
  obj->owner = ch;
  if (IS_SET (obj->extract_flags, EXTRACT_BURN_ON_EXTRACT))
    {
      if (!silent)
	act ("You bust out your handy lighter and `alight`` `Othe rag`` on "
	     "from $p.\n\rBetter get rid of that pal.  `!Looks to be "
	     "`1burning`` pretty fast there!``", ch, obj, NULL, TO_CHAR);
      act ("$n busts out a lighter and `alights`` `Othe rag`` on $p.", ch,
	   obj, NULL, TO_ROOM);
    }
  else
    {
      if (!silent)
	act ("You `apull`` `Othe pin`` from $p.\n\rBetter get rid of that "
	     "pal.  `!You only got 10 seconds!``", ch, obj, NULL, TO_CHAR);
      act ("$n `apulls`` `Othe pin`` from $p.", ch, obj, NULL, TO_ROOM);
    }
}

int do_pull (CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];

  if (ch->in_room == safe_area)
    {
      send_to_char ("There will be no hostilities in the safe room.  "
		    "Take it outside.\n\r", ch);
      return;
    }

  if (ch->in_room->inside_mob)
    {
      send_to_char ("There is not enough space in here to be violent.\n\r", ch);
      return;
    }

  if (!IS_NPC (ch) && (!ch->pcdata->account || !ch->pcdata->account[0]))
    {
      send_to_char ("You must be a registerred player to use explosives.", ch);
      return;
    }

  one_argument (argument, arg);
  if ( !str_cmp( arg, "all" ) || !str_prefix( "all.", arg ) )
    {
      int found = 0;
      /* 'pull all' or 'pull all.obj' */
      for ( obj = ch->carrying; obj != NULL; obj = obj->next_content)
	if ((obj->wear_loc == WEAR_NONE) &&
	    ((arg[3] == '\0') || is_name( &arg[4], obj->name)))
	  {
	    pull_obj (obj, ch, 1);
	    found = 1;
	  }
      
      if ( !found ) 
	{
	  if ( arg[3] == '\0' )
	    send_to_char( "I see nothing here.\n\r", ch );
	  else
	    act( "I see no $T here.", ch, NULL, &arg[4], TO_CHAR );
	}
      else
	send_to_char ("I'd run.\n\r", ch);
	  
      return;
    }

  if ((obj = get_obj_carry (ch, arg)) != NULL)
    pull_obj (obj, ch, 0);
  else
    send_to_char ("You don't have that.\n\r", ch);
}

/* Routes to correct _obj function */
void bang_obj (OBJ_DATA *obj, int perm_extract)
{
  if (!obj)
    {
      log_string ("REALBUG: Attempt to bang a NULL object.");
      return;
    }
  if (!obj->carried_by && !obj->in_room)
    {
      sbug ("REALBUG: Attempt to bang object not in room or on player (no longer extracting).");
      /*      extract_obj (obj, perm_extract || obj->extract_me);*/
      return;
    }
  
  if (((obj->in_room == safe_area) ||
       (obj->carried_by && (obj->carried_by->in_room == safe_area))) && 
      (safe_area->people))
    {
      CHAR_DATA *chars_hit;

      chars_hit = safe_area->people;
      act ("Aren't you glad you're in the safe room?", chars_hit, obj, NULL,
	   TO_CHAR);
      act ("Aren't you glad you're in the safe room?", chars_hit, obj, NULL,
	   TO_ROOM);
      extract_obj (obj, perm_extract || obj->extract_me);
      return;
    }

  if (!obj->extract_flags)
    {
      if (obj->in_room)
	{
	  if(obj->in_room->people)
	    {
	      act ("$p falls apart.", obj->in_room->people, obj, NULL, 
		   TO_CHAR);
	      act ("$p falls apart.", obj->in_room->people, obj, NULL, 
		   TO_ROOM);
	    }
	}
      else
	{
	  act ("$p falls apart.", obj->carried_by->in_room->people, obj, NULL,
	       TO_CHAR);
	  act ("$p falls apart.", obj->carried_by->in_room->people, obj, NULL,
	       TO_ROOM);
	}
    }
  else
    {
      if (IS_SET (obj->extract_flags, EXTRACT_EXPLODE_ON_EXTRACT))
	explode_obj (obj);
      if (IS_SET (obj->extract_flags, EXTRACT_BLIND_ON_EXTRACT))
	flash_obj (obj);
      if (IS_SET (obj->extract_flags, EXTRACT_BURN_ON_EXTRACT))
	burn_obj (obj);
      if (IS_SET (obj->extract_flags, EXTRACT_STUN_ON_EXTRACT))
	sound_obj (obj);
      if (IS_SET (obj->extract_flags, EXTRACT_DARK_ON_EXTRACT))
	dark_obj (obj, obj->in_room ? obj->in_room : obj->carried_by->in_room,
		  2);
    }

  extract_obj (obj, perm_extract || obj->extract_me);
}

int burn_obj (OBJ_DATA *obj)
{
  CHAR_DATA *chars_hit, *next_char;
  OBJ_DATA *the_fire;

  if (IS_SET (obj->general_flags, GEN_BURNS_ROOM))
    {
      /* guaranteed to be objservers from call */
      if ((chars_hit = obj->in_room->people) == NULL)
	return;
      for (; chars_hit; chars_hit = next_char)
	{
	  next_char = chars_hit->next_in_room;
	  if (IS_NPC (chars_hit))
	    continue;
	  act ("$n is `1burned``!", chars_hit, NULL, NULL, TO_ROOM);
	  act ("Blisters erupt on your skin as $p burns you!", chars_hit, 
	       obj, NULL, TO_CHAR);
	  damage_char (chars_hit, obj->owner, obj->damage_char[1]);
	}
      return;
    }

  if (obj->in_room)
    chars_hit = obj->in_room->people;
  else
    chars_hit = obj->carried_by->in_room->people;
      
  if (chars_hit)
    {
      act ("$p `!bursts`` into `1flame`` setting the room on fire!", 
	   chars_hit, obj, NULL, TO_ROOM);
      act ("$p `!bursts`` into `1flame`` setting the room on fire!", 
	   chars_hit, obj, NULL, TO_CHAR);
      for (; chars_hit; chars_hit = next_char)
	{
	  next_char = chars_hit->next_in_room;
	  if (IS_NPC (chars_hit))
	    continue;
	  act ("$n is `1burned``!", chars_hit, NULL, NULL, TO_ROOM);
	  act ("You are `1burned``!", chars_hit, obj, NULL, TO_CHAR);
	  damage_char (chars_hit, obj->owner, obj->damage_char[1]);
	}
    }

  if (obj->carried_by)
    obj_to_room (the_fire = create_object (get_obj_index(VNUM_FIRE), 0), 
		 obj->carried_by->in_room);
  else
    obj_to_room (the_fire = create_object (get_obj_index(VNUM_FIRE), 0), 
		 obj->in_room);

  the_fire->timer = obj->burn_time;
  the_fire->owner = obj->owner;

  return;
}

int flash_obj (OBJ_DATA *obj)
{
  CHAR_DATA *chars_hit;

  if (obj->in_room)
    chars_hit = obj->in_room->people;
  else
    chars_hit = obj->carried_by->in_room->people;
      
  if (chars_hit)
    {
      act ("$p `!flares`` brightly!", chars_hit, obj, NULL, TO_ROOM);
      act ("$p `!flares`` brightly!", chars_hit, obj, NULL, TO_CHAR);
    }
  else
    return;
  
  for (; chars_hit; chars_hit = chars_hit->next_in_room)
    {
      OBJ_DATA *prot_obj;

      if (((prot_obj = get_eq_char (chars_hit, WEAR_HEAD)) == NULL) ||
	  !IS_SET (prot_obj->general_flags, GEN_ANTI_BLIND))
	{
	  act ("$n is `!blinded``!", chars_hit, NULL, NULL, TO_ROOM);
	  act ("You are `!blinded``!", chars_hit, NULL, NULL, TO_CHAR);
	  SET_BIT (chars_hit->affected_by, AFF_BLIND);
	}
      else
	act ("Aren't you glad you are wearing $p?", chars_hit, prot_obj,
	     NULL, TO_CHAR);
    }

  return;
}

int sound_obj (OBJ_DATA *obj)
{
  CHAR_DATA *chars_hit;

  if (obj->in_room)
    chars_hit = obj->in_room->people;
  else
    chars_hit = obj->carried_by->in_room->people;

  for (; chars_hit; chars_hit = chars_hit->next_in_room)
    {
      act ("$n is `!dazed``!", chars_hit, NULL, NULL, TO_ROOM);
      act ("You are `!dazed``!", chars_hit, NULL, NULL, TO_CHAR);
      SET_BIT (chars_hit->affected_by, AFF_DAZE);
      }

  return;
}

int dark_obj (OBJ_DATA *obj, ROOM_DATA *room, int recurs_depth)
{
  int dir;
  OBJ_DATA *the_darkness;

  if (IS_SET (room->room_flags, ROOM_DARK))
    return;

  obj_to_room (the_darkness = create_object (get_obj_index(VNUM_DARK), 0), 
	       room);
  the_darkness->owner = obj->owner;
  the_darkness->timer = LIFETIME_SMOKE;

  if (room->people)
    {
      act ("The room fills with smoke making it impossible to see.",
	   room->people, NULL, NULL, TO_ROOM);
      act ("The room fills with smoke making it impossible to see.",
	   room->people, NULL, NULL, TO_CHAR);
    }

  if (!recurs_depth)
    return;

  for (dir = 0; dir < 4; dir++)
    if (!room->exit[dir])
      dark_obj (obj, get_to_room (room, dir), recurs_depth - 1);
}

int explode_obj (OBJ_DATA *obj)
{
  CHAR_DATA *chars_hit, *ch_next, *a_moron = NULL, *listenners;
  CHAR_DATA *the_owner = NULL;
  sh_int damage, counter, max_dist, dist_x, dist_y, count, dir, x_mod, y_mod;
  ROOM_DATA *start_room, *temp_room;
  OBJ_DATA *objs_hit, *obj_next, *first_obj;
  char buf[MAX_STRING_LENGTH];

  if (obj->carried_by)
    {
      a_moron = obj->carried_by;
      if (!IS_SET (obj->general_flags, GEN_CAN_BURY))
	if (obj->explode_desc[0] == "\0")
	  {
	    act ("$p `aexplodes`` in your hand!", obj->carried_by, obj, NULL,
		 TO_CHAR);
	    act ("$p `aexplodes`` in $n's hand!", obj->carried_by, obj, NULL,
		 TO_ROOM);
	  }
	else
	  {
	    sprintf (buf, "%s `aexplodes`` in your hand!", obj->explode_desc);
	    act (buf, obj->carried_by, NULL, NULL, TO_CHAR);
	    sprintf (buf, "%s `aexplodes`` in $n's hand!", obj->explode_desc);
	    act (buf, obj->carried_by, NULL, NULL, TO_ROOM);
	  }
      else
	{
	  act ("Oops!  $n `astepped`` on $p!", obj->carried_by, obj, NULL, 
	       TO_ROOM);
	  act ("Oops!  You `astepped`` on $p!", obj->carried_by, obj, NULL,
	       TO_CHAR);
	}
      start_room = obj->carried_by->in_room;
      obj_from_char (obj);
      chars_hit = start_room->people;
    }
  else
    {
      start_room = obj->in_room;
      obj_from_room (obj);
      chars_hit = start_room->people;
      if (chars_hit)
	{
	  act ("$p `aexplodes``, sending schrapnel flying!\n\r", chars_hit, 
	       obj, NULL, TO_CHAR);
	  act ("$p `aexplodes``, sending schrapnel flying!\n\r", chars_hit, 
	       obj, NULL, TO_ROOM);
	}
    }

  /* find damage extent */
  max_dist = UMAX (obj->damage_char[1] ? ((int) ((float) obj->damage_char[1] / 
			  (float) (obj->damage_char[1] - 
				   obj->damage_char[2]) - .5) + 1) : 0, 
		   obj->damage_structural[1] ?
		   ((int) ((float) obj->damage_structural[1] / 
			   (float) (obj->damage_structural[1] - 
				    obj->damage_structural[2]) - .5) + 1) : 0);
  /* damage chars in room where damage initiated */
  for (;chars_hit;chars_hit = ch_next)
    {
      ch_next = chars_hit->next_in_room;
      act ("You are `1hit`` by schrapnel!\n\r", chars_hit, NULL, NULL, 
	   TO_CHAR);
      act ("$n is `1hit`` by schrapnel!\n\r", chars_hit, NULL, NULL, TO_ROOM);
      chars_hit->last_hit_by = obj->owner;
      if (chars_hit != obj->owner)
	if (chars_hit != a_moron)
	  damage_char (chars_hit, obj->owner, obj->damage_char[1]);
	else
	  damage_char (chars_hit, obj->owner, obj->damage_char[0]);
      else
	the_owner = obj->owner;
    }
  /* ensure that the owner does get his kills if he killed others and 
     himself ie the owner must be damaged last. */
  if (the_owner)
    if (the_owner != a_moron)
      damage_char (the_owner, the_owner, obj->damage_char[1]);
    else
      damage_char (the_owner, the_owner, obj->damage_char[0]);
  the_owner = NULL;
  /* damage objects in room where damage initiated */
  objs_hit = start_room->contents;
  first_obj = objs_hit;
  for (;objs_hit;objs_hit = objs_hit->next_content)
    {
      if (!objs_hit->in_room)
	{
	  sprintf (log_buf, "%s could not be damaged correctly because it "
		   "was both in a room and not (was extracted). (local)", 
		   objs_hit->name);
	  log_string (log_buf);
	  extract_obj (objs_hit, objs_hit->extract_me);
	}
      sprintf (buf, "$p is `1hit`` by schrapnel!");
      if (objs_hit->in_room->people)
	{
	  act (buf, objs_hit->in_room->people, objs_hit, NULL, TO_CHAR);
	  act (buf, objs_hit->in_room->people, objs_hit, NULL, TO_ROOM);
	}
      mod_hp_obj (objs_hit, obj->damage_char[1], obj->damage_structural[1],
		  obj->owner);
    }
  /* All this convoluted stuff is done so that it doesn't happen that an object
     damages another object which explodes destroying another, then the first
     tries to damage the one already destroyed by what it causes to explode.
     ex.
     -> = causes to explode/be extracted
       AP mine -> clip
       AP mine -> grenade -> C4
       AP mine -> grenade -> pants
       AP mine -> C4 *** crash *** because was already destroyed by grenade
  */
  /* ship out the dead stuff */
  for (objs_hit = first_obj;objs_hit;objs_hit = obj_next)
    {
      obj_next = objs_hit->next_content;
      if (!check_obj_alive (objs_hit))
	{
	  objs_hit->destination = objs_hit->in_room;
	  if (!objs_hit->in_room)
	    sbug ("explode_obj1: obj not in room");
	  obj_from_room (objs_hit);
	  obj_to_room (objs_hit, explosive_area);
	}
    }
  /* damage walls in room where damage initiated */
  damage_room (start_room, obj->damage_structural[1]);
  /* figure out who all hears it */
  for (listenners = char_list; listenners; listenners = listenners->next)
    {
      dist_x = listenners->in_room->x - start_room->x;
      dist_y = listenners->in_room->y - start_room->y;
      if ((listenners->in_room->x >= 0) && (listenners->in_room->y >= 0) &&
	  (dist_x < 5) && (dist_x > -5) && (dist_y < 5) && (dist_y > -5) && 
	  (listenners->in_room->level == start_room->level) &&
	  (listenners->in_room != start_room))
	{
	  act ("You hear $p `aexplode`` nearby.", listenners, obj, NULL, 
	       TO_CHAR);
	}
    }

  /* go in each direction to do damage in other rooms */
  /* in retrospect, some recursion could have been done here to avoid copying
     alot of this code.  Wrote this in 95 though and changing this could
     affect stability.  11/8/98 */
  for (dir = 0; dir < 4; dir++)
    {
      temp_room = start_room;
      for (count = 0;!temp_room->exit[dir] && count < max_dist; count++)
	{
	  x_mod = y_mod = 0;
	  if (dir)
	    if (dir > 1)
	      if (dir > 2)
		x_mod = -1;
	      else
		y_mod = -1;
	    else
	      x_mod = 1;
	  else
	    y_mod = 1;
	  temp_room = index_room (temp_room, x_mod, y_mod);
	  /* damage the characters */
	  if (temp_room->people)
	    {
	      for (chars_hit = temp_room->people; chars_hit; 
		   chars_hit = ch_next)
		{
		  ch_next = chars_hit->next_in_room;
		  act ("You are `1hit`` by schrapnel!\n\r", chars_hit, NULL, 
		       NULL, TO_CHAR);
		  act ("$n is `1hit`` by schrapnel!\n\r", chars_hit, NULL, 
		       NULL, TO_ROOM);
		  chars_hit->last_hit_by = obj->owner;
		  if (chars_hit != obj->owner)
		    {
		      dist_x = chars_hit->in_room->x - start_room->x;
		      dist_y = chars_hit->in_room->y - start_room->y;
		      damage_char (chars_hit, obj->owner, 
				   obj->damage_char [1] -
				   (obj->damage_char[1] - 
				    obj->damage_char[2]) * 
				   (dist_x ? abs(dist_x) : abs(dist_y)));
		    }
		  else
		    the_owner = obj->owner;
		}
	    }
	  /* damage the objects */
	  if (temp_room->contents)
	    {
	      first_obj = objs_hit = temp_room->contents;
	      for (;objs_hit;objs_hit = obj_next)
		{
		  obj_next = objs_hit->next_content;
		  if (!objs_hit->in_room)
		    {
		      sprintf (log_buf, "%s could not be damaged correctly "
			       "because it was both in a room and not (was "
			       "extracted) (distant).", objs_hit->name);
		      log_string (log_buf);
		      extract_obj (objs_hit, objs_hit->extract_me);
		    }
		  if (objs_hit->in_room->people)
		    {
		      act ("$p is `1hit`` by schrapnel!", 
			   objs_hit->in_room->people, objs_hit, NULL, TO_CHAR);
		      act ("$p is `1hit`` by schrapnel!", 
			   objs_hit->in_room->people, objs_hit, NULL, TO_ROOM);
		    }
		  mod_hp_obj (objs_hit, (obj->damage_char[1] - count *
			       (obj->damage_char[1] - obj->damage_char[2])),
			      (obj->damage_structural[1] - count *
			       (obj->damage_structural[1] - 
				obj->damage_structural[2])), obj->owner);
		}
	      /* ship out the dead stuff */
	      for (objs_hit = first_obj;objs_hit;objs_hit = obj_next)
		{
		  obj_next = objs_hit->next_content;
		  if (!check_obj_alive (objs_hit))
		    {
		      objs_hit->destination = objs_hit->in_room;
		      if (!objs_hit->in_room)
			sbug ("explode_obj2: obj not in room");
		      obj_from_room (objs_hit);
		      obj_to_room (objs_hit, explosive_area);
		    }
		}
	    }
	  /* damage the walls */
	  damage_room (temp_room, obj->damage_structural[1]);
	}
    }
  /* ensure that the owner does get his kills if he killed others and 
     himself ie the owner must be damaged last. */
  if (the_owner)
    {
      dist_x = the_owner->in_room->x - start_room->x;
      dist_y = the_owner->in_room->y - start_room->y;
      damage_char (the_owner, the_owner, obj->damage_char [1] -
		   (obj->damage_char[1] - 
		    obj->damage_char[2]) * 
		   (dist_x ? abs(dist_x) : abs(dist_y)));
    }
  /* all the itsy bitsy pieces from destroyed stuff goes the way of the 
     dodo now. */
  while ((objs_hit = explosive_area->contents) != NULL)
    {
      if (objs_hit->destination)
	{
	  ROOM_DATA *temp_room;

	  if (!objs_hit->in_room)
	    sbug ("explode_obj3: obj not in room");
	  obj_from_room (objs_hit);
	  temp_room = objs_hit->destination;
	  objs_hit->destination = NULL;
	  obj_to_room (objs_hit, temp_room);
	}
      bang_obj (objs_hit, objs_hit->extract_me);
    }
  obj_to_room (obj, start_room);
}

int do_slay (CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *the_dead_one;
  char arg [MAX_INPUT_LENGTH];
  one_argument (argument, arg);

  if ((the_dead_one = get_char_room (ch,  arg)) != NULL)
    {
      if (the_dead_one->trust >= ch->trust)
	{
	  send_to_char ("Yea right pal.  Slay someone less than your "
			"trust level.\n\r", ch);
	  return;
	}
      act ("$n `acrams`` `Oa grenade`` down $N's throat.  You are coverred in "
	   "`1blood``.\n\r", ch, NULL, the_dead_one, TO_NOTVICT);
      act ("You `acram`` `Oa grenade`` down $N's throat.  You are coverred in "
	   "`1blood``.\n\r", ch, NULL, the_dead_one, TO_CHAR);
      act ("$n `acrams`` `Oa grenade`` down your throat.  You feel a "
	   "tremendous `1pain``\n\rin your stomach before you become a pasty "
	   "film coverring everything\n\rin the room.\n\r", ch, NULL, 
	   the_dead_one, TO_VICT);
      the_dead_one->last_hit_by = ch;
      char_death (the_dead_one);
    }
  else
    send_to_char ("They aren't here.\n\r", ch);

  return;
}

void update_pos (CHAR_DATA *ch)
{
  return;
}

void report_condition (CHAR_DATA *ch, CHAR_DATA *victim)
{
  char wound[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  float percent;

  if (victim->max_hit > 0)
    percent = victim->hit * 100 / victim->max_hit;
  else
    percent = -1;
  
  if (percent >= 100)
    sprintf(wound,"is in excellent condition.");
  else if (percent >= 90)
    sprintf(wound,"has a few scratches.");
  else if (percent >= 75)
    sprintf(wound,"has some small wounds and bruises.");
  else if (percent >= 50)
    sprintf(wound,"has quite a few wounds.");
  else if (percent >= 30)
    sprintf(wound,"has some big nasty wounds and scratches.");
  else if (percent >= 15)
    sprintf(wound,"looks pretty hurt.");
  else if (percent >= 0)
    sprintf(wound,"is in `2awful`c condition.");
  else
    sprintf(wound,"is `1bleeding`c to death.");
  
  act ("$N `c$t``", ch, wound, victim, TO_CHAR);
}

void chase (CHAR_DATA *ch, CHAR_DATA *vict)
{
  /* movement can kill the mover, return immediately */
  if (!number_range (0, 1))
    {
      if (vict->in_room->x > ch->in_room->x)
	{
	  do_east (ch, "");
	  return;
	}
      if (vict->in_room->x < ch->in_room->x)
	{
	  do_west (ch, "");
	  return;
	}
    }
  else
    {
      if (vict->in_room->y > ch->in_room->y)
	{
	  do_north (ch, "");
	  return;
	}
      if (vict->in_room->y < ch->in_room->y)
	{
	  do_south (ch, "");
	  return;
	}
    }
}

void move_random (CHAR_DATA *ch)
{
  switch (number_range (0, 3))
    {
    case 0: do_north (ch, "");
      break;
    case 1: do_south (ch, "");
      break;
    case 2: do_east (ch, "");
      break;
    case 3: do_west (ch, "");
      break;
    }
}

void npc_act (CHAR_DATA *ch)
{
  CHAR_DATA *enemies;

  if (ch->move_delay && (iteration%(ch->move_delay+1)))
    return;
  switch (ch->ld_behavior)
    {
    case BEHAVIOR_LD:
    case BEHAVIOR_PLAYER_CONTROL:
      return;
    case BEHAVIOR_PILLBOX:
    case BEHAVIOR_GUARD:
      if (!ch->fighting)
	for (enemies = ch->in_room->people; enemies; 
	     enemies = enemies->next_in_room)
	  if ((enemies != ch) && is_aggro (ch, enemies))
	    {
	      ch->fighting = enemies;
	      break;
	    }
      return;
    case BEHAVIOR_SEEKING_PILLBOX:
      chase (ch, ch->chasing);
      return;
    case BEHAVIOR_SEEK:
      if (ch->fighting && (ch->in_room->level == ch->fighting->in_room->level))
	{
	  chase (ch, ch->fighting);
	  return;
	}
      else
	{
	  for (enemies = char_list; enemies; enemies = enemies->next)
	    if ((ch->in_room->level == enemies->in_room->level) &&
		(ch->in_room->x - enemies->in_room->x <= 8) &&
		(ch->in_room->y - enemies->in_room->y <= 8) &&
		!IS_NPC (enemies) && is_aggro (ch, enemies))
	      ch->fighting = enemies;
	  return;
	}
      break;
    case BEHAVIOR_WANDER:
      if (ch->fighting && (ch->in_room->level == ch->fighting->in_room->level))
	{
	  chase (ch, ch->fighting);
	  return;
	}
      else
	{
	  move_random (ch);
	  return;
	}
    }
}

CHAR_DATA *next_violence;
/* reason for this being global is for concern about people offing those
   in front of them in the character list (see extract_char in handler.c */

void violence_update ()
{
  CHAR_DATA *ch;
  OBJ_DATA *obj;

  for (ch = char_list; ch; ch = next_violence)
    {
      next_violence = ch->next;
      if ((ch->in_room->level < 0) && !ch->desc && !IS_NPC(ch) && 
	  !IS_IMMORTAL (ch))
	do_teleport (ch, "");
      if (ch->valid != VALID_VALUE)
	{
	  sprintf (log_buf, "%s's character is corrupted", ch->names);
	  log_string (log_buf);
	}
      if (ch->in_room && ch->chasing && can_see_linear_char (ch, ch->chasing))
	ch->fighting = ch->chasing;
      if (ch->in_room && ch->fighting && (ch->in_room != safe_area) &&
	  (ch->in_room != store_area) &&
	  can_see_linear_char (ch, ch->fighting))
	if (!ch->interior || (ch->turret_dir == door_dir (ch, ch->fighting)))
	  {
	    if (shoot_char (ch, ch->fighting, 0))
	      if (ch->fighting)
		ch->report = 1;
	  }
      if (!ch->desc)
	npc_act (ch);
    }

  for (ch = char_list; ch; ch = ch->next)
    if (ch->report)
      {
	ch->report = 0;
	if (ch->fighting)
	  report_condition (ch, ch->fighting);
      }


  /* consistency check */
  fprintf (stderr, "*");
  for ( obj = object_list; obj != NULL; obj = obj->next ) 
    {
      if (!obj->in_room && !obj->carried_by && !obj->in_obj && 
	  !obj->destination)
	{
	  sprintf (log_buf, "%s is nowhere.", obj->name);
	  sbug (log_buf);
	};
      if (obj->valid != VALID_VALUE)
	{
	  sprintf (log_buf, "%s is corrupted", obj->name);
	  sbug (log_buf);
	}
      if (obj->in_room && ((obj->in_room->level > 2) || 
			   (obj->in_room->level < -1)))
	{
	  sprintf (log_buf, "%s is in an invalid room number", obj->name);
	  sbug (log_buf);
	}
    }

  return;
}

int test_char_alive (CHAR_DATA *ch)
{
  return (ch->hit <= 0);
}

int check_char_alive (CHAR_DATA *ch)
{
  OBJ_DATA *obj;
  int count;
  if (test_char_alive(ch))
    {
      char_death (ch);
      return 1;
    }
  else
    return 0;
}

void char_death (CHAR_DATA *ch)
{
  DESCRIPTOR_DATA *d;
  OBJ_DATA *obj, *obj_next;
  CHAR_DATA *all_chars, *killer;
  char message_buf[MAX_STRING_LENGTH] = "";
  
  if (!ground0_down)
    {
      if (ch->last_hit_by && ch->last_hit_by->interior)
	adjust_kp (find_manning (ch->last_hit_by->interior, MAN_TURRET), ch);
      else
	adjust_kp (ch->last_hit_by, ch);
    }

  /* RELEASE: here is your favorite code where it says that you wasted
     someone.  I just took it out for now because I'm too lazy to make it
     not take teams into account and too nice to remove all together -
     this code might be useful and hints at how alot of the team stuff
     was handled if you wanted to do something similar */
#if 0
  if (ch->last_hit_by && (ch->last_hit_by != ch))
    {
      killer = ch->last_hit_by;
      if ((killer->team == TEAM_NONE) && (ch->team == TEAM_NONE) &&
	  !IS_IMMORTAL (killer) && !IS_IMMORTAL (ch) &&
	  !IS_SET (killer->temp_flags, TRAITOR) &&
	  !IS_SET (ch->temp_flags, TRAITOR) && !IS_NPC (ch) &&
	  !IS_NPC (killer) && (ch->pcdata->solo_hit > 5000) &&
	  !ground0_down)
	{
	  killer->max_hit = ++killer->pcdata->solo_hit;
	  ch->max_hit = --ch->pcdata->solo_hit;
	}
      if ((ch->team != killer->team) || (killer->team == TEAM_NONE))
	{
	  if (ch->max_hit >= 5000)
	    sprintf(message_buf, "%s(%s) just `!wasted`` %s(%s).  "
		      "%s\n\r", IS_NPC (killer) ?
		      killer->short_descript : killer->names,
		      team_table[killer->team].f_color_name,
		      IS_NPC (ch) ? ch->short_descript : ch->names, 
		      team_table[ch->team].f_color_name,
		    killer->kill_msg ? killer->kill_msg : "CARNAGE!!!!");
	}
      else
	{
	  sprintf(message_buf, "%s(%s) just `!wasted`` %s(%s).  "
		  "Accidents happen . . .\n\r", IS_NPC (killer) ? 
		  killer->short_descript : killer->names, 
		  team_table[killer->team].f_color_name,
		  IS_NPC (ch) ? ch->short_descript : ch->names, 
		  team_table[ch->team].f_color_name);
	}
    }
  else
    {
      sprintf(message_buf, "%s(%s) died of natural causes.  What a wimp!\n\r", 
	      IS_NPC(ch) ? ch->short_descript : ch->names,
	      team_table[ch->team].f_color_name);
      killer = ch;
    }
#endif

  if (ch->last_hit_by)
    killer = ch->last_hit_by;
  else
    killer = ch;
  if (!IS_NPC (killer) && !ground0_down)
    save_char_obj (killer);
  top_stuff (killer);
  top_stuff (ch);
  log_string(message_buf);
  send_to_char ("`!YOU ARE DEAD!``\n\r"
		"Welp, you like bought it, ya know?\n\r"
		"I mean you are like dead, etc.\n\r"
		"You are experiencing the denial stage right now.\n\r"
		"It'll pass . . . into darkness . . .\n\r", ch);
  act ("$n is `!DEAD``!", ch, NULL, NULL, TO_ROOM);
  if (message_buf[0])
    do_kills (NULL, message_buf);
  
  for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
      obj_next = obj->next_content;
      if (obj->carried_by != ch)
	sbug ("char_death: carried_by not ch");
      if (obj->extract_me)
	{
	  act ("$p drops to the ground and falls apart, useless.", 
	       ch, obj, NULL, TO_ROOM);
	  obj_from_char (obj);
	  extract_obj(obj, obj->extract_me);
	}
      else
	{
	  act ("$p drops to the ground.", ch, obj, NULL, TO_ROOM);
	  obj_from_char (obj);
	  obj_to_room (obj, ch->in_room);
	}
    }
  ch->armor = 0;
  ch->carry_number = 0;

  REMOVE_BIT (ch->affected_by, AFF_BLIND);
  REMOVE_BIT (ch->affected_by, AFF_DAZE);

  if (ch->interior)
    {
      OBJ_DATA *obj = revert_tank (ch);
      
      obj->owner = ch->last_hit_by;
      bang_obj (obj, obj->extract_me);
      return;
    }
  if (!IS_NPC (ch))
    {
      save_char_obj( ch );
    }
  d = ch->desc;
  ch->hit = 1500;
  if (!d || ground0_down)
    {
      if (!ground0_down)
	extract_char( ch, FALSE );
      ch->next_extract = extract_list;
      extract_list = ch;
    }
  else
    {
      extract_char( ch, FALSE );
      ch->last_hit_by = NULL;
      if (ch->chan_delay)
	{
	  WAIT_STATE (ch, ch->chan_delay);
	  ch->chan_delay *= 2;
	}
      else
	ch->chan_delay = PULSE_PER_SECOND;
    }
  return;
}

int do_bury (CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj, *the_mine;
  char buf[MAX_INPUT_LENGTH];

  if (!IS_NPC (ch) && (!ch->pcdata->account || !ch->pcdata->account[0]))
    {
      send_to_char ("You must be a registerred player to use explosives.", ch);
      return;
    }

  for (obj = ch->in_room->contents; obj; obj= obj->next_content)
    if (IS_SET (obj->general_flags, GEN_CAN_BURY))
      break;
  if (!obj)
    {
      send_to_char ("You don't see that in this room.", ch);
      return;
    }
  if (obj->timer > 0)
    {
      send_to_char ("You cannot bury a live object.\n\r", ch);
      return;
    }
  if ((the_mine = ch->in_room->mine) != NULL)
    {
      send_to_char ("Oops.  Someone beat you to it!  DOH!!!\n\r", ch);
      the_mine->destination = NULL;
      ch->in_room->mine = NULL;
      obj_to_char (the_mine, ch);
      bang_obj (the_mine, 0);
      return;
    }
  obj->owner = ch;
  act ("$n buries $p.", ch, obj, NULL, TO_ROOM);
  act ("You bury $p.", ch, obj, NULL, TO_CHAR);
  if (!obj->in_room)
    sbug ("do_bury: obj not in room");
  obj_from_room (obj);
  obj->destination = ch->in_room;
  ch->in_room->mine = obj;
  return;
}

int do_track (CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char *west, *east, *south, *north, *here, buf[MAX_STRING_LENGTH];

  send_to_char ("You stop to examine your tracking device.\n\r", ch);
  WAIT_STATE (ch, 3);
  if ((victim = get_char_world (ch, argument)) && 
      (victim->in_room->this_level == ch->in_room->this_level) &&
      (victim->in_room->x >= 0) && (victim->in_room->y >= 0))
    {
      west = (victim->in_room->x < ch->in_room->x) ? "west" : "";
      east = (victim->in_room->x > ch->in_room->x) ? "east" : "";
      south = (victim->in_room->y < ch->in_room->y) ? "south" : "";
      north = (victim->in_room->y > ch->in_room->y) ? "north" : "";
      here = ((victim->in_room->x == ch->in_room->x) &&
	      (victim->in_room->y == ch->in_room->y)) ? 
		"RIGHT IN THE ROOM WITH YOU!" : " of you.";
      sprintf (buf, "%s is %s%s%s%s%s", victim->names, north, south, east, 
	       west, here);
      send_to_char (buf, ch);
    }
  else
    send_to_char ("Your tracking device cannot pick up their signal.\n\r",
		  ch);
  return;
}

#if 0
void do_explode (CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj, *next_obj;
  ROOM_DATA *room_was_in = ch->in_room;

  if (ch->in_room == safe_area)
    {
      send_to_char ("There will be no hostilities in the safe room.  "
		    "Take it outside.\n\r", ch);
      return;
    }

  if (ch->in_room->inside_mob)
    {
      send_to_char ("There is not enough space in here to be violent.\n\r", ch);
      return;
    }

  if (ch->hit < ch->max_hit)
    {
      send_to_char ("Coward!  Wanna get out the easy way huh?  Huh?!  Die "
		    "like a man!  Grit your\n\rteeth and laugh the pain off "
		    "instead of taking the quick exit from your\n\rpathetic "
		    "existance!  Wimp!\n\r", ch);
      return;
    }

  act ("$n activates a small device that causes every item in his inventory "
       "to `1EXPLODE``!", ch, NULL, NULL, TO_ROOM);
  send_to_char ("You activate your self destruct device, setting off every "
		"explosive item in your inventory!\n\r", ch);
  for (obj = ch->carrying; obj; obj = next_obj)
    {
      next_obj = obj->next_content;
      if ((obj->item_type == ITEM_EXPLOSIVE) ||
	  (obj->item_type == ITEM_EXPLOSIVE_ROUNDS))
	{
	  obj->owner = ch;
	  bang_obj (obj, 0);
	  if (!ch || (ch->in_room != room_was_in))
	    break;
	}
    }

  if (ch && (ch->in_room != room_was_in))
    save_char_obj (ch);
  return;
}


void do_boo (CHAR_DATA *ch, char *argument)
{
  send_to_char ("Type it all out . . . 'boom' if you want to self-destruct.",
		ch);
  return;
}

void do_boom (CHAR_DATA *ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  ROOM_DATA *room_was_in = ch->in_room;

  if (ch->in_room == safe_area)
    {
      send_to_char ("There will be no hostilities in the safe room.  "
		    "Take it outside.\n\r", ch);
      return;
    }

  if (ch->in_room->inside_mob)
    {
      send_to_char ("There is not enough space in here to be violent.\n\r", ch);
      return;
    }

  if ((!been_pushed && (ch->hit == ch->max_hit)) || IS_IMMORTAL (ch))
    {
      act ("$n has self destructed.", ch, NULL, NULL, TO_ROOM);
      sprintf (buf, "%s has boomed out.", ch->name);
      log_string (buf);

      do_explode (ch, "");

      if (ch->in_room == room_was_in)
	{
	  adjust_kp (NULL, ch);
	  save_char_obj (ch);
	}
      send_to_char ("See ya here next time you get the urge to kill.  "
		    "Grin.\n\r", ch);
      if (ch->desc)
	close_socket (ch->desc);
      extract_char (ch, TRUE);
      return;
    }
  if (ch->hit < ch->max_hit)
    {
      send_to_char ("Your not in too good shape anyway, might as well just "
		    "wait for someone to wax\n\ryour silly ass.  (if you "
		    "really need to leave, ld will let you drop link).\n\r", 
		    ch);
      return;
    }
  else
    {
      send_to_char ("Sorry pal, button been pressed, lose link if you really "
		    "gotta go.  Command for that is ld.\n\rIf you just want "
		    "the damage, the command is explode.\n\r", ch);
      return;
    }
}
#endif

CHAR_DATA *find_manning (ROOM_DATA *room, long int man_pos)
{
  CHAR_DATA *tracker;

  for (tracker = room->people; tracker; tracker = tracker->next_in_room)
    if (IS_SET (tracker->temp_flags, man_pos))
      break;
  return tracker;
}

CHAR_DATA *convert_tank (OBJ_DATA *tank)
{
  CHAR_DATA *mob = clone_mobile (tank_mob);
  CHAR_DATA *ch;

  log_string ("converting tank");
  mob->interior = tank->interior;
  tank->interior = NULL;
  mob->interior->interior_of = NULL;
  mob->interior->inside_mob = mob;
  char_from_room (mob);
  char_to_room (mob, tank->in_room);
  obj_from_room (tank);
  /* isn't it ironic that the tank is in the room that used to be inside it? */
  obj_to_room (tank, mob->interior);
  mob->hit = tank->hp_char;
  act ("The hatch closes and the tank turns on.", mob->interior->people, NULL,
       NULL, TO_ROOM);
  act ("The hatch closes and the tank turns on.", mob->interior->people, NULL,
       NULL, TO_CHAR);
  return mob;
}

OBJ_DATA *revert_tank (CHAR_DATA *tank)
{
  OBJ_DATA *obj;

  for (obj = tank->interior->contents; obj; obj = obj->next_content)
    if (obj->item_type == ITEM_TEAM_VEHICLE)
      break;

  if (!obj)
    sbug ("revert_tank: Could not find tank object within mob.");

  log_string ("reverting tank");
  obj_from_room (obj);
  obj->interior = tank->interior;
  tank->interior = NULL;
  obj->interior->interior_of = obj;
  obj->interior->inside_mob = NULL;
  obj_to_room (obj, tank->in_room);
  obj->hp_char = tank->hit;
  extract_char (tank, FALSE);
  tank->next_extract = extract_list;
  extract_list = tank;
  act ("The hatch opens and the tank turns off.", obj->interior->people, NULL,
       NULL, TO_ROOM);
  act ("The hatch opens and the tank turns off.", obj->interior->people, NULL,
       NULL, TO_CHAR);
  return obj;
}

int verify_tank (ROOM_DATA *room)
{
  CHAR_DATA *gun, *turret, *drive, *ch;
  int count = 0;

  log_string ("verifying tank");
  if (((drive = find_manning (room, MAN_DRIVE)) != NULL) &&
      ((gun = find_manning (room, MAN_SHIELD)) != NULL) &&
      ((turret = find_manning (room, MAN_TURRET)) != NULL))
    {
      log_string ("found all positions.");
      log_string ("about to count");
      for (ch = room->people; ch; ch = ch->next_in_room)
	count++;
      if (count == 3)
	return 1;
    }
  return 0;
}

void stop_manning (CHAR_DATA *ch)
{
  REMOVE_BIT (ch->temp_flags, MAN_SHIELD);
  REMOVE_BIT (ch->temp_flags, MAN_TURRET);
  REMOVE_BIT (ch->temp_flags, MAN_DRIVE);
  if (ch->in_room->inside_mob)
    if (!verify_tank (ch->in_room))
      revert_tank (ch->in_room->inside_mob);
}  

int do_man (CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *manning;

  if (!IS_SET (ch->temp_flags, IN_TANK))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  stop_manning (ch);
  if (!argument[0])
    {
      send_to_char ("What do you want to man?\n\r", ch);
      return;
    }
  if (!str_prefix (argument, "shield"))
    {
      if ((manning = find_manning(ch->in_room, MAN_SHIELD)) != NULL)
	{
	  act ("$N is manning the shield position right now.", ch,
	       NULL, manning, TO_CHAR);
	  return;
	}
      SET_BIT (ch->temp_flags, MAN_SHIELD);
      send_to_char ("You man the shield position.\n\r", ch);
      act ("$n mans the shield position.\n\r", ch, NULL, NULL, TO_ROOM);
      if (verify_tank (ch->in_room))
	convert_tank (ch->in_room->interior_of);
      return;
    }
  if (!str_prefix (argument, "turret"))
    {
      if ((manning = find_manning(ch->in_room, MAN_TURRET)) != NULL)
	{
	  act ("$N is manning the turret right now.", ch,
	       NULL, manning, TO_CHAR);
	  return;
	}
      SET_BIT (ch->temp_flags, MAN_TURRET);
      send_to_char ("You man the turret.\n\r", ch);
      act ("$n mans the turret.\n\r", ch, NULL, NULL, TO_ROOM);
      if (verify_tank (ch->in_room))
	convert_tank (ch->in_room->interior_of);
      return;
    }
  if (!str_prefix (argument, "drive"))
    {
      if ((manning = find_manning(ch->in_room, MAN_DRIVE)) != NULL)
	{
	  act ("$N is driving right now.", ch,
	       NULL, manning, TO_CHAR);
	  return;
	}
      SET_BIT (ch->temp_flags, MAN_DRIVE);
      send_to_char ("You man the driving position.\n\r", ch);
      act ("$n mans the driving position.\n\r", ch, NULL, NULL, TO_ROOM);
      if (verify_tank (ch->in_room))
	convert_tank (ch->in_room->interior_of);
      return;
    }
  send_to_char ("Please read help tank to determine which positions can be "
		"manned.\n\r", ch);
}

#if 0
int do_turn (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH];
  sh_int dir;
  if (!IS_SET (ch->temp_flags, IN_TANK))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  if (!IS_SET (ch->temp_flags, MAN_TURRET))
    {
      send_to_char ("You must man the turret to do that.", ch);
      return;
    }
  one_argument (argument, arg);
  if (!str_prefix (arg, "north"))
    {
      dir = 0;
      sprintf (message, "north");
    }
  else
    if (!str_prefix (arg, "south"))
      {
	dir = 2;
	sprintf (message, "south");
      }
    else
      if (!str_prefix (arg, "east"))
	{
	  dir = 1;
	  sprintf (message, "east");
	}
      else
	if (!str_prefix (arg, "west"))
	  {
	    dir = 3;
	    sprintf (message, "west");
	  }
	else
	  {
	    send_to_char ("What dirction do you want to turn it?", 
			  ch);
	    return;
	  }
  act ("You turn the turret to face $t.", ch, message, NULL, TO_CHAR);
  act ("$n turns the turret to face $t.", ch, message, NULL, TO_ROOM);
  ch->in_room->interior_of->orientation = dir;
  if (ch->in_room->interior_of->in_room->people)
    {
      act ("$p's turret slowly swivels to face $T.", 
	   ch->in_room->interior_of->in_room->people, ch->in_room->interior_of,
	   message, TO_CHAR);
      act ("$p's turret slowly swivels to face $T.", 
	   ch->in_room->interior_of->in_room->people, ch->in_room->interior_of,
	   message, TO_ROOM);
    }
  return;
}

int do_drive (CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char *the_dir;
  ROOM_DATA *temp_room;
  OBJ_DATA *the_tank, *obj;
  CHAR_DATA *fch;
  sh_int dir, x_mod, y_mod, distance;
  if (!IS_SET (ch->temp_flags, IN_TANK))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  if (!IS_SET (ch->temp_flags, MAN_DRIVE))
    {
      send_to_char ("You must man the drive control to do that.", ch);
      return;
    }
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
	    send_to_char ("What dirction do you want to drive it?", 
			  ch);
	    return;
	  }
  if (ch->in_room->interior_of->in_room->exit[dir])
    {
      send_to_char ("A wall blocks your way.", ch);
      return;
    }

  the_tank = ch->in_room->interior_of;
  temp_room = index_room (the_tank->in_room, x_mod, y_mod);

  for (obj = temp_room->contents; obj; obj = obj->next_content)
    if (obj->item_type == ITEM_TEAM_VEHICLE)
      {
	send_to_char ("Two tanks cannot fit into a single room.", ch);
	return;
      }

  act ("You drive $P $t.", ch, message, the_tank, TO_CHAR);
  act ("$n manipulates the controls to drive $t.", ch, message, NULL, TO_ROOM);
  sprintf (buf, "`D%s`` [`X%d``, `X%d``] L - `L%d``\n\r", 
	   temp_room->name, temp_room->x, temp_room->y, temp_room->level);
  send_to_char( buf, ch );
  send_to_char("\n\r",ch);
  show_exits( temp_room, ch, "auto" );
  show_list_to_char( temp_room->contents, ch, FALSE, FALSE );
  show_char_to_char( temp_room->people, ch );
  obj_from_room (the_tank);
  obj_to_room (the_tank, temp_room);

  for (fch = char_list; fch; fch = fch->next)
    {
      if (ch == fch)
	continue;
      if (IS_SET (fch->in_room->room_flags, ROOM_TANK))
	continue; /* add tank seeing tank code here */
      if (!can_see_linear_room (fch, the_tank->in_room))
	    continue;
      if (the_tank->in_room->x - fch->in_room->x > 0)
	{
	  distance = the_tank->in_room->x - fch->in_room->x;
	  the_dir = "east";
	}
      else
	if (the_tank->in_room->x - fch->in_room->x < 0)
	  {
	    distance = fch->in_room->x - the_tank->in_room->x;
	    the_dir = "west";
	  }
	else
	  if (the_tank->in_room->y - fch->in_room->y > 0)
	    {
	      distance = the_tank->in_room->y - fch->in_room->y;
	      the_dir = "north";
	    }
	  else
	    if (the_tank->in_room->y - fch->in_room->y < 0)
	      {
		distance = fch->in_room->y - the_tank->in_room->y;
		the_dir = "south";
	      }
	    else
	      {
		act ("$p has arrived.", fch, the_tank, NULL, TO_CHAR);
		continue;
	      }
      sprintf (buf, "$p has arrived %d %s.", distance, the_dir);
      act (buf, fch, the_tank, NULL, TO_CHAR);
    }
  if (temp_room->mine)
    {
      temp_room->mine->destination = NULL;
      obj_to_room (temp_room->mine, temp_room);
      bang_obj (temp_room->mine, 0);
      temp_room->mine = NULL;
    }
  return;
}

int do_fire (CHAR_DATA *ch, char *argument)
{
  ROOM_DATA *shoot_room, *prev_room;
  OBJ_DATA *the_tank, *the_shell;
  char arg[MAX_INPUT_LENGTH];
  sh_int the_num, count;
  unsigned int pexit;

  if (!IS_SET (ch->temp_flags, IN_TANK))
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return;
    }
  if (!IS_SET (ch->temp_flags, MAN_GUN))
    {
      send_to_char ("You must man the gun to do that.\n\r", ch);
      return;
    }
  the_tank = ch->in_room->interior_of;
  one_argument (argument, arg);
  the_num = 1;
  if (is_number (arg))
    the_num = atoi (arg);
  else
    {
      send_to_char ("syntax: fire <distance>\n\r", ch);
      return;
    }
  if (the_num > 6)
    {
      send_to_char ("The tank cannot fire a shell that far.  You fire it as "
		    "far as you can.\n\r", ch);
      the_num = 6;
    }
  else
    if (the_num < 3)
      {
	send_to_char ("The tank cannot fire at a trajectory that will achieve "
		      "that close range.  You fire as close as you can.\n\r",
		      ch);
	the_num = 3;
      }
  shoot_room = the_tank->in_room;
  for (count = 0; count < the_num; count++)
    {
      prev_room = shoot_room;
      pexit = prev_room->exit[the_tank->orientation];
      if ( IS_SET (pexit, EX_CLOSED) || IS_SET (pexit, EX_ISWALL))
	{
	  shoot_room = prev_room;
	  break;
	}
      shoot_room = get_to_room (prev_room, the_tank->orientation);
      if (!rooms_linear_with_no_walls (the_tank->in_room, shoot_room))
	{
	  shoot_room = prev_room;
	  break;
	}
    }
  send_to_char ("You fire the shell then start reloading . . .\n\r", ch);
  act ("$n fires a shell, then starts reloading . . .", ch, NULL, NULL,
       TO_ROOM);
  if (ch->in_room->interior_of->in_room->people)
    {
      act ("The explosive sound of $p firing echos in your ears.", 
	   ch->in_room->interior_of->in_room->people, ch->in_room->interior_of,
	   NULL, TO_CHAR);
      act ("The explosive sound of $p firing echos in your ears.", 
	   ch->in_room->interior_of->in_room->people, ch->in_room->interior_of,
	   NULL, TO_ROOM);
    }
  obj_to_room (the_shell = 
	       create_object (get_obj_index(VNUM_TANK_SHELL), 0), 
	       shoot_room);
  the_shell->owner = ch;
  bang_obj (the_shell, 1);
  WAIT_STATE (ch, 30);
  return;
}
#endif
