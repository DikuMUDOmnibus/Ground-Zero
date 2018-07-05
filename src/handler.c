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
#include <string.h>
#include <time.h>
#include "ground0.h"

AFFECT_DATA *		affect_free;



/*
 * Local functions.
 */
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void    scatter_obj     args( ( OBJ_DATA *obj) );
bool    can_see_linear_char  args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
bool    can_see_linear_room  args( ( CHAR_DATA *ch, ROOM_DATA *a_room) );
bool    rooms_linear_with_no_walls  args( ( ROOM_DATA *room1, 
					   ROOM_DATA *room2) );
CHAR_DATA *char_file_active args ((char *argument ) );


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
  return ch->trust;
}

/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    return 17 + ( ch->played + (int) (current_time - ch->logon) ) / 72000;
}

	
/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->trust )
	return 1000;

    return 150;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->trust )
	return 1000000;

    return 500;
}


bool is_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;


    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_cmp(string,name))
		return TRUE; /* full pattern match */

	    if (!str_prefix(part,name))
		break;
	}
    }
}

bool is_name_exact ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;


    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_cmp(string,name))
		return TRUE; /* full pattern match */

	    if (!str_cmp(part,name))
		break;
	}
    }
}



/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ch->in_room == NULL )
    {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }

    if ( ch == ch->in_room->people )
    {
	ch->in_room->people = ch->next_in_room;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	{
	    if ( prev->next_in_room == ch )
	    {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Char_from_room: ch not found.", 0 );
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    return;
}



/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if ( pRoomIndex == NULL )
    {
	bug( "Char_to_room: NULL.", 0 );
	return;
    }

    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;

    return;
}

void bug_object_state (OBJ_DATA *obj)
{
  if (obj->in_room)
    {
      sprintf (log_buf, "in room %d, %d, %d.", obj->in_room->x,
	       obj->in_room->y, obj->in_room->level);
      sbug (log_buf);
    }
  if (obj->destination)
    {
      sprintf (log_buf, "has destination %d, %d, %d.", obj->destination->x,
	       obj->destination->y, obj->destination->level);
      sbug (log_buf);
    }
  if (obj->in_obj)
    {
      sprintf (log_buf, "inside object %s.", obj->in_obj->name);
      sbug (log_buf);
    }
  if (obj->carried_by)
    {
      sprintf (log_buf, "carried by %s.", obj->carried_by->names);
      sbug (log_buf);
    }
}

/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
  if (obj->in_room || obj->destination || obj->in_obj || obj->carried_by)
    {
      sprintf (log_buf, "obj_to_char: %s could not go to %s because it is "
	       "somewhere.", obj->name, ch->names);
      sbug (log_buf);
      bug_object_state (obj);
      return;
    }
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;

    if ( ( ch = obj->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch.", 0 );
	bug_object_state (obj);
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }

    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );
    return;
}


/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
      if (obj->carried_by != ch)
	{
	  char buf[MAX_STRING_LENGTH];
	  if (obj->carried_by)
	    sprintf (buf, "get_eq_char: Carried by %s; in list of %s",
		     obj->carried_by->names, ch->names);
	  else
	    sprintf (buf, "get_eq_char: Carried by null; in list of %s",
		     ch->names);
	  sbug (buf);
	}
      if ( obj->wear_loc == iWear )
	return obj;
    }

    return NULL;
}

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *find_eq_char (CHAR_DATA *ch, int search_type,
			unsigned int search_value)
{
  OBJ_DATA *obj;
  char buf[MAX_INPUT_LENGTH];
  
  if (ch == NULL)
    return NULL;
  
  for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
      if (obj->carried_by != ch)
	sbug ("find_eq_char: carried_by not ch");
      switch (search_type)
	{
	case SEARCH_ITEM_TYPE:
	  if ( obj->item_type == search_value )
	    return obj;
	  break;
	case SEARCH_GEN_FLAG:
	  if (IS_SET (obj->general_flags, search_value))
	    return obj;
	  break;
	case SEARCH_AMMO_TYPE:
	  if ((obj->item_type == ITEM_AMMO) &&
	      (obj->ammo_type == search_value))
	    return obj;
	  break;
	}	  
    }

  return NULL;
}

