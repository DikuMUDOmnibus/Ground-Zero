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
 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif



/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];



/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest ) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );
void    set_default_pcdata args ((CHAR_DATA *ch));
void    set_default_colors args ((CHAR_DATA *ch));
void    set_char_defaults  args ((CHAR_DATA *ch));
PC_DATA *alloc_pcdata      args(());
CHAR_DATA *alloc_char      args(());


void do_save_all (CHAR_DATA *ch, char *argument)
{
  FILE *fp;
  CHAR_DATA *chars_to_save;
  OBJ_DATA *objs_to_save;
  char buf[MAX_STRING_LENGTH];
  int count_objs = 0, count_chars = 0;
  int count;

  /* form =
            random number seed
	    max on
	    tick counter
	    expansions
            !name1 kills killed rep x y level hp
            vnum1 hpc1 hps1 vnum2 hpc2 hps2 . . . vnumN hpcN hpsN 0
            !name2 kills killed rep x y level hp
            vnum1 hpc1 hps1 vnum2 hpc2 hps2 . . . vnumN hpcN hpsN 0
	    .
	    .
            !nameN kills killed rep x y level hp
            vnum1 hpc1 hps1 vnum2 hpc2 hps2 . . . vnumN hpcN hpsN 0
	    ~
	    vnum1 x y level hpc hps special#
	    vnum2 x y level hpc hps special#
	    .
	    .
	    vnum3 x y level hpc hps special#
	    0
  special# is zero in general case, but can be assigned an arbitrary value to
  be checked when loaded in for cases such as a mine burried under the ground,
  etc that requires special action by the game
  */

  if (ch)
    send_to_char ("Saving EVERYTHING . . .\n\r", ch);
  if ((fp = fopen ("../boot/fullsave.txt", "w")) == NULL)
    {
      if (ch)
	send_to_char ("Error openning file fullsave.c.", ch);
      return;
    }
  fprintf (fp, "%d\n%d\n%d\n%d\n", game_seed, max_on, tick_counter,
	   expansions);
  for (chars_to_save = char_list; chars_to_save; 
       chars_to_save = chars_to_save->next)
    {
      /* don't save the imms cuz they come back untrusted which puts them
         in a random room when they are kicked out of their god room for
	 being ld and untrusted */
      if (IS_NPC (chars_to_save) || IS_IMMORTAL (chars_to_save))
	continue;
      count_chars++;
      fprintf (fp, "!%s\n%d %d %d %d %d %d\n", chars_to_save->names, 
	       chars_to_save->kills, chars_to_save->deaths,
	       chars_to_save->in_room->x, 
	       chars_to_save->in_room->y, 
	       chars_to_save->in_room->level, 
	       chars_to_save->hit);
      for (objs_to_save = chars_to_save->carrying; objs_to_save; 
	   objs_to_save = objs_to_save->next_content)
	{
	  count_objs++;
	  if (!objs_to_save->pIndexData->number_to_put_in)
	    continue;
	  fprintf (fp, "%d %d %d ", objs_to_save->pIndexData->vnum,
		   objs_to_save->hp_char, objs_to_save->hp_struct);
	  if (objs_to_save->contains)
	    {
	      count_objs++;
	      fprintf (fp, "%d %d %d ", 
		       objs_to_save->contains->pIndexData->vnum,
		       objs_to_save->contains->hp_char, 
		       objs_to_save->contains->hp_struct);
	    }
	}
      fprintf (fp, "0\n");
    }
  fprintf (fp, "~\n");
  for (objs_to_save = object_list; objs_to_save; 
       objs_to_save = objs_to_save->next)
    {
      if (objs_to_save->carried_by)
	continue;
      if (!objs_to_save->pIndexData->number_to_put_in)
	continue;
      if (objs_to_save->in_room)
	{
	  fprintf (fp, "%d %d %d %d %d %d", objs_to_save->pIndexData->vnum,
		   objs_to_save->in_room->x, objs_to_save->in_room->y,
		   objs_to_save->in_room->level, objs_to_save->hp_char,
		   objs_to_save->hp_struct);
	  count_objs++;
	}
      else
	if (objs_to_save->destination)
	  {
	    fprintf (fp, "%d %d %d %d %d %d", objs_to_save->pIndexData->vnum,
		     objs_to_save->destination->x,
		     objs_to_save->destination->y,
		     objs_to_save->destination->level, objs_to_save->hp_char,
		     objs_to_save->hp_struct);
	    count_objs++;
	  }
	else
	  {
	    /* otherwise it is ammo in a weapon and should have been gotten 
	       when characters were saved */
	    continue;
	  }
      if (IS_SET (objs_to_save->general_flags, GEN_CAN_BURY) &&
	  (objs_to_save->destination) && 
	  (objs_to_save->destination->mine == objs_to_save))
	fprintf (fp, " 1\n");
      else
	fprintf (fp, " 0\n");
    }
  fprintf (fp, "0\n");
  fclose (fp);
  sprintf (buf, "%%%d objects and %d characters were saved.\n\r", count_objs,
	   count_chars);
  do_immtalk (NULL, buf);
  log_string (&(buf[1]));
  if (ch)
    send_to_char ("Done.\n\r", ch);
}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    FILE *fp;

    if ( IS_NPC(ch) )
	return;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    /* create god log */
    if (ch->trust)
    {
	fclose(fpReserve);
	sprintf(strsave, "%sPS%s",GOD_DIR, capitalize(ch->names));
	if ((fp = fopen(strsave,"w")) == NULL)
	{
	    bug("Save_char_obj: fopen",0);
	    perror(strsave);
 	}

	fprintf(fp,"Trust %2d  %s%s\n",
	   get_trust(ch), ch->names, ch->pcdata->title_line);
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
    }

    fclose( fpReserve );
    sprintf( strsave, "%sPS%s", PLAYER_DIR, capitalize( ch->names ) );
    if ( ( fp = fopen( PLAYER_TEMP, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	fprintf (fp, "#END\n");
    }
    fclose( fp );
    /* move the file */
    sprintf(buf,"mv %s %s",PLAYER_TEMP,strsave);
    system(buf);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn, gn;

    fprintf( fp, "#PLAYER\n");

    fprintf( fp, "Name %s~\n",	ch->names		);
    if (ch->short_descript && ch->short_descript[0])
      	fprintf( fp, "ShD  %s~\n",	ch->short_descript);
    fprintf( fp, "Sex  %d\n",	ch->sex			);
    fprintf (fp, "nhp_solo %d\n", ch->pcdata->solo_hit);
    if (ch->kills)
      fprintf (fp, "Kills2 %d\n", ch->kills);
    if (ch->deaths)
      fprintf (fp, "Killed2 %d\n", ch->deaths);
    fprintf( fp, "Plyd %d\n",
	ch->played + (int) (current_time - ch->logon)	);
    fprintf( fp, "Note %d\n",		ch->last_note	);
    fprintf( fp, "Scro %d\n", 	ch->lines		);
    if (ch->act != 0)
	fprintf( fp, "Act  %d\n",   ch->act		);
    if (ch->invis_level != 0)
	fprintf( fp, "Invi %d\n", 	ch->invis_level	);
    fprintf( fp, "Pass %s~\n",	ch->pcdata->password);
    if (ch->kill_msg)
      fprintf( fp, "KillMsg %s~\n", ch->kill_msg);
    if (ch->pcdata->account)
      fprintf (fp, "Account %s~\n", ch->pcdata->account);
    if (ch->pcdata->poofin_msg && ch->pcdata->poofin_msg[0])
      fprintf( fp, "Bin  %s~\n",	ch->pcdata->poofin_msg);
    if (ch->pcdata->poofout_msg && ch->pcdata->poofout_msg[0])
      fprintf( fp, "Bout %s~\n",	ch->pcdata->poofout_msg);
    fprintf( fp, "Titl %s~\n",	ch->pcdata->title_line	);

    fprintf( fp, "Color %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
    	       ch->color,
               ch->pcdata->color_hp,
               ch->pcdata->color_desc,
               ch->pcdata->color_obj,
               ch->pcdata->color_action,
               ch->pcdata->color_combat_o,
               ch->pcdata->color_combat_condition_s,
               ch->pcdata->color_combat_condition_o,
               ch->pcdata->color_xy,
               ch->pcdata->color_wizi,
               ch->pcdata->color_level,
               ch->pcdata->color_exits,
               ch->pcdata->color_say,
               ch->pcdata->color_tell,
               ch->pcdata->color_reply);
    fprintf (fp, "End\n\n");
}

CHAR_DATA *alloc_char ()
{
  CHAR_DATA *temp;

  if ( char_free == NULL )
    return alloc_perm( sizeof(CHAR_DATA) );
  else
    {
      temp = char_free;
      char_free = char_free->next;
      return temp;
    }
}

PC_DATA *alloc_pcdata ()
{
  PC_DATA *temp;
  if ( pcdata_free == NULL )
    return alloc_perm( sizeof(PC_DATA) );
  else
    {
      temp = pcdata_free;
      pcdata_free = pcdata_free->next;
      return temp;
    }
}

void set_char_defaults (CHAR_DATA *ch)
{
  ch->move_delay                        = 0;
  ch->where_start                       = NULL;
  ch->valid                             = VALID_VALUE;
  ch->ld_behavior                       = 0;
  ch->affected_by			= 0;
  ch->temp_flags                        = 0;
  ch->act				= 0;
  ch->comm				= COMM_COMBINE | COMM_PROMPT;
  ch->invis_level			= 0;
  ch->trust				= 0;
  ch->armor                             = 0;
  ch->last_fight			= 0; 
}

void set_default_colors (CHAR_DATA *ch)
{
  ch->pcdata->color_combat_condition_s= 0x1;
  ch->pcdata->color_action		= 0x9;
  ch->pcdata->color_xy		        = 0xe;
  ch->pcdata->color_wizi		= 0x4;
  ch->pcdata->color_hp		        = 0x5;
  ch->pcdata->color_combat_condition_o  = 0x8;
  ch->pcdata->color_combat_o		= 0xd;
  ch->pcdata->color_level		= 0x8;
  ch->pcdata->color_exits		= 0x9;
  ch->pcdata->color_desc		= 0xa;
  ch->pcdata->color_obj		        = 0xb;
  ch->pcdata->color_say		        = 0xc;
  ch->pcdata->color_tell		= 0x0;
  ch->pcdata->color_reply		= 0x1;
}

void set_default_pcdata (CHAR_DATA *ch)
{
  ch->pcdata->confirm_delete		= FALSE;
  ch->pcdata->password			= NULL;
  ch->pcdata->poofin_msg		= NULL;
  ch->pcdata->poofin_msg		= NULL;
  ch->pcdata->title_line		= NULL;
  ch->pcdata->account			= NULL;
  ch->pcdata->solo_hit			= HIT_POINTS_MORTAL;
}

/*
 * Load a char and inventory into a new ch structure.
 */
CHAR_DATA *load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
  static PC_DATA pcdata_zero;
  char strsave[MAX_INPUT_LENGTH];
  char buf[100];
  CHAR_DATA *ch;
  FILE *fp;
  bool found;
  int stat;
  
  ch = alloc_char ();
  clear_char(ch);
  ch->pcdata = alloc_pcdata ();
  *ch->pcdata = pcdata_zero;
  
  if (d)
    {
      d->character = ch;
      ch->desc = d;
    }
  else
    ch->desc = NULL;

  ch->names = str_dup (name);
  set_char_defaults (ch);
  ch->color = 1;
  set_default_colors (ch);
  set_default_pcdata (ch);

  found = FALSE;
  fclose( fpReserve );
    
    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%sPS%s%s", PLAYER_DIR, capitalize(name),".gz");
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    sprintf( strsave, "%sPS%s", PLAYER_DIR, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	/* freed from above, will be found in pfile */
	free_string (ch->names);
	ch->names = NULL;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );

    if (found)
      return ch;
    else
      return 0;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

#define IGNORE(literal)					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    fMatch = TRUE;			\
                                    fread_to_eol (fp);                  \
				    break;				\
				}
		

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Act",		ch->act,		fread_number( fp ) );
	    KEY( "Account",	ch->pcdata->account,	fread_string(fp));

	case 'B':
	    KEY( "Bamfin",	ch->pcdata->poofin_msg,	fread_string( fp ) );
	    KEY( "Bamfout",	ch->pcdata->poofout_msg,fread_string( fp ) );
	    KEY( "Bin",		ch->pcdata->poofin_msg,	fread_string( fp ) );
	    KEY( "Bout",	ch->pcdata->poofout_msg,fread_string( fp ) );
	    break;

	case 'C':
            if ( !str_cmp( word, "Color")) 
               {
               log_string("Reading colors");
               ch->color        		    = fread_number( fp );
               ch->pcdata->color_hp 		    = fread_number( fp );
               ch->pcdata->color_desc 		    = fread_number( fp );
               ch->pcdata->color_obj 		    = fread_number( fp );
               ch->pcdata->color_action 	    = fread_number( fp );
               ch->pcdata->color_combat_o 	    = fread_number( fp );
               ch->pcdata->color_combat_condition_s = fread_number( fp );
               ch->pcdata->color_combat_condition_o = fread_number( fp );
               ch->pcdata->color_xy	 	    = fread_number( fp );
               ch->pcdata->color_wizi 		    = fread_number( fp );
               ch->pcdata->color_level  	    = fread_number( fp );
               ch->pcdata->color_exits  	    = fread_number( fp );
               ch->pcdata->color_say		    = fread_number( fp );
               ch->pcdata->color_tell		    = fread_number( fp );
               ch->pcdata->color_reply		    = fread_number( fp );
               fMatch=TRUE;
               break;
               }
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
		return;
	    break;

	case 'H':
	  IGNORE ("hp_solo");
	  /*	  KEY ("hp_solo", ch->pcdata->solo_hit, fread_number (fp));*/
	  break;

	case 'I':
	    KEY( "InvisLevel",	ch->invis_level,	fread_number( fp ) );
	    KEY( "Invi",	ch->invis_level,	fread_number( fp ) );
	    break;

        case 'K':
	    IGNORE ("Killed");
	    IGNORE ("Kills");
	    KEY ("Kills2",	ch->kills,		fread_number(fp));
	    KEY ("Killed2",	ch->deaths,		fread_number(fp));
	    KEY( "KillMsg",	ch->kill_msg,		fread_string( fp ) );
            break;

	case 'N':
	    KEY( "Name",	ch->names,		fread_string( fp ) );
	    KEY ("nhp_solo", ch->pcdata->solo_hit, fread_number (fp));
	    KEY( "Note",	ch->last_note,		fread_number( fp ) );
	    break;

	case 'P':
	    KEY( "Password",    ch->pcdata->password,	fread_string( fp ) );
	    KEY( "Pass",	ch->pcdata->password,	fread_string( fp ) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    KEY( "Plyd",	ch->played,		fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Scro",	ch->lines,		fread_number( fp ) );
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    KEY( "ShortDescr",  ch->short_descript,	fread_string( fp ) );
	    KEY( "ShD",		ch->short_descript,	fread_string( fp ) );
	    break;

	case 'T':
	    KEY( "Trust",	ch->trust,		fread_number( fp ) );
	    KEY( "Tru",		ch->trust,		fread_number( fp ) );

	    if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
	    {
	      ch->pcdata->title_line = fread_string(fp);
	      if (ch->pcdata->title_line[0] != '.' &&
		  ch->pcdata->title_line[0] != ',' &&
		  ch->pcdata->title_line[0] != '!' && 
		  ch->pcdata->title_line[0] != '?')
		{
		    sprintf( buf, " %s", ch->pcdata->title_line );
		    free_string (ch->pcdata->title_line);
		    ch->pcdata->title_line = str_dup (buf);
		}
		fMatch = TRUE;
		break;
	    }

	    break;
	}

	if ( !fMatch )
	{
	    bug( "Fread_char: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}

void save_account_list ()
{
  FILE *fp;
  ACCOUNT_DATA *tracker;

  if ((fp = fopen (ACCOUNTS_FILE, "w")) == NULL)
    return;
  fprintf (fp, "#ACCOUNTS\n");
  for (tracker = accounts_list; tracker; tracker = tracker->next)
    fprintf (fp, "%s %s\n", tracker->login, tracker->password);
  fprintf (fp, "$\n\n#$\n\n");
  fclose (fp);
}