/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    int i;

    if ( get_eq_char( ch, iWear ) != NULL )
    {
	bug( "Equip_char: already equipped (%d).", iWear );
	return;
    }

    ch->armor += obj->armor;
    obj->wear_loc	 = iWear;
    ch->carry_number	-= get_obj_number( obj );

    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf;
    int i;

    if ( obj->wear_loc == WEAR_NONE )
    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    obj->wear_loc	 = -1;
    ch->armor -= obj->armor;
    ch->carry_number	+= get_obj_number( obj );

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_DATA *in_room;

    if ( ( in_room = obj->in_room ) == NULL )
    {
	bug( "obj_from_room: NULL room.", 0 );
	return;
    }

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

    if ((obj->in_room == explosive_area) && obj->in_room->people)
      {
	act ("$p flashes out of existence to be destroyed somewhere.", 
	     obj->in_room->people, obj, NULL, TO_ROOM);
	act ("$p flashes out of existence to be destroyed somewhere.", 
	     obj->in_room->people, obj, NULL, TO_CHAR);
      }

    if (IS_SET (obj->general_flags, GEN_DARKS_ROOM))
      REMOVE_BIT (obj->in_room->room_flags, ROOM_DARK);

    obj->in_room      = NULL;
    obj->next_content = NULL;

    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_DATA *pRoomIndex )
{
  if (obj->in_room || 
      (obj->destination && (pRoomIndex != explosive_area)) || 
      obj->in_obj || obj->carried_by)
    {
      sprintf (log_buf, "obj_to_room: %s could not go to %d,%d,%d because "
	       "it is somewhere.", obj->name, pRoomIndex->x, 
	       pRoomIndex->y, pRoomIndex->level);
      sbug (log_buf);
      bug_object_state (obj);
      return;
    }

  if ((pRoomIndex == explosive_area) && (explosive_area->people))
    {
      act ("$p flashes into existance.", explosive_area->people, obj, NULL, 
	   TO_ROOM);
      act ("$p flashes into existance.", explosive_area->people, obj, NULL, 
	   TO_CHAR);
    }

  if (IS_SET (obj->general_flags, GEN_DARKS_ROOM))
    SET_BIT (pRoomIndex->room_flags, ROOM_DARK);

    obj->next_content		= pRoomIndex->contents;
    pRoomIndex->contents	= obj;
    obj->in_room		= pRoomIndex;
    obj->carried_by		= NULL;
    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
  if (obj->in_room || obj->destination || obj->in_obj || obj->carried_by)
    {
      sprintf (log_buf, "obj_to_obj: %s could not go to %s because it is "
	       "somewhere.", obj->name, obj->name);
      sbug (log_buf);
      bug_object_state (obj);
      return;
    }
  obj->next_content		= NULL;
  obj_to->contains		= obj;
  obj->in_obj                   = obj_to;
  return;
}

void obj_from_obj (OBJ_DATA *obj)
{
  obj->in_obj->contains = NULL;
  obj->in_obj = NULL;
}

void scatter_obj (OBJ_DATA *obj)
{
  sh_int x, y;
  int count;
  LEVEL_DATA *a_level;
/*  char buf[MAX_STRING_LENGTH]; */
  ROOM_DATA *temp_room = NULL;

  if (obj->in_room || obj->destination || obj->in_obj || obj->carried_by)
    {
      sprintf (log_buf, "%s could not be recycled because it still is "
	       "somewhere.", obj->name);
      sbug (log_buf);
      bug_object_state (obj);
      return;
    }
  if ((obj->item_type == ITEM_AMMO) && !expand_event && !fBootDb)
    temp_room = ammo_repop[number_range(0, 2)];
  else
    {
      x = number_range (0, the_city->x_size - 1);
      y = number_range (0, the_city->y_size - 1);
      a_level = the_city;
    }
  if (!temp_room)
    temp_room = index_room (a_level->rooms_on_level, x, y);
  obj_to_room (obj, temp_room);
  if (temp_room->people)
    {
      act ("$p glides down to earth on a small parachute to land at "
	   "your feet.", temp_room->people, obj, NULL, TO_ROOM);
      act ("$p glides down to earth on a small parachute to land at "
	   "your feet.", temp_room->people, obj, NULL, TO_CHAR);
    }
}

/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj, int perm_extract )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;
    OBJ_DATA *new_obj;

    if (obj->valid != VALID_VALUE)
      {
	sprintf (log_buf, "Extract object: the object was not set to valid "
		 "(%s).", obj->name);
	sbug (log_buf);
	return;
      }

    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( obj->carried_by != NULL )
	obj_from_char( obj );

    if (!perm_extract)
      {
	int count;

	new_obj = create_object (obj->pIndexData, 0);
	scatter_obj (new_obj);
      }

    if (obj->interior)
      {
	CHAR_DATA *the_dead, *dead_next;
	OBJ_DATA *extra_obj, *extra_next;

	log_string ("Taking out the interior.\n\r");
	for (the_dead = obj->interior->people; the_dead; the_dead = dead_next)
	  {
	    dead_next = the_dead->next_in_room;
	    the_dead->last_hit_by = obj->owner;
	    char_death (the_dead);
	  }
	for (extra_obj = obj->interior->contents; extra_obj;
	     extra_obj = extra_next)
	  {
	    extra_next = extra_obj->next_content;
	    extract_obj (extra_obj, extra_obj->extract_me);
	  }
	free_mem (obj->interior, sizeof (ROOM_DATA));
	obj->interior = NULL;
      }

    if (obj->contains)
      extract_obj(obj->contains, obj->contains->extract_me);

    if ( object_list == obj )
    {
	object_list = obj->next;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = object_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == obj )
	    {
		prev->next = obj->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
	    return;
	}
    }

    obj->valid = 0;
    free_string( obj->name        );
    free_string( obj->description );
    free_string( obj->short_descr );
    if (obj->explode_desc)
      free_string( obj->explode_desc);
    --obj->pIndexData->count;
    obj->next	= obj_free;
    obj_free	= obj;
    return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
  CHAR_DATA *wch;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  
  if ( ch->in_room == NULL )
    {
      bug( "Extract_char: NULL room.", 0 );
      *((char*)NULL) = 'b';
      return;
    }
  stop_manning (ch);
  REMOVE_BIT (ch->temp_flags, IN_TANK);
  ch->fighting = NULL;
    
  char_from_room( ch );

  for ( wch = char_list; wch != NULL; wch = wch->next )
    {
      if (wch->last_hit_by == ch )
	wch->last_hit_by = NULL;	
      if (wch->fighting == ch )
	wch->fighting = NULL;
      if (wch->chasing == ch )
	{
	  wch->chasing = NULL;
	  wch->ld_behavior = BEHAVIOR_PILLBOX;
	}
      if (wch->owner == ch)
	wch->owner = NULL;
    }

  if (!fPull)
    {
      char_to_room (ch, safe_area);
      if (!IS_NPC(ch))
      {
	do_look (ch, "auto");
	act ("A mangled and bloody corpse arrives in a box carried by several "
	     "orderlies, \n\rwho dump the unsightly remains into a large "
	     "machine with the words\n\rREGENERATION UNIT inscribed on the "
	     "side.  Several seconds later\n\ra door on the side opens and "
	     "$n steps out grinning from ear to ear.", ch, NULL, NULL, 
	     TO_ROOM);
      }
      return;
    }
  
  for ( wch = char_list; wch != NULL; wch = wch->next )
    {
      if ( wch->reply == ch )
	wch->reply = NULL;
    }

  if ( ch == char_list )
    {
      char_list = ch->next;
    }
  else
    {
      CHAR_DATA *prev;
      
      for ( prev = char_list; prev != NULL; prev = prev->next )
	{
	  if ( prev->next == ch )
	    {
	      prev->next = ch->next;
	      if (ch == next_violence)
		next_violence = prev->next;
	      break;
	    }
	}
      
      if ( prev == NULL )
	{
	  bug( "Extract_char: char not found.", 0 );
	  return;
	}
    }
  if ( ch->desc )
    ch->desc->character = NULL;
  ch->valid = 0;
  free_char( ch );
  return;
}



/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    if ( !str_cmp( arg, "self" ) )
	return ch;
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( !can_see( ch, rch ) || !is_name( arg, rch->names ) )
	    continue;
	if ( ++count == number )
	    return rch;
    }

    return NULL;
}




/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if (ch && (( wch = get_char_room (ch, argument)) != NULL))
      return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( wch->in_room == NULL || (ch && !can_see( ch, wch )) 
	||   !is_name( arg, wch->names ) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    return NULL;
}

CHAR_DATA *char_file_active (char *argument )
{
  CHAR_DATA *wch;
  char name_cap1[MAX_INPUT_LENGTH], *name_cap2;
  sh_int count;
  
  for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
      name_cap2 = capitalize(argument);
      for (count = 0; name_cap2[count]; count++)
	name_cap1[count] = name_cap2[count];
      name_cap1[count] = 0;
      if (!strcmp (name_cap1, capitalize (wch->names)))
	return wch;
    }
  
  return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if (is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
      if (obj->carried_by != ch)
	sbug ("get_obj_carry: carried_by not ch");
      if ( obj->wear_loc == WEAR_NONE
	   &&   is_name( arg, obj->name ) )
	{
	  if ( ++count == number )
	    return obj;
	}
    }

    return NULL;
}


/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
      if (obj->carried_by != ch)
	sbug ("get_obj_carry: get_obj_wear not ch");
      if ( obj->wear_loc != WEAR_NONE
	   &&   is_name( arg, obj->name ) )
	{
	  if ( ++count == number )
	    return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );
    if ( obj != NULL )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument ) ) != NULL )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
	return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    if (number == 1)
      {
	for ( obj = object_list; obj != NULL; obj = obj->next )
	  {
	    if ( is_name_exact( arg, obj->name ))
	      {
		if ( ++count == number )
		  return obj;
	      }
	  }
	count = 0;
      }
    for ( obj = object_list; obj != NULL; obj = obj->next )
      {
	if ( is_name( arg, obj->name ))
	  {
	    if ( ++count == number )
	      return obj;
	  }
      }
    return NULL;
}


/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
/*    int number;
 
    if ( obj->item_type == ITEM_CONTAINER)
        number = 0;
    else
        number = 1;
 
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );
 
    return number; */
  return 1;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;

    weight = obj->weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
	weight += get_obj_weight( obj );

    return weight;
}



/*
 * True if room is private.
 */
bool room_is_private( ROOM_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count;

    count = 0;
    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
	count++;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return TRUE;
    
    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_DATA *pRoomIndex )
{
    if (IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
    &&  !IS_IMMORTAL(ch))
	return FALSE;

    return TRUE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
  OBJ_DATA *prot_obj;

/* RT changed so that WIZ_INVIS has levels */
    if ((ch == victim) ||
	(IS_NPC(ch) && ((ch->in_room->level) ||
			(ch->ld_behavior == BEHAVIOR_GUARD))))
	return TRUE;

    if ( !IS_NPC(victim)
    &&   IS_SET(victim->act, PLR_WIZINVIS)
    &&   get_trust( ch ) < victim->invis_level )
	return FALSE;

    if ( (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT)) 
    ||   (IS_NPC(ch) && IS_IMMORTAL(ch)))
	return TRUE;

    if ( IS_SET(ch->affected_by, AFF_BLIND))
      return FALSE;

    if ((ch->in_room && IS_SET (ch->in_room->room_flags, ROOM_DARK)) ||
	(victim->in_room && IS_SET (victim->in_room->room_flags, ROOM_DARK)))
      if (((prot_obj = get_eq_char (ch, WEAR_HEAD)) == NULL) ||
	  !IS_SET (prot_obj->general_flags, GEN_SEE_IN_DARK))
	return FALSE;

    return TRUE;
}

int max_sight (CHAR_DATA *ch)
{
  return 4;
}

bool can_see_linear_char ( CHAR_DATA *ch, CHAR_DATA *victim )
{
  sh_int diff_x, diff_y, direction, count, pos_neg;
  char report_buf[MAX_INPUT_LENGTH];
  ROOM_DATA *temp_room;

  if (can_see (ch, victim))
    {
      if (ch->in_room->level != victim->in_room->level)
	return FALSE;
      if (ch->in_room == victim->in_room)
	return TRUE;
      if ((ch->in_room->level < 0) || (ch->in_room->x < 0) || 
	  (ch->in_room->y < 0))
	return FALSE;
      if (ch->in_room->x == victim->in_room->x)
	{
	  diff_x = 0;
	  diff_y = ch->in_room->y - victim->in_room->y;
	  if ((diff_y > max_sight (ch)) || (diff_y < -max_sight (ch)))
	    return FALSE;
	  else
	    if (diff_y > 0)
	      direction = 2;
	    else
	      direction = 0;
	}
      else
	if (ch->in_room->y == victim->in_room->y)
	  {
	    diff_x = ch->in_room->x - victim->in_room->x;
	    diff_y = 0;
	    if ((diff_x > max_sight (ch)) || (diff_x < -max_sight (ch)))
	      return FALSE;
	    else
	      if (diff_x > 0)
		direction = 3;
	      else
		direction = 1;
	  }
	else
	  return FALSE;
      pos_neg = (diff_x > 0) ? -1 : ((diff_x < 0) ? 1 : 0);
      if (!pos_neg)
	pos_neg = (diff_y > 0) ? -1 : ((diff_y < 0) ? 1 : 0);
      for (count = 0; count != (diff_x ? -diff_x : -diff_y); count += pos_neg)
	{
	  temp_room = index_room (ch->in_room, (diff_x ? count : 0),
				  (diff_y ? count : 0));
	  if (temp_room->exit[direction])
	    return FALSE;
	}
      return TRUE;
    }

    return FALSE;
}

bool can_see_linear_room( CHAR_DATA *ch, ROOM_DATA *a_room )
{
  sh_int diff_x, diff_y, direction, count, pos_neg;
  char report_buf[MAX_INPUT_LENGTH];
  ROOM_DATA *temp_room;

  if (ch->in_room->level != a_room->level)
    return FALSE;
  if (ch->in_room == a_room)
    return TRUE;
  if (ch->in_room->x == a_room->x)
    {
      diff_x = 0;
      diff_y = ch->in_room->y - a_room->y;
      if ((diff_y > max_sight (ch)) || (diff_y < -max_sight (ch)))
	return FALSE;
      else
	if (diff_y > 0)
	  direction = 2;
	else
	  direction = 0;
    }
  else
    if (ch->in_room->y == a_room->y)
      {
	diff_x = ch->in_room->x - a_room->x;
	diff_y = 0;
	if ((diff_x > max_sight (ch)) || (diff_x < -max_sight (ch)))
	  return FALSE;
	else
	  if (diff_x > 0)
	    direction = 3;
	  else
	    direction = 1;
      }
    else
      return FALSE;
  pos_neg = (diff_x > 0) ? -1 : ((diff_x < 0) ? 1 : 0);
  if (!pos_neg)
    pos_neg = (diff_y > 0) ? -1 : ((diff_y < 0) ? 1 : 0);
  for (count = 0; count != (diff_x ? -diff_x : -diff_y); count += pos_neg)
    {
      temp_room = index_room (ch->in_room, (diff_x ? count : 0),
			      (diff_y ? count : 0));
      if (temp_room->exit[direction])
	return FALSE;
    }
  return TRUE;
}

bool rooms_linear_with_no_walls( ROOM_DATA *room1, ROOM_DATA *room2 )
{
  sh_int diff_x, diff_y, direction, count, pos_neg;
  char report_buf[MAX_INPUT_LENGTH];
  ROOM_DATA *temp_room;

  if (room1->level != room2->level)
    return FALSE;
  if (room1 == room2)
    return TRUE;
  if (room1->x == room2->x)
    {
      diff_x = 0;
      diff_y = room1->y - room2->y;
      if (diff_y > 0)
	direction = 2;
      else
	direction = 0;
    }
  else
    if (room1->y == room2->y)
      {
	diff_x = room1->x - room2->x;
	diff_y = 0;
	if (diff_x > 0)
	  direction = 3;
	else
	  direction = 1;
      }
    else
      return FALSE;
  pos_neg = (diff_x > 0) ? -1 : ((diff_x < 0) ? 1 : 0);
  if (!pos_neg)
    pos_neg = (diff_y > 0) ? -1 : ((diff_y < 0) ? 1 : 0);
  for (count = 0; count != (diff_x ? -diff_x : -diff_y); count += pos_neg)
    {
      temp_room = index_room (room1, (diff_x ? count : 0),
			      (diff_y ? count : 0));
      if (temp_room->exit[direction])
	return FALSE;
    }
  return TRUE;
}

/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA *obj )
{
  sprintf (log_buf, "item type is %d.", obj->item_type);
  log_string (log_buf);
  switch ( obj->item_type )
    {
    case ITEM_MISC	         : return "misc";
    case ITEM_WEAPON             : return "weapon";
    case ITEM_ARMOR              : return "armor";
    case ITEM_EXPLOSIVE          : return "explosive";
    case ITEM_AMMO	         : return "ammo";
    case ITEM_TEAM_VEHICLE	 : return "team vehicle";
    case ITEM_TEAM_ENTRANCE	 : return "team entrance";
    }

    bug( "Item_type_name: unknown type %d.", obj->item_type );
    return "(unknown)";
}


/* return ascii name of an act vector */
char *act_bit_name( int act_flags )
{
    static char buf[512];

    buf[0] = '\0';

    if (act_flags & PLR_AGGRO_ALL )   strcat(buf, " agg_all");
    if (act_flags & PLR_HOLYLIGHT	) strcat(buf, " holy_light");
    if (act_flags & PLR_WIZINVIS	) strcat(buf, " wizinvis");
    if (act_flags & PLR_FREEZE	) strcat(buf, " frozen");
    if (act_flags & PLR_FREEZE	) strcat(buf, " noleader");
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *comm_bit_name(int comm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (comm_flags & COMM_QUIET		) strcat(buf, " quiet");
    if (comm_flags & COMM_NOWIZ		) strcat(buf, " no_wiz");
    if (comm_flags & COMM_COMPACT	) strcat(buf, " compact");
    if (comm_flags & COMM_BRIEF		) strcat(buf, " brief");
    if (comm_flags & COMM_PROMPT	) strcat(buf, " prompt");
    if (comm_flags & COMM_COMBINE	) strcat(buf, " combine");
    if (comm_flags & COMM_NOEMOTE	) strcat(buf, " no_emote");
    if (comm_flags & COMM_NOSHOUT	) strcat(buf, " no_shout");
    if (comm_flags & COMM_NOTELL	) strcat(buf, " no_tell");
    if (comm_flags & COMM_NOCHANNELS	) strcat(buf, " no_channels");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}


char *wear_bit_name(int wear_flags)
{
    static char buf[512];

    buf [0] = '\0';
    if (wear_flags & ITEM_TAKE		) strcat(buf, " take");
    if (wear_flags & ITEM_WEAR_FINGER	) strcat(buf, " finger");
    if (wear_flags & ITEM_WEAR_NECK	) strcat(buf, " neck");
    if (wear_flags & ITEM_WEAR_BODY	) strcat(buf, " torso");
    if (wear_flags & ITEM_WEAR_HEAD	) strcat(buf, " head");
    if (wear_flags & ITEM_WEAR_LEGS	) strcat(buf, " legs");
    if (wear_flags & ITEM_WEAR_FEET	) strcat(buf, " feet");
    if (wear_flags & ITEM_WEAR_HANDS	) strcat(buf, " hands");
    if (wear_flags & ITEM_WEAR_ARMS	) strcat(buf, " arms");
    if (wear_flags & ITEM_WEAR_SHIELD	) strcat(buf, " shield");
    if (wear_flags & ITEM_WEAR_ABOUT	) strcat(buf, " body");
    if (wear_flags & ITEM_WEAR_WAIST	) strcat(buf, " waist");
    if (wear_flags & ITEM_WEAR_WRIST	) strcat(buf, " wrist");
    if (wear_flags & ITEM_WIELD		) strcat(buf, " wield");
    if (wear_flags & ITEM_HOLD		) strcat(buf, " hold");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

int rem_from_top (TOP_DATA *top_array, CHAR_DATA *ch)
{
  int count;
  int count2;
  int ret_val = 0;

  for (count = 0; count < NUM_TOP; count++)
    if (!str_cmp (top_array[count].name, ch->names))
      {
	ret_val = 1;
	free_string(top_array[count].name);
	for (count2 = count+1; (count2 < NUM_TOP) && 
	       (top_array[count2].name[0] != '<'); count2++)
	  top_array[count2-1] = top_array[count2];
	top_array[count2-1].name = str_dup ("<empty>");
	top_array[count2-1].kills = 0;
	count--;
      }
  return ret_val;
}

int better_than (CHAR_DATA *ch, TOP_DATA *top_array, int num)
{
  if (top_array == top_players_kills)
    return (ch->kills > top_array[num].kills);
  else
    return 0;
}

void add_to_top (TOP_DATA *top_array, CHAR_DATA *ch, int num)
{
  int count;
  TOP_DATA temp, temp2;

  temp = top_array[num];
  top_array[num].name = str_dup (ch->names);
  top_array[num].kills = ch->kills;

  for (count = num+1; count < NUM_TOP; count++)
    {
      temp2 = top_array[count];
      top_array[count] = temp;
      temp = temp2;
    }
}

void show_top_list (CHAR_DATA *ch, TOP_DATA *top_array)
{
  int count;
  char buf[MAX_STRING_LENGTH];

  send_to_char ("`!TOP`` `1Ground`@ZERO`` `!PLAYERS``\n\r", ch);
  for (count = 0; count < NUM_TOP; count++)
    {
      sprintf (buf, "[K:`!%d``] `1%s``\n\r", 
	       top_array[count].kills, top_array[count].name);
      send_to_char (buf, ch);
    }
}

void insert_top (CHAR_DATA *ch, TOP_DATA *top_array)
{
  int old_member, count;
  FILE *fp;

  old_member = rem_from_top (top_array, ch);
  for (count = NUM_TOP - 2; count >= 0; count--)
    {
      if (!better_than (ch, top_array, count))
	break;
    }
  add_to_top (top_array, ch, count + 1);
  if (!old_member)
    send_to_char ("Your name has been added to the top player list!!\n\r", ch);
  if ((fp = fopen (TOP_FILE, "w")) == NULL)
    return;
  fprintf (fp, "#TOP\n");
  fprintf (fp, "%d %s~\n",
	   top_players_kills[count].kills,
	     top_players_kills[count].name);
  fprintf (fp, "\n#$\n");
  fclose(fp);
}

void top_stuff (CHAR_DATA *ch)
{
  if (!IS_IMMORTAL (ch) && !IS_NPC (ch))
    if (ch->pcdata->account && ch->pcdata->account[0])
      if (better_than (ch, top_players_kills, NUM_TOP-1))
	insert_top (ch, top_players_kills);
}
