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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include "ground0.h"
#include "db.h"
#include "memory.h"


#if defined(unix)
extern int getrlimit(int resource, struct rlimit *rlp);
extern int setrlimit(int resource, const struct rlimit *rlp);
#endif

#if !defined(macintosh)
extern	int	_filbuf		args( (FILE *) );
#endif

void distribute_mobs ();
void scatter_objects ();
void randomize_level args( (LEVEL_DATA *a_level));
void find_level_commonality args ((LEVEL_DATA *first_level, 
				     LEVEL_DATA *second_level, int *x_size, 
				     int *y_size, int *lev1_lower_x,
				     int *lev1_lower_y, int *lev2_lower_x,
				     int *lev2_lower_y));

/*
 * Globals.
 */
OBJ_INDEX_DATA *        all_objects = NULL;
LEVEL_DATA *            the_city;
HELP_DATA *		help_first;
HELP_DATA *		help_last;

char			bug_buf		[2*MAX_INPUT_LENGTH];
CHAR_DATA *		char_list;
char                   *help_greeting1, *help_greeting2a, *help_greeting2b;
char			log_buf		[2*MAX_INPUT_LENGTH];
NOTE_DATA *		note_list = NULL;
OBJ_DATA *		object_list;
ACCOUNT_DATA	       *accounts_list;
TIME_INFO_DATA		time_info;

int 			mobile_count = 0;
int			newmobs = 0;
int			newobjs = 0;
GOD_TYPE               *god_table;
CHAR_DATA	       *enforcer;
CHAR_DATA	       *pill_box;
CHAR_DATA	       *guardian;
int			expansions = 0;
TOP_DATA	        top_players_kills[NUM_TOP];
int expand_event = 0;

/*
 * Semi-locals.
 */
bool			fBootDb;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];

struct social_type	social_table		[MAX_SOCIALS];
int						social_count		= 0;

/*
 * Local booting procedures.
*/
void    init_mm         args( ( void ) );
void	load_area	args( ( FILE *fp ) );
void	load_helps	args( ( FILE *fp ) );
void    load_obj_header args( ( FILE *fp ) );
void 	load_objects	args( ( FILE *fp ) );
void 	load_socials	args( ( FILE *fp ) );
void	load_notes	args( ( void ) );
void    load_whole_game args( ( FILE *fp ) );
void    load_gods       args( ( FILE *fp ) );
void    load_site_bans  args( ( FILE *fp, int newbie ) );
void    load_mobs       args( ( FILE *fp ) );

#if defined(unix)
/* RT max open files fix */
 
void maxfilelimit()
{
    struct rlimit r;
 
    getrlimit(RLIMIT_NOFILE, &r);
    r.rlim_cur = r.rlim_max;
    setrlimit(RLIMIT_NOFILE, &r);
}
#endif

/*
 * Big mama top level function.
 */
void boot_db( void )
{
  char buf[MAX_STRING_LENGTH];
  FILE *start_file;

#if defined(unix)
    /* open file fix */
    maxfilelimit();
#endif


  get_string_space ();
  fBootDb		= TRUE;

  /* get a start file if there is one */
  start_file = fopen ("../boot/startfile.txt", "r");

    /*
     * Set time.
     */
    {
	long lhour, lday, lmonth;

	lhour		= (current_time - 650336715)
			/ (PULSE_TICK / PULSE_PER_SECOND);
	time_info.hour	= lhour  % 24;
	lday		= lhour  / 24;
	time_info.day	= lday   % 35;
	lmonth		= lday   / 35;
	time_info.month	= lmonth % 17;
	time_info.year	= lmonth / 17;

	sprintf (buf, "%s%s", BOOT_DIR, "boot.seed");
	if ((fpArea = fopen (buf, "r")) == NULL)
	  {
	    bug ("failed to open boot.seed", 0);
	    exit (STATUS_ERROR);
	  }
	else
	  {
	    boot_seed = fread_number (fpArea);
	    fclose (fpArea);
	  }

	sprintf (buf, "boot seed = %d", boot_seed);
	log_string (buf);

	/* note that we srandom game seed right after this */
	srandom (boot_seed);

	if (start_file)
	  {
	    int count;
	    fscanf (start_file, "%d", &game_seed);
	    sprintf (buf, "previous game seed %d\n\r", game_seed);
	    log_string (buf);
	    fscanf (start_file, "%d", &max_on);
	    sprintf (buf, "previous max chars %d\n\r", max_on);
	    log_string (buf);
	    fscanf (start_file, "%d", &tick_counter);
	    sprintf (buf, "previous tick counter %d\n\r", tick_counter);
	    log_string (buf);
	    fscanf (start_file, "%d", &expansions);
	    sprintf (buf, "previous expansions %d\n\r", expansions);
	    log_string (buf);
	  }	    
	else
	  {
	    game_seed = current_time;
	    sprintf (buf, "new seed %d\n\r", game_seed);
	    log_string (buf);
	  }
	srandom (game_seed);
    }

    log_string ("Reading boot files.");
    /*
     * Read in all the area files.
     */
    {
	FILE *fpList;

	sprintf (buf, "%s%s", BOOT_DIR, BOOT_LIST);
	if ( ( fpList = fopen( buf, "r" ) ) == NULL )
	{
	    perror( buf );
	    exit (STATUS_ERROR);
	}

	for ( ; ; )
	{
	    strcpy( strArea, fread_word( fpList ) );
	    if ( strArea[0] == '$' )
		break;

	    if ( strArea[0] == '-' )
	    {
		fpArea = stdin;
	    }
	    else
	    {
	      sprintf (buf, "%s%s", BOOT_DIR, strArea);
		if ( ( fpArea = fopen( buf, "r" ) ) == NULL )
		{
		    perror( buf );
		    exit (STATUS_ERROR);
		}
	    }

	    for ( ; ; )
	    {
		char *word;

		if ( fread_letter( fpArea ) != '#' )
		{
		    bug( "Boot_db: # not found.", 0 );
		    exit (STATUS_ERROR);
		}

		word = fread_word( fpArea );

		if (word[0] == '$')
		  break;
		else
		  {
		    int err = 0;
		    switch (word[0])
		      {
		      default:
			err = 1;
		      case 'O':
			if (!str_cmp (word, "OBJ_HEADER"))
			  load_obj_header (fpArea);
			else 
			  if (!str_cmp (word, "OBJECTS")) 
			    load_objects (fpArea);
			  else
			    err = 1;
			break;
		      case 'A':
			if (!str_cmp (word, "AREA")) 
			  load_area    (fpArea);
			else 
			  if (!str_cmp (word, "ACCOUNTS"))
			    load_accounts (fpArea);
			  else
			    err = 1;
			break;
		      case 'H':
			if (!str_cmp (word, "HELPS")) 
			  load_helps   (fpArea);
			else
			  err = 1;
			break;
		      case 'S':
			if (!str_cmp(word, "SOCIALS")) 
			  load_socials (fpArea);
			else
			  err = 1;
			break;
		      case 'G':
			if (!str_cmp(word, "GODS")) 
			  load_gods (fpArea);
			else
			  err = 1;
			break;
		      case 'B':
			if (!str_cmp(word, "BANS")) 
			  load_site_bans (fpArea, 0);
			else
			  err = 1;
			break;
		      case 'N':
			if (!str_cmp(word, "NBANS")) 
			  load_site_bans (fpArea, 1);
			else
			  err = 1;
			break;
		      case 'M':
			if (!str_cmp(word, "MOBS")) 
			  load_mobs (fpArea);
			else err = 1;
			break;
		      case 'T':
			if (!str_cmp(word, "TOP"))
			  load_top (fpArea);
			else err = 1;
			break;
		      }
		    if (err)
		      {
			bug( "Boot_db: bad section name.", 0 );
			exit (STATUS_ERROR);
		      }
		  }
	      }

	    if ( fpArea != stdin )
		fclose( fpArea );
	    fpArea = NULL;
	}
	log_string ("All files read.");
	fclose( fpList );
    }

    srandom (boot_seed);
    set_eq_numbers ();
    srandom (game_seed);

    {
      int x = number_range (0, the_city->x_size - 1);
      int y = number_range (0, the_city->y_size - 1);

      ammo_repop[0] = index_room (the_city->rooms_on_level, x, y);
      ammo_repop[2] = index_room (the_city->rooms_on_level, 
				  the_city->x_size - 1 - x,
				  the_city->y_size - 1 - y);
      ammo_repop[1] = index_room (the_city->rooms_on_level,
				  the_city->x_size / 2, the_city->y_size / 2);
      log_string ("ammo repops at: (%d, %d), (%d, %d), (%d, %d)",
		  x,y,y,x,the_city->x_size / 2, the_city->y_size / 2);
    }
    if (!start_file)
      {
	log_string ("Distributing objects . . .");
	scatter_objects ();
	do_save_all (NULL, "");
      }
    else
      {
	log_string ("Loading from saved game . . .");
	fpArea = start_file;
	load_whole_game (fpArea);
	fclose (fpArea);
	fpArea = NULL;
	system ("mv ../boot/startfile.txt ../boot/fullsave.txt");
      }
  
    log_string ("Setting bases . . .");
    set_bases ();

    log_string ("Distributing mobs . . .");
    distribute_mobs ();
  
    /*
     * Declare db booting over.
     * Load up the notes file.
     */
    log_string ("Reading notes . . .");
    load_notes( );
    fBootDb	= FALSE;
}


void load_whole_game (FILE *fp)
{
  CHAR_DATA *inject_char;
  OBJ_DATA *inject_obj, *obj;
  LEVEL_DATA *level_finder;
  char a_file_name [MAX_STRING_LENGTH] = "!";
  int special, x, y, level, a_vnum, load_counter = 0;
  int count;

  for (count = 0; count < expansions; count++)
    expand_city();
  fscanf (fp, "%s", a_file_name);
  while (a_file_name[0] == '!')
    {
      log_string (a_file_name+1);
      inject_char = load_char_obj (NULL, &(a_file_name[1]));
      if (!inject_char)
	{
	  bug ("Character listed in startfile.txt not found.", 0);
	  log_string ("%s", &(a_file_name[1]));
	  system ("touch ../boot/shutdown.txt");
	  exit (STATUS_BOOT_ERROR);
	}
      inject_char->ld_timer = LD_TICKS;
      inject_char->next = char_list;
      char_list = inject_char;
      fscanf (fp, "%d%d%d%d%d%d\n", &(inject_char->kills),
	      &(inject_char->deaths), &x, &y, &level, &(inject_char->hit));
      if (level < 0)
	if (level == -2)
	  char_to_room (inject_char, god_general_area);
	else
	  char_to_room (inject_char, safe_area);
      else
	{
	  for (level_finder = the_city; level_finder->level_down != the_city;
	       level_finder = level_finder->level_down)
	    if (level_finder->level_number == level) break;
	  if (level_finder->level_number != level)
	    {
	      bug ("level not found", 0);
	      system ("mv ../boot/startfile.txt ..");
	      exit (STATUS_ERROR);
	    }
	  else
	    if ((x > (level_finder->x_size - 1)) || (x < 0) ||
		(y > (level_finder->y_size - 1)) || (y < 0))
	      {
		bug ("Coordinates out of range", 0);
		system ("mv ../boot/startfile.txt ..");
		exit (STATUS_ERROR);
	      }
	    else
	      char_to_room (inject_char, 
			    index_room (level_finder->rooms_on_level, x, y));
	}
      if (inject_char->trust != 10)
	inject_char->max_hit = HIT_POINTS_MORTAL;
      else
	inject_char->max_hit = HIT_POINTS_IMMORTAL;
      fscanf (fp, "%d\n", &a_vnum);
      while (a_vnum)
	{
	  inject_obj = create_object (get_obj_index (a_vnum), 0);
	  obj_to_char (inject_obj, inject_char);
	  load_counter++;
	  fscanf (fp, "%d%d", &(inject_obj->hp_char), 
		  &(inject_obj->hp_struct));
	  fscanf (fp, "%d", &a_vnum);
	}
      do_wear (inject_char, "all");
      do_load (inject_char, "");
      fscanf (fp, "\n", &a_vnum);
      fscanf (fp, "%s", a_file_name);
    }
  fscanf (fp, "%d", &a_vnum);
  while (a_vnum)
    {
      if (feof (fp))
	{
	  sprintf (log_buf, "Incorrect termination of start file.  Exiting "
		   "with status %d.", STATUS_REBOOT);
	  log_string (log_buf);
	  system ("rm ../boot/startfile.txt");
	  exit (STATUS_REBOOT);
	}
      inject_obj = create_object (get_obj_index (a_vnum), 0);
      fscanf (fp, "%d %d %d", &x, &y, &level);
      if ((x < 0) && (y < 0) && (level < 0))
	scatter_obj (inject_obj);
      else
	{
	  for (level_finder = the_city; level_finder->level_down != the_city;
	       level_finder = level_finder->level_down)
	    if (level_finder->level_number == level) break;
	  if (level_finder->level_number != level)
	    {
	      bug ("level not found", 0);
	      system ("mv ../boot/startfile.txt ..");
	      exit (STATUS_ERROR);
	    }
	  else
	    if ((x > (level_finder->x_size - 1)) || (x < 0) ||
		(y > (level_finder->y_size - 1)) || (y < 0))
	      {
		bug ("Coordinates out of range", 0);
		system ("mv ../boot/startfile.txt ..");
		exit (STATUS_ERROR);
	      }
	    else
	      obj_to_room (inject_obj, 
			   index_room (level_finder->rooms_on_level, x, y));
	}
      load_counter++;
      fscanf (fp, "%d %d %d", &inject_obj->hp_struct, &inject_obj->hp_char,
	      &special);
      switch (special)
	{
	case 1:
	  inject_obj->in_room->mine = inject_obj;
	  inject_obj->destination = inject_obj->in_room;
	  obj_from_room (inject_obj);
	  break;
	}
	
      fscanf(fp, "\n");
      fscanf (fp, "%d", &a_vnum);
    }
  /* you might want to comment this back in if you are expecting at least X number of objects to be loaded.  Then you can look at
     the saved file later if that number didn't load and see why it was so low */
  /*  if (load_counter < 500)
    {
      system ("cp ../boot/startfile.txt ..");
      bug ("0 encounterred and only %d objects loaded.  copying startfile "
	   "to ..", load_counter);
    }*/
  fscanf (fp, "\n");
  log_string ("Old game read in successfully.");
}

void set_bases ()
{
  OBJ_DATA *obj;
 
  /* RELEASE: obviously this set all the bases before I modifed the code */ 
  obj = create_object (get_obj_index (VNUM_HQ), 0);
  obj->name = str_dup ("An HQ.");
  obj->short_descr = str_dup ("a headquarters entrance");
  obj->description = str_dup ("The headquarters entrance is here.");
  obj->interior->name = str_dup ("The inside of HQ.");
  obj_to_room (obj, safe_area);
}

void scatter_objects ()
{
  OBJ_INDEX_DATA *obj_ind;
  OBJ_DATA *obj;
  int count;
  int add_obj;
  char buf[MAX_INPUT_LENGTH];

  for (obj_ind = all_objects; obj_ind->name[0] != '$'; obj_ind = &(obj_ind[1]))
    if (obj_ind->number_to_put_in > 0)
      {
	add_obj = 0;
	if (fBootDb)
	  add_obj = 1;
	else
	  /* we don't want to add vehicles every time we expand the grid */
	  if (obj_ind->item_type != ITEM_TEAM_VEHICLE)
	    add_obj = 1;
	if (add_obj)
	  for (count = 0; count < obj_ind->number_to_put_in; count++)
	    {
	      obj = create_object(obj_ind, 0);
	      scatter_obj (obj);
	    }
      }
}

void load_site_bans( FILE *fp, int newbie )
{
  char *site, *reason;
  char buf[MAX_INPUT_LENGTH];

  log_string ("Banning troublesome sites . . .");
  site = fread_string (fp);
  while (site[0] != '$')
    {
      reason = fread_string (fp);
      if (strlen (reason) + strlen (site) > MAX_INPUT_LENGTH - 3)
	{
	  bug ("Reason too long.", 0);
	  exit (STATUS_ERROR);
	}
      sprintf (buf, "%s %s %s", newbie ? "newbie" : "full", site, reason);
      do_ban (NULL, buf);
      site = fread_string (fp);
    }
}

void load_gods( FILE *fp )
{
  int num_gods, count = 0;

  log_string ("Reading the god list . . .");
  num_gods = fread_number(fp);
  num_gods++; /* blank record at the end. */
  god_table = alloc_perm (num_gods*sizeof (GOD_TYPE));
  god_table[count].rl_name = fread_string(fp);
  while (god_table[count].rl_name[0] != '$')
    {
      god_table[count].trust = fread_number(fp);
      if (god_table[count].trust >= MAX_TRUST)
	{
	  bug ("No characters of IMP trust are allowed in the god bootfile.", 
	       0);
	  exit (STATUS_ERROR);
	}
      god_table[count].game_names = fread_string(fp);
      god_table[count].password = fread_string(fp);
      god_table[count].room_name = fread_string(fp);
      god_table[count].honorary = fread_number (fp);
      count++;
      if (count == num_gods)
	{
	  bug ("More gods were listed than were declared", 0);
	  exit (STATUS_ERROR);
	}
      god_table[count].rl_name = fread_string(fp);
    }
  god_table[count].rl_name = god_table[count].game_names = 
    god_table[count].password = god_table[count].room_name = "";
  god_table[count].trust = 0;
}

void distribute_mobs()
{
  CHAR_DATA *mob;
  ROOM_DATA *room;
  char stat_buf[MAX_INPUT_LENGTH];
  int count;

  for (mob = char_list; mob; mob = mob->next)
    if (IS_NPC (mob))
      do_goto (mob, mob->where_start);

  /* RELEASE: code I wrote to destribute guardians - feel free to adapt */
#if 0
  for (count = TEAM_RED; count <= MAX_TEAM; count++)
    {
      if (!*team_table[count].team_on)
	continue;
      mob = clone_mobile (guardian);
      sprintf (stat_buf, "%s guardian", team_table[count].name);
      mob->names = str_dup (stat_buf);
      sprintf (stat_buf, "The %s GUARDIAN", team_table[count].f_color_name);
      mob->short_descript = str_dup (stat_buf);
      mob->team = count;
      char_from_room (mob);
      sprintf (log_buf, "Sending %s to %d, %d.", mob->names,
	       (the_city->x_size - 1)*team_table[count].x_mult,
	       (the_city->y_size - 1)*team_table[count].y_mult);
      log_string (log_buf);
      room = index_room (the_city->rooms_on_level,
			 (the_city->x_size - 1)*team_table[count].x_mult,
			 (the_city->y_size - 1)*team_table[count].y_mult);
      char_to_room (mob, room);
    }
#endif
}

void load_mobs( FILE *fp )
{
  CHAR_DATA *inject_mob;
  char *buf;
  int num_objects, vnum;
  int low, high;

  srandom (boot_seed);
  log_string ("Reading mobs . . .");
  buf = NULL;
  do
    {
      buf = fread_string_eol (fp);
    } while (buf[0] == ']');
  buf = fread_string (fp);
  while (buf[0] != '$')
    {
      inject_mob = alloc_char ();
      clear_char(inject_mob);
      inject_mob->names = str_dup(buf);
      if (!str_cmp (inject_mob->names, "droid"))
	pill_box = inject_mob;
      if (!str_cmp (inject_mob->names, "tank"))
	tank_mob = inject_mob;
      if (!str_cmp (inject_mob->names, "enforcer"))
	enforcer = inject_mob;
      if (!str_cmp (inject_mob->names, "guardian"))
	guardian = inject_mob;
      inject_mob->short_descript = fread_string (fp);
      set_char_defaults(inject_mob);
      inject_mob->pcdata = NULL;
      fread_word (fp);
	{
	  low = fread_number (fp);
	  if (fgetc (fp) != '-')
	    {
	      bug ("'-' expected.", 0);
	      exit (STATUS_ERROR);
	    }
	  high = fread_number (fp);
	  inject_mob->hit = inject_mob->max_hit = number_range (low, high);
	}
      fread_word (fp);
      inject_mob->move_delay = fread_number (fp);
      fread_word (fp);
      inject_mob->ld_behavior = fread_number (fp);
      fread_word (fp);
      inject_mob->act = fread_number (fp);
      fread_word (fp);
      inject_mob->sex = fread_number (fp);
      fread_word (fp);
      inject_mob->where_start = fread_string (fp);
      char_to_room (inject_mob, safe_area);
      fread_word (fp);
      while ((vnum = fread_number (fp)) != 0)
	obj_to_char (create_object (get_obj_index (vnum), 0), inject_mob);
      do_wear (inject_mob, "all");
      do_load (inject_mob, "");
      inject_mob->next = char_list;
      char_list = inject_mob;
      buf = fread_string (fp);
    }

  log_string ("Mobs have been read.");
  srandom (game_seed);
  return;
}

void load_obj_header( FILE *fp )
{
  char *buf, arg[MAX_STRING_LENGTH];
  int num_objects;

  log_string ("Reading the object header file");
  buf = NULL;
  do
    {
      buf = fread_string_eol (fp);
    } while (buf[0] == ']');
  buf = one_argument (buf, arg);
  one_argument (buf, arg);
  num_objects = atoi (arg);
  all_objects = alloc_perm (sizeof (OBJ_INDEX_DATA) * num_objects);
              /*malloc (sizeof (OBJ_INDEX_DATA) * num_objects);*/
  log_string ("Object header has been read.");
  return;
}

void load_damage_numbers (FILE *fp, OBJ_INDEX_DATA *current_obj)
{
  char *buf;
  int count;

  buf = fread_word (fp);
  if (strcmp (buf, "DAM_CH"))
    {
      log_string ("got %s instead of DAM_CH.", buf);
      bug ("DAM_CH expected.", 0);
      exit (STATUS_ERROR);
    }
  for (count = 0; count < 3; count++)
    current_obj->damage_char[count] = fread_number (fp);
  fread_to_eol (fp);
  buf = fread_word (fp);
  for (count = 0; count < 3; count++)
    current_obj->damage_structural[count] = fread_number (fp);
}

void load_gen_obj_flags (FILE *fp, OBJ_INDEX_DATA *current_obj)
{
  char *buf;
  unsigned int flags;

  buf = fread_word (fp);
  flags = current_obj->general_flags = fread_flag (fp);

  if (IS_SET (flags, GEN_BURNS_ROOM))
    load_damage_numbers (fp, current_obj);
}

void load_extract_obj_flags (FILE *fp, OBJ_INDEX_DATA *current_obj)
{
  char *buf;
  unsigned int flags;
  int count;

  buf = fread_word (fp);
  flags = current_obj->extract_flags = fread_flag (fp);

  if (flags)
    current_obj->explode_desc = fread_string (fp);

  if (IS_SET (flags, EXTRACT_EXPLODE_ON_EXTRACT))
    load_damage_numbers (fp, current_obj);

  if (IS_SET (flags, EXTRACT_BURN_ON_EXTRACT))
    {
      buf = fread_word (fp);
      current_obj->burn_time = fread_number (fp);
    }
}

void load_use_obj_flags (FILE *fp, OBJ_INDEX_DATA *current_obj)
{
  char *buf;
  unsigned int flags;

  buf = fread_word (fp);
  if (strcmp (buf, "USE"))
    {
      log_string ("got %s instead of USE.", buf);
      bug ("USE expected.", 0);
      exit (STATUS_ERROR);
    }
  flags = current_obj->usage_flags = fread_flag (fp);

  if (IS_SET (flags, USE_HEAL))
    load_damage_numbers (fp, current_obj);
}

void load_obj_item_type (FILE *fp, OBJ_INDEX_DATA *current_obj)
{
  char *buf;

  buf = fread_word (fp);
  if (strcmp (buf, "ITEM_TYPE"))
    {
      log_string ("got %s instead of item_type.", buf);
      bug ("Item_type expected.", 0);
      exit (STATUS_ERROR);
    }
  current_obj->item_type = fread_number (fp);

  buf = fread_word (fp);
  if (strcmp (buf, "WEAR"))
    {
      log_string ("got %s instead of wear.", buf);
      bug ("Wear expected.", 0);
      exit (STATUS_ERROR);
    }

  current_obj->wear_flags = fread_number (fp);

  switch (current_obj->item_type)
    {
    case ITEM_WEAPON:
      buf = fread_word (fp);
      current_obj->rounds_per_second = fread_number (fp);
      buf = fread_word (fp);
      current_obj->ammo_type = fread_number (fp);
      buf = fread_word (fp);
      current_obj->range = fread_number (fp);
      break;
    case ITEM_ARMOR:
      buf = fread_word (fp);
      current_obj->armor = fread_number (fp);
      break;
    case ITEM_EXPLOSIVE:
      buf = fread_word (fp);
      current_obj->range = fread_number (fp);
      break;
    case ITEM_AMMO:
      buf = fread_word (fp);
      current_obj->ammo_type = fread_number (fp);
      buf = fread_word (fp);
      current_obj->ammo = fread_number (fp);
      load_damage_numbers (fp, current_obj);
    case ITEM_TEAM_VEHICLE:
    case ITEM_TEAM_ENTRANCE:
    case ITEM_MISC:
      break;
    default:
      log_string ("Illegal item type %d found.", current_obj->item_type);
      exit (STATUS_ERROR);
    }
}

int get_next_char (FILE *fp)
{
  int ch = ' ';

  while (isspace (ch))
    ch = fgetc (fp);

  return ch;
}

int get_count_num (int item_type)
{
  switch (item_type)
    {
    case ITEM_WEAPON:
      return COUNT_WEAPONS;
    case ITEM_ARMOR:
      return COUNT_ARMOR;
    case ITEM_EXPLOSIVE:
      return COUNT_EXPLOSIVES;
    case ITEM_AMMO:
      return COUNT_AMMO;
    default:
      return COUNT_MISC;
    }
}

int counts_match (int *counter, int *counter2)
{
  int count;

  for (count = 0; count < NUM_COUNTS; count++)
    if (counter[count] != counter2[count])
      return 0;
  return 1;
}

void set_eq_numbers ()
{
  int current_vnum;
  int counters[NUM_COUNTS];
  int objectives[NUM_COUNTS];
  int count;
  OBJ_INDEX_DATA *current_obj;
  int top_vnum;

  for (count = 0; count < NUM_COUNTS; count++)
    {
      counters[count] = 0;
      objectives[count] = 0;
    }

  /* first we find our objective counts and set the current to the number
     that are guaranteed to be in it. */
  for (current_vnum = 1; all_objects[current_vnum - 1].name[0] != '$';
       current_vnum++)
    {
      int count_num;

      current_obj = all_objects + current_vnum - 1;
      count_num = get_count_num (current_obj->item_type);
      if (current_obj->prob_in > 0)
	{
	  objectives[count_num]++;
	  if (current_obj->prob_in == 100)
	    counters[count_num]++;
	}
    }
  top_vnum = current_vnum;

  /* we want 3/4 of them for everything but ammo/guns */
  for (count = 0; count < NUM_COUNTS; count++)
    if ((count == COUNT_WEAPONS) || (count == COUNT_AMMO))
      objectives[count] = (objectives[count] * 87) / 100;
    else
      objectives[count] = (objectives[count] * 3) / 4;

  /* now make them match */
  while (!counts_match (counters, objectives))
    {
      current_obj = all_objects + number_range (0, top_vnum - 1);
      if ((current_obj->prob_in != 100) &&
	  (number_range (1, 100) < current_obj->prob_in))
	{
	  int count_num = get_count_num (current_obj->item_type);
	  
	  if (counters[count_num] < objectives[count_num])
	    {
	      counters[count_num]++;
	      current_obj->prob_in = 100;
	    }
	}
    }

  /* everything that isn't 100% now should not go in */
  for (current_vnum = 1; all_objects[current_vnum - 1].name[0] != '$';
       current_vnum++)
    {
      current_obj = all_objects + current_vnum - 1;
      if (current_obj->prob_in != 100)
	{
	  current_obj->prob_in = 0;
	  current_obj->number_to_put_in = 0;
	}
    }
}


/*
 * Snarf some objects
 */
void load_objects( FILE *fp )
{
  char *buf;
  sh_int count2;
  OBJ_INDEX_DATA *current_obj;
  int low, high;
  int a_char;
  static int current_vnum = 1;

  srandom (boot_seed);
  if (!all_objects)
    {
      log_string ("Attempt to load objects before header.");
      exit (STATUS_ERROR);
    }

  log_string ("Reading an object file . . .");
  a_char = get_next_char (fp);
  for (;a_char != '$'; current_vnum++)
    {
      ungetc (a_char, fp);
      current_obj = all_objects + current_vnum - 1;
      current_obj->vnum = current_vnum;

      load_obj_item_type (fp, current_obj);

      load_gen_obj_flags (fp, current_obj);

      load_extract_obj_flags (fp, current_obj);

      load_use_obj_flags (fp, current_obj);

      current_obj->name = fread_string (fp);
      current_obj->short_descr = fread_string (fp);
      current_obj->description = fread_string (fp);
      
      buf = fread_word (fp);
      if ((current_obj->prob_in = fread_number (fp)) > 0)
	{
	  low = fread_number (fp);
	  if (fgetc (fp) != '-')
	    {
	      bug ("'-' expected.", 0);
	      exit (STATUS_ERROR);
	    }
	  high = fread_number (fp);
	  current_obj->number_to_put_in = number_range (low, high);
	}
      else
	current_obj->number_to_put_in = 0;

      buf = fread_word (fp);
      current_obj->hp_char = fread_number (fp);
      current_obj->hp_struct = fread_number (fp);

      a_char = get_next_char (fp);
    }
  top_obj_index = current_obj->vnum + 1;
  current_obj = all_objects + current_vnum - 1;
  current_obj->name = str_dup ("$");
  srandom (game_seed);
}

void clear_level (ROOM_DATA *rooms_on_level, int x_size, int y_size,
		  LEVEL_DATA *the_level)
{
  static ROOM_DATA room_zero;
  int current_x, current_y;

  for (current_x = 0; current_x < x_size; current_x++)
    for (current_y = 0; current_y < y_size; current_y++)
      rooms_on_level[current_x + x_size * current_y] = room_zero;
}

void set_descs_level (ROOM_DATA *rooms_on_level, int x_size, int y_size,
		      LEVEL_DATA *the_level)
{
  ROOM_DATA *temp_room;
  int current_x, current_y;
  char *the_desc[9] = 
    {
      "room description 1",
      "room description 2",
      "room description 3",
      "room description 4",
      "room description 5",
      "room description 6",
      "room description 7",
      "room description 8",
      "room description 9",
   };

  log_string ("Initializing all room_descs");
  for (current_x = 0; current_x < x_size; current_x++)
    for (current_y = 0; current_y < y_size; current_y++)
      {
	temp_room = &(rooms_on_level[current_x + x_size * current_y]);
	temp_room->level = the_level->level_number;
	temp_room->this_level = the_level;
	temp_room->people = NULL;
	temp_room->interior_of = NULL;
	temp_room->contents = NULL;
	temp_room->mine = NULL;
	temp_room->room_flags = NULL;
	
	if (current_x < x_size / 3)
	  {
	    if (current_y < y_size / 3)
	      temp_room->name = the_desc[0];
	    else
	      if (current_y >= (y_size * 2) / 3)
		temp_room->name = the_desc[1];
	      else
		temp_room->name = the_desc[2];
	  }
	else
	  if (current_x >= (x_size * 2) / 3)
	    {
	      if (current_y < y_size / 3)
		temp_room->name = the_desc[3];
	      else
		if (current_y >= (y_size * 2) / 3)
		  temp_room->name = the_desc[4];
		else
		  temp_room->name = the_desc[5];
	    }
	  else
	    {
	      if (current_y < y_size / 3)
		temp_room->name = the_desc[6];
	      else
		if (current_y >= (y_size * 2) / 3)
		  temp_room->name = the_desc[7];
		else
		  temp_room->name = the_desc[8];
	    }
	
	temp_room->x = current_x;
	temp_room->y = current_y;
	temp_room->description = the_desc;
	temp_room->mine = NULL;
      }
}

void set_walls_level (ROOM_DATA *rooms_on_level, int x_size, int y_size)
{
  int current_x, current_y;
  ROOM_DATA *temp_room;
  int room_killer;
  
  for (current_x = 0; current_x < x_size; current_x++)
    for (current_y = 0; current_y < y_size; current_y++)
      {
	temp_room = index_room (rooms_on_level, current_x, current_y);
	for (room_killer = 0; room_killer < 6; room_killer++)
	  {
	    temp_room->exit[room_killer] = 0;
	    if ((!current_x && (room_killer == DIR_WEST)) ||
		(!current_y && (room_killer == DIR_SOUTH)) ||
		((current_x == x_size - 1) && (room_killer == DIR_EAST)) ||
		((current_y == y_size - 1) && (room_killer == DIR_NORTH)) ||
		(room_killer == DIR_DOWN) || (room_killer == DIR_UP))
	      {
		SET_BIT (temp_room->exit[room_killer], EX_ISWALL);
		SET_BIT (temp_room->exit[room_killer], EX_ISNOBREAKWALL);
	      }
	  }
      }
}

/*
 * Snarf the whole city
 */
void load_area( FILE *fp )
{
  static ROOM_DATA room_zero;

  char *buf, *dirs = "neswud", *tracker;
  char number_buf [20];
  LEVEL_DATA *current_level;
  ROOM_DATA *some_room;
  sh_int x_size, y_size, num_levels, count_levels, count_extra_rooms;
  sh_int count;
  
  the_city = alloc_perm (sizeof (LEVEL_DATA));
  the_city->level_up = the_city;
  the_city->level_down = the_city;

  log_string ("Loading city.");
  do
    {
      buf = fread_string_eol (fp);
    } while (buf[0] == ']');
  buf = &(buf[2]);
  buf = one_argument (buf, number_buf);
  num_levels = atoi (number_buf);
  sprintf (log_buf, "There will be %d levels in the city.", num_levels);
  log_string (log_buf);
  
  current_level = the_city;
  for (count_levels = 0; count_levels < num_levels; count_levels++)
    {
      log_string ("Reading a level . . .");
      if (count_levels)
	{
	  current_level->level_down = alloc_perm (sizeof (LEVEL_DATA));
	  current_level->level_down->level_up = current_level;
	  current_level = current_level->level_down;
	  the_city->level_up = current_level;
	}

      buf = fread_string_eol (fp);
      buf = buf + 3;

      buf = one_argument (buf, number_buf);
      x_size = atoi (number_buf);
      buf = one_argument (buf, number_buf);
      y_size = atoi (number_buf);

      current_level->num_levels = num_levels;
      current_level->level_number = count_levels;
      current_level->x_size = x_size;
      current_level->y_size = y_size;
      current_level->reference_x = number_range (0, x_size - 1);
      current_level->reference_y = number_range (0, y_size - 1);
      sprintf (buf, "Level %d has reference point (%d, %d).",
	       current_level->level_number, current_level->reference_x,
	       current_level->reference_y);
      log_string (buf);
      current_level->level_down = the_city;
      current_level->rooms_on_level = 
	alloc_mem(x_size*y_size*sizeof (ROOM_DATA));

      clear_level (current_level->rooms_on_level, x_size, y_size,
		   current_level);
      set_descs_level (current_level->rooms_on_level, x_size, y_size,
		       current_level);

      set_walls_level (current_level->rooms_on_level, x_size, y_size);
		       
    }

  /* make a safe area for people to start from and god rooms for the friendly
     imms */
  log_string ("Making safe room and god rooms . . .");
  
#define NUM_EXTRA_ROOMS 5
  for (count_extra_rooms = 0; count_extra_rooms < NUM_EXTRA_ROOMS; 
       count_extra_rooms++)
    {
      some_room = alloc_perm (sizeof (ROOM_DATA));
      *some_room = room_zero;
      switch (count_extra_rooms)
	{
	case 0 : some_room->name = 
	  str_dup ("Type 'leave' to leave the safe room and enter the game.");
	  safe_area = some_room;
	  break;
	case 1 : some_room->name = 
	  str_dup ("Explosives depot.  Dropping things here will lag game.");
	  explosive_area = some_room;
	  break;
	case 2 : some_room->name = 
	  str_dup ("This is a standard imp room.  WOW!");
	  someimp_area = some_room;
	  break;
	case 3 : some_room->name = 
	  str_dup ("This is a general god room.  WOW!");
	  god_general_area = some_room;
	  break;
	case 4 : some_room->name =
	  str_dup ("This is a mob storage room.  Do not destroy any of these "
		   "mobs.");
	  store_area = some_room;
	  break;
	}
      some_room->description = 
	some_room->name;
      for (count = 0; count < 6; count++)
	{
	  SET_BIT (some_room->exit[count], EX_ISWALL);
	  SET_BIT (some_room->exit[count], EX_ISNOBREAKWALL);
	}
      some_room->contents = NULL;
      some_room->people = NULL;
      some_room->mine = NULL;
      some_room->interior_of = NULL;
      some_room->this_level = the_city;
      some_room->x = some_room->y = some_room->level = -1;
    }

  log_string ("City has been fully read.");

  log_string ("Randomizing all levels.");
  for (current_level = the_city; current_level->level_down != the_city; 
       current_level = current_level->level_down)
    {
      sprintf (buf, "Randomizing level %d.", current_level->level_number);
      log_string(buf);
      randomize_level (current_level);
    }
  sprintf (buf, "Randomizing level %d.", current_level->level_number);
  log_string(buf);
  randomize_level (current_level);

  return;
}

/* returns info on the common area where 2 levels correspond */
void find_level_commonality (LEVEL_DATA *first_level, 
			       LEVEL_DATA *second_level, int *x_size, 
			       int *y_size, int *lev1_lower_x,
			       int *lev1_lower_y, int *lev2_lower_x,
			       int *lev2_lower_y)
{
  int x_dist_first, x_dist_second, y_dist_first, y_dist_second;

  /* use x/y level referrences to determine the amount of area the level
     has with its up and down neighbors and then calculate the number of up
     and down exits there should be */
  if ((*lev1_lower_x = first_level->reference_x - 
       second_level->reference_x) >= 0)
    /* left limit of second level has corresponding square on first level */
    *lev2_lower_x = 0;
  else
    {
      *lev1_lower_x = 0;
      *lev2_lower_x = second_level->reference_x - first_level->reference_x;
    }

  if ((*lev1_lower_y = first_level->reference_y - 
       second_level->reference_y) >= 0)
    /* bottom limit of second level has corresponding square on first level */
    *lev2_lower_y = 0;
  else
    {
      *lev1_lower_y = 0;
      *lev2_lower_y = second_level->reference_y - first_level->reference_y;
    }

  x_dist_first = (first_level->x_size - 1) - first_level->reference_x;
  x_dist_second = (second_level->x_size - 1) - second_level->reference_x;
  y_dist_first = (first_level->y_size - 1) - first_level->reference_y;
  y_dist_second = (second_level->y_size - 1) - second_level->reference_y;
  *x_size = (first_level->reference_x - *lev1_lower_x) + 
    ((x_dist_first > x_dist_second) ? x_dist_second : x_dist_first);
  *y_size = (first_level->reference_y - *lev1_lower_y) + 
    ((y_dist_first > y_dist_second) ? y_dist_second : y_dist_first);
}

void mark_connected (ROOM_DATA *a_room)
{
  int count;

  SET_BIT (a_room->room_flags, ROOM_CONNECTED);
  for (count = 0; count < 4; count++)
    if (!IS_SET (a_room->exit[count], EX_ISWALL) &&
	!IS_SET ((get_to_room (a_room, count))->room_flags, ROOM_CONNECTED))
      mark_connected (get_to_room (a_room, count));
}

void connect_level (LEVEL_DATA *a_level)
{
  int x, y, topx, topy, dir;
  ROOM_DATA *curr_room, *to_room;
  const	sh_int	rev_dir		[]		=
    {
      2, 3, 0, 1, 5, 4
    };

  mark_connected (a_level->rooms_on_level);
  topx = a_level->x_size - 2;
  topy = a_level->y_size - 2;
  for (x = 1; x <= topx; x++)
    for (y = 1; y <= topy; y++)
      if (!IS_SET ((curr_room = to_room =
		    index_room (a_level->rooms_on_level, x, y))->room_flags,
		   ROOM_CONNECTED))
	{
	  do
	    {
	      curr_room = to_room;
	      dir = number_range (0, 3);
	      to_room = get_to_room (curr_room, dir);
	      REMOVE_BIT (curr_room->exit[dir], EX_ISWALL);
	      REMOVE_BIT (curr_room->exit[dir], EX_ISNOBREAKWALL);
	      REMOVE_BIT (to_room->exit[rev_dir[dir]], EX_ISWALL);
	      REMOVE_BIT (to_room->exit[rev_dir[dir]], EX_ISNOBREAKWALL);
	    } while (!IS_SET(to_room->room_flags, ROOM_CONNECTED));
	  mark_connected (curr_room);
	}
  /* just to make sure */
  log_string ("Disconnected:");
  for (x = 1; x <= topx; x++)
    for (y = 1; y <= topy; y++)
      if (!IS_SET ((index_room (a_level->rooms_on_level, x, y))->room_flags,
		   ROOM_CONNECTED))
	{
	  sprintf (log_buf, "%d, %d", x, y);
	  log_string (log_buf);
	}
}

void randomize_level (LEVEL_DATA *a_level)
{
  const	sh_int	rev_dir		[]		=
    {
      2, 3, 0, 1, 5, 4
    };

  char buf[MAX_STRING_LENGTH];
  ROOM_DATA *temp_room;
  sh_int count, x, y, topx, topy, the_exit;
  sh_int common_area, num_up_exits, num_down_exits;
  int x_area, y_area, lev_x_lower, lev_y_lower, down_x_lower, down_y_lower;
  int up_x_lower, up_y_lower;

  log_string ("Randomizing cardinal direction exits.");
  topx = a_level->x_size - 2;
  topy = a_level->y_size - 2;
  for (count = 0; count < (topx*topy*3) / 4; count++)
    {
      x = number_range (1, topx);
      y = number_range (1, topy);
      the_exit = number_range (0, 3);
      temp_room = index_room (a_level->rooms_on_level, x, y);
      SET_BIT (temp_room->exit[the_exit], EX_ISWALL);
      temp_room = get_to_room(temp_room, the_exit);
      SET_BIT (temp_room->exit[rev_dir[the_exit]], EX_ISWALL);
    }

  log_string ("Guaranteeing level connectivity.");
  connect_level (a_level);

  log_string ("Randomizing verticle exits.");
  find_level_commonality (a_level, a_level->level_down, &x_area, &y_area,
			  &lev_x_lower, &lev_y_lower, &down_x_lower,
			  &down_y_lower);
  if (a_level->level_down == the_city)
    num_down_exits = 0;
  else
    {
      /* one exit per 5X5 area on average */
      num_down_exits = x_area*y_area / (5*5);
      if (num_down_exits < 1)
	num_down_exits = 1;
    }

  if (num_down_exits)
    sprintf (buf, "There will be %d down exits on level %d in the range "
	     "(%d - %d, %d - %d)", num_down_exits, a_level->level_number, 
	     lev_x_lower, lev_x_lower + x_area, lev_y_lower, lev_y_lower + 
	     y_area);
  else
    sprintf (buf, "no down exits are possible on this level.");
  log_string (buf);
  for (count = 0; count < num_down_exits; count++)
    {
      do
	{
	  temp_room = index_room (a_level->rooms_on_level, 
				  x = number_range (lev_x_lower, lev_x_lower + 
						    x_area), 
				  y = number_range (lev_y_lower, lev_y_lower + 
						    y_area));
	}
      while (!IS_SET (temp_room->exit[DIR_DOWN], EX_ISWALL));
      log_string ("down added to %d, %d, %d", temp_room->x,
		  temp_room->y, temp_room->level);
      temp_room->exit[DIR_DOWN] = 0;
    }
  find_level_commonality (a_level, a_level->level_up, &x_area, &y_area,
			  &lev_x_lower, &lev_y_lower, &up_x_lower,
			  &up_y_lower);
  if (a_level == the_city)
    num_up_exits = 0;
  else
    {
      /* one exit per 5X5 area on average */
      num_up_exits = x_area*y_area / (5*5);
      if (num_up_exits < 1)
	num_up_exits = 1;
    }
  if (num_up_exits)
    sprintf (buf, "There will be %d up exits on level %d in the range "
	     "(%d - %d, %d - %d)", num_up_exits, a_level->level_number, 
	     lev_x_lower, lev_x_lower + x_area, lev_y_lower, lev_y_lower + 
	     y_area);
  else
    sprintf (buf, "no up exits are possible on this level.");
  log_string (buf);
  for (count = 0; count < num_up_exits; count++)
    {
      do
	{
	  temp_room = index_room (a_level->rooms_on_level, 
				  x = number_range (lev_x_lower, lev_x_lower + 
						    x_area), 
				  y = number_range (lev_y_lower, lev_y_lower + 
						    y_area));
	}
      while (!IS_SET (temp_room->exit[DIR_UP], EX_ISWALL));
      log_string ("up added to %d, %d, %d", temp_room->x,
		  temp_room->y, temp_room->level);
      temp_room->exit[DIR_UP] = 0;
    }
}

/*
 * Snarf a help section.
 */
void load_helps( FILE *fp )
{
    HELP_DATA *pHelp;

    log_string ("Loading helps.");
    for ( ; ; )
    {
	pHelp		= alloc_perm( sizeof(*pHelp) );
	pHelp->level	= fread_number( fp );
	pHelp->keyword	= fread_string( fp );
	if ( pHelp->keyword[0] == '$' )
	    break;
	pHelp->text	= fread_string( fp );

	if ( !str_cmp( pHelp->keyword, "greeting1" ) )
	    help_greeting1 = pHelp->text;
	if ( !str_cmp( pHelp->keyword, "greeting2a" ) )
	    help_greeting2a = pHelp->text;
	if ( !str_cmp( pHelp->keyword, "greeting2b" ) )
	    help_greeting2b = pHelp->text;

	if ( help_first == NULL )
	    help_first = pHelp;
	if ( help_last  != NULL )
	    help_last->next = pHelp;

	help_last	= pHelp;
	pHelp->next	= NULL;
	top_help++;
    }
    log_string ("Helps have been read.");
    return;
}

/*
 * Snarf notes file.
 */
void load_notes( void )
{
    FILE *fp;
    NOTE_DATA *pnotelast;

    if ( ( fp = fopen( NOTE_FILE, "r" ) ) == NULL )
	return;

    pnotelast = NULL;
    for ( ; ; )
    {
	NOTE_DATA *pnote;
	char letter;

	do
	{
	    letter = getc( fp );
	    if ( feof(fp) )
	    {
		fclose( fp );
		return;
	    }
	}
	while ( isspace(letter) );
	ungetc( letter, fp );

	pnote		= alloc_perm( sizeof(*pnote) );

	if ( str_cmp( fread_word( fp ), "sender" ) )
	    break;
	pnote->sender	= fread_string( fp );

	if ( str_cmp( fread_word( fp ), "date" ) )
	    break;
	pnote->date	= fread_string( fp );

	if ( str_cmp( fread_word( fp ), "stamp" ) )
	    break;
	pnote->date_stamp = fread_number(fp);

	if ( str_cmp( fread_word( fp ), "to" ) )
	    break;
	pnote->to_list	= fread_string( fp );

	if ( str_cmp( fread_word( fp ), "subject" ) )
	    break;
	pnote->subject	= fread_string( fp );

	if ( str_cmp( fread_word( fp ), "text" ) )
	    break;
	pnote->text	= fread_string( fp );
	pnote->next = NULL;

 	if ( pnote->date_stamp < current_time - (14*24*60*60) /* 2 wks */)
        {
            free_string( pnote->text );
            free_string( pnote->subject );
            free_string( pnote->to_list );
            free_string( pnote->date );
            free_string( pnote->sender );
            pnote->next     = note_free;
            note_free       = pnote;
            pnote           = NULL;
	    continue;
        }

	if ( note_list == NULL )
	    note_list		= pnote;
	else
	    pnotelast->next	= pnote;

	pnotelast	= pnote;
    }

    strcpy( strArea, NOTE_FILE );
    fpArea = fp;
    bug( "Load_notes: bad key word.", 0 );
    exit (STATUS_ERROR);
    return;
}

void load_accounts (FILE *fp)
{
  char *login;
  ACCOUNT_DATA *creator;

  log_string ("loading accounts . . .");
  while ((login = fread_word (fp))[0] != '$')
    {
      creator = alloc_mem (sizeof(ACCOUNT_DATA));
      creator->login = str_dup(login);
      creator->password = str_dup (fread_word(fp));
      creator->character = NULL;
      creator->next = accounts_list;
      accounts_list = creator;
    }
  log_string ("Done.");
}

void load_top (FILE *fp)
{
  int kills;
  int count;

  log_string ("loading top file . . .");
  for (count = 0; count < NUM_TOP; count++)
    {
      top_players_kills[count].kills = fread_number(fp);
      top_players_kills[count].name = fread_string(fp);
    }
  log_string ("Done.");
}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
    static OBJ_DATA obj_zero;
    static ROOM_DATA room_zero;
    OBJ_DATA *obj;
    sh_int count;

    if ( pObjIndex == NULL )
    {
	bug( "Create_object: NULL pObjIndex.", 0 );
	exit (STATUS_ERROR);
    }

    if ( obj_free == NULL )
    {
	obj		= alloc_perm( sizeof(*obj) );
    }
    else
    {
	obj		= obj_free;
	obj_free	= obj_free->next;
    }

    *obj		= obj_zero;
    obj->pIndexData	= pObjIndex;
    obj->in_room	= NULL;

    obj->wear_loc	= -1;

    obj->name		= pObjIndex->name;
    obj->short_descr	= pObjIndex->short_descr;
    obj->description	= pObjIndex->description;
    obj->explode_desc   = pObjIndex->explode_desc;
    obj->extract_flags	= pObjIndex->extract_flags;
    obj->general_flags	= pObjIndex->general_flags;
    obj->usage_flags	= pObjIndex->usage_flags;
    obj->wear_flags	= pObjIndex->wear_flags;
    obj->weight		= pObjIndex->weight;
    obj->item_type      = pObjIndex->item_type;

    obj->next		= object_list;
    obj->range          = pObjIndex->range;
    obj->owner          = NULL;
    obj->arrival_time   = -1;
    obj->ammo           = pObjIndex->ammo;
    obj->ammo_type      = pObjIndex->ammo_type;
    obj->burn_time      = pObjIndex->burn_time;
    obj->rounds_per_second = pObjIndex->rounds_per_second;
    obj->armor          = pObjIndex->armor;
    obj->wait_time      = pObjIndex->rounds_per_second;
    obj->hp_char        = pObjIndex->hp_char;
    obj->hp_struct      = pObjIndex->hp_struct;
    obj->extract_me     = 0;
    obj->destination    = NULL;
    obj->valid          = VALID_VALUE;
    if ((obj->item_type == ITEM_TEAM_VEHICLE) || 
	(obj->item_type == ITEM_TEAM_ENTRANCE))
      {
	obj->interior = alloc_mem (sizeof (ROOM_DATA));
                      /*malloc (sizeof (ROOM_DATA));*/
	*(obj->interior) = room_zero;
	obj->interior->interior_of = obj;
	switch (obj->item_type)
	  {
	  case ITEM_TEAM_VEHICLE:
	    obj->interior->name = str_dup ("The inside of a tank.");
	    SET_BIT (obj->interior->room_flags, ROOM_TANK);
	    break;
	  }
	obj->interior->description = obj->interior->name;
	for (count = 0; count < 6; count++)
	  {
	    SET_BIT (obj->interior->exit[count], EX_ISWALL);
	    SET_BIT (obj->interior->exit[count], EX_ISNOBREAKWALL);
	  }
	obj->interior->contents = NULL;
	obj->interior->people = NULL;
	obj->interior->mine = NULL;
	obj->interior->this_level = the_city;
	obj->interior->x = obj->interior->y = obj->interior->level = -1;
      }
    else
      obj->interior = NULL;
    for (count = 0; count < 3; count++)
      {
	obj->damage_char[count] = pObjIndex->damage_char[count];
	obj->damage_structural[count] = pObjIndex->damage_structural[count];
      }
    object_list		= obj;
    pObjIndex->count++;

    return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone)
{
    int i, count;
    AFFECT_DATA *paf;
/*    EXTRA_DESCR_DATA *ed,*ed_new; */

    if (parent == NULL || clone == NULL)
	return;

    /* start fixing the object */
    clone->name 	= str_dup(parent->name);
    clone->short_descr 	= str_dup(parent->short_descr);
    clone->description	= str_dup(parent->description);
    clone->item_type	= parent->item_type;
    clone->extract_flags	= parent->extract_flags;
    clone->general_flags	= parent->general_flags;
    clone->usage_flags	= parent->usage_flags;
    clone->wear_flags	= parent->wear_flags;
    clone->weight	= parent->weight;
    clone->timer	= parent->timer;
    clone->owner        = parent->owner;
    clone->range        = parent->range;
    for (count = 0; count < 3; count++)
      {
	clone->damage_char[count] = parent->damage_char[count];
	clone->damage_structural[count] = parent->damage_structural[count];
      }

}

CHAR_DATA *clone_mobile (CHAR_DATA *ch)
{
  CHAR_DATA *clone;
  OBJ_DATA *tracker;
  OBJ_DATA *obj, *obj2;

  clone = alloc_char ();
  *clone = *ch;

  /* duplicate inventory */
  clone->carrying = NULL;
  
  char_to_room (clone, ch->in_room);
  for (tracker = ch->carrying; tracker; tracker = tracker->next_content)
    {
      obj_to_char (obj = create_object (tracker->pIndexData, 0), clone);
      obj->extract_me = 1;
      if (tracker->contains)
	{
	  obj_to_obj (obj2 = create_object (tracker->contains->pIndexData, 0),
		      obj);
	  obj2->extract_me = 1;
	}
	  
    }
  do_wear (clone, "all");
  do_load (clone, "");
  clone->next = char_list;
  char_list = clone;
  return clone;
}

/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
    static CHAR_DATA ch_zero;
    int i;

    *ch				= ch_zero;
    ch->logon			= current_time;
    ch->lines			= PAGELEN;
    ch->position		= POS_STANDING;
    ch->hit			= 1;
    ch->max_hit			= 3000;
    return;
}



/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    if ((vnum >= top_obj_index) || all_objects[vnum - 1].vnum != vnum )
      {
	if ( fBootDb )
	  {
	    log_string( "Get_obj_index: bad vnum %d.", vnum);
	    exit (STATUS_ERROR);
	  }
	return NULL;
      }

    return &(all_objects[vnum-1]);
}

/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	bug( "Fread_number: bad format.", 0 );
	exit (STATUS_ERROR);
    }

    while ( isdigit(c) )
    {
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

long fread_flag( FILE *fp)
{
    int number;
    char c;

    do
    {
	c = getc(fp);
    }
    while ( isspace(c));

    number = 0;

    if (!isdigit(c))
    {
	while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
	{
	    number |= flag_convert(c);
	    c = getc(fp);
	}
    }

    while (isdigit(c))
    {
	number = number * 10 + c - '0';
	c = getc(fp);
    }

    if (c == '|')
	number |= fread_flag(fp);

    else if  ( c != ' ')
	ungetc(c,fp);

    return number;
}

long flag_convert(char letter )
{
    long bitsum = 0;
    char i;

    if ('A' <= letter && letter <= 'Z') 
    {
	bitsum = 1;
	for (i = letter; i > 'A'; i--)
	    bitsum *= 2;
    }
    else if ('a' <= letter && letter <= 'z')
    {
	bitsum = 67108864; /* 2^26 */
	for (i = letter; i > 'a'; i --)
	    bitsum *= 2;
    }

    return bitsum;
}

char *fread_string( FILE *fp )
{
  char temp [32000], *thestring;
  int count = 1;

  temp[0] = getc(fp);

  while (isspace (temp[0]))
    temp[0] = getc (fp);

  if (temp[0] != '~')
    {
      while ((count < 32000) && ((temp[count++] = getc(fp)) != '~'))
        if (temp[count - 1] == '\n')
          {
            temp[count++ - 1] = '\r';
            temp[count - 1] = '\n';
          }
	else
	  if (temp[count - 1] == '\r')
	    count--;

      temp[count - 1] = '\0';
      if (count >= 20000) log_string ("REALBUG: Unterminated string found in "
                                      "fread_string.");
      thestring = top_string;
      top_string = &(top_string[strlen(temp) + 1]);
      if (top_string > string_space + MAX_STRING)
        {
          log_string ("REALBUG: Out of string memory.");
          exit (STATUS_ERROR);
        }
      strcpy (thestring, temp);
      return thestring;
    }
  else
    return &str_empty[0];
}

char *fread_string_eol( FILE *fp )
{
  char temp [32000], *thestring;
  int count = 1;

  temp[0] = getc(fp);

  while (isspace (temp[0]))
    temp[0] = getc (fp);

  if (temp[0] != '~')
    {
      while ((count < 32000) && ((temp[count++] = getc(fp)) != '\n'))
        if (temp[count - 1] == '\n')
          {
            temp[count++ - 1] = '\r';
            temp[count - 1] = '\n';
          }
	else
	  if (temp[count - 1] == '\r')
	    count--;

      temp[count - 1] = '\0';
      if (count >= 32000) log_string ("REALBUG: Unterminated string found in "
                                      "fread_string.");
      thestring = top_string;
      top_string = &(top_string[strlen(temp) + 1]);
      if (top_string > string_space + MAX_STRING)
        {
          log_string ("REALBUG: Out of string memory.");
          exit (STATUS_ERROR);
        }
      strcpy (thestring, temp);
      return thestring;
    }
  else
    return &str_empty[0];
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    bug( "Fread_word: word too long.", 0 );
    exit (STATUS_ERROR);
    return NULL;
}



/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '~' )
	    *str = '-';
    }

    return;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
	bug( "Str_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )
    {
	bug( "Str_cmp: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
	bug( "Strn_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )
    {
	bug( "Strn_cmp: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
	return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
	if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
	    return FALSE;
    }

    return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
	return FALSE;
    else
	return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strcap[i] = LOWER(str[i]);
    strcap[i] = '\0';
    strcap[0] = UPPER(strcap[0]);
    return strcap;
}


/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;

    if ( IS_NPC(ch) || str[0] == '\0' )
	return;

    fclose( fpReserve );
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	perror( file );
	send_to_char( "Could not open the file!\n\r", ch );
    }
    else
    {
	fprintf( fp, "%s: %s\n", ch->names, str );
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Reports a bug.
 */
void bug( const char *str, int param )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;

    if ( fpArea != NULL )
    {
	int iLine;
	int iChar;

	if ( fpArea == stdin )
	{
	    iLine = 0;
	}
	else
	{
	    iChar = ftell( fpArea );
	    fseek( fpArea, 0, 0 );
	    for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
	    {
		while ( getc( fpArea ) != '\n' )
		    ;
	    }
	    fseek( fpArea, iChar, 0 );
	}

	sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
	log_string( buf );

	if ( ( fp = fopen( "../boot/shutdown.txt", "a" ) ) != NULL )
	{
	    fprintf( fp, "[*****] %s\n", buf );
	    fclose( fp );
	}
    }

    strcpy( buf, "[*****] BUG: " );
    sprintf( buf + strlen(buf), str, param );
    log_string( buf );

    return;
}

void log_string (char *format, ...)
{
  va_list ap;
  struct timeval time;
  char *dateline;
  char buf[MAX_STRING_LENGTH], *buf_p = buf;

  gettimeofday (&time, NULL);
  dateline = ctime((time_t*)&(time.tv_sec));
  dateline[strlen(dateline)-1] = '\0';
  sprintf (buf_p, "%d %s: ", iteration, dateline);
  buf_p += strlen (buf_p);
  va_start (ap, format);
  vsprintf (buf_p, format, ap);
  buf_p += strlen (buf_p);
  buf_p[0] = '\n';
  buf_p[1] = '\r';
  buf_p[2] = '\0';
  va_end (ap);

  fprintf (stderr, "%s", buf);
}

int number_range (int from, int to)
{
  return random ()%(to-from + 1) + from;
}

void expand_city ()
{
#define EXPAND_AMOUNT 6
  ROOM_DATA *old_level1;
  int old_size, x, y;
  OBJ_DATA *obj;
  CHAR_DATA *ch;
  ROOM_DATA *old_room;
  int count;

  expand_event = 1;
  log_string ("EXPANDING CITY");
  srandom (game_seed);
  old_level1 = the_city->rooms_on_level;
  old_size = the_city->x_size * the_city->y_size * sizeof (ROOM_DATA);
  the_city->x_size += EXPAND_AMOUNT;
  the_city->y_size += EXPAND_AMOUNT;
  the_city->rooms_on_level =
    alloc_mem(the_city->x_size*the_city->y_size*sizeof (ROOM_DATA));
  /* room descriptions */
  clear_level (the_city->rooms_on_level, the_city->x_size, the_city->y_size,
	       the_city);
  set_descs_level (the_city->rooms_on_level, the_city->x_size,
		   the_city->y_size, the_city);

  /* walls */
  set_walls_level (the_city->rooms_on_level, the_city->x_size,
		   the_city->y_size);
  /* we add more downs to make them more plentiful with the bigger grid */
  the_city->reference_x += EXPAND_AMOUNT / 2;
  the_city->reference_y += EXPAND_AMOUNT / 2;

  randomize_level (the_city);

  /* down exits */
  for (x = 0; x < the_city->x_size-EXPAND_AMOUNT; x++)
    for (y = 0; y < the_city->y_size-EXPAND_AMOUNT; y++)
      {
	/* this is a dirty hack, but it preserves the call to index_room */
	/* (index_room relies on old_level1->this_level which points to
	   the_city) */
	the_city->x_size -= EXPAND_AMOUNT;
	the_city->y_size -= EXPAND_AMOUNT;
	old_room = index_room(old_level1, x, y);
	the_city->x_size += EXPAND_AMOUNT;
	the_city->y_size += EXPAND_AMOUNT;
	if (!old_room->exit[DIR_DOWN])
	  {
	    log_string ("copying down exit for %d, %d", x, y);
	    (index_room(the_city->rooms_on_level, x + EXPAND_AMOUNT / 2,
			y + EXPAND_AMOUNT / 2))->exit[DIR_DOWN] = 0;
	  }
      }

  /* move objects from old level to new level */
  for (obj = object_list; obj; obj = obj->next)
    {
      if (!obj->in_room)
	if (!obj->destination)
	  {
	    if (obj->carried_by || obj->in_obj || 
		(obj->item_type == ITEM_TEAM_ENTRANCE))
	      continue;
	    log_string ("lost object detected.");
	    *((char*)NULL) = 'b';
	  }
	else
	  {
	    x = obj->destination->x + EXPAND_AMOUNT / 2;
	    y = obj->destination->y + EXPAND_AMOUNT / 2;

	    if (obj->destination->mine == obj)
	      {
		(index_room (the_city->rooms_on_level, x, y))->mine = obj;
		obj->destination->mine = NULL;
	      }
	    obj->destination = index_room (the_city->rooms_on_level, x, y);
	  }
      else
	{
	  if (obj->in_room->level)
	    continue;

	  if (!obj->in_room->x)
	    x = 0;
	  else
	    x = (obj->in_room->x == the_city->x_size - EXPAND_AMOUNT - 1) ?
	      the_city->x_size - 1 : obj->in_room->x + EXPAND_AMOUNT / 2;
	  if (!obj->in_room->y)
	    y = 0;
	  else
	    y = (obj->in_room->y == the_city->y_size - EXPAND_AMOUNT - 1) ?
	      the_city->y_size - 1 : obj->in_room->y + EXPAND_AMOUNT / 2;
	  
	  obj_from_room (obj);
	  obj_to_room (obj, index_room (the_city->rooms_on_level, x, y));
	}
    }

  /* move mobs from old level to new level */
  for (ch = char_list; ch; ch = ch->next)
    {
      if (ch->in_room->level)
	continue;

      if (!ch->in_room->x)
	x = 0;
      else
	x = (ch->in_room->x == the_city->x_size - EXPAND_AMOUNT - 1) ?
	  the_city->x_size - 1 : ch->in_room->x + EXPAND_AMOUNT / 2;
      if (!ch->in_room->y)
	y = 0;
      else
	y = (ch->in_room->y == the_city->y_size - EXPAND_AMOUNT - 1) ?
	  the_city->y_size - 1 : ch->in_room->y + EXPAND_AMOUNT / 2;

      char_from_room (ch);
      char_to_room (ch, index_room (the_city->rooms_on_level, x, y));
    }
  for (count = 0; count <3; count++)
    ammo_repop[count] = index_room (the_city->rooms_on_level,
				    ammo_repop[count]->x + EXPAND_AMOUNT / 2,
				    ammo_repop[count]->y + EXPAND_AMOUNT / 2);
  free_mem (old_level1, old_size);
  log_string ("EXPANSION COMPLETE");
  expand_event = 0;
}

/* snarf a socials file */
void load_socials( FILE *fp)
{
  char buf[MAX_STRING_LENGTH];
  log_string ("Loading socials.");
    for ( ; ; ) 
    {
    	struct social_type social;
    	char *temp;
        /* clear social */
	social.char_no_arg = NULL;
	social.others_no_arg = NULL;
	social.char_found = NULL;
	social.others_found = NULL;
	social.vict_found = NULL; 
	social.char_not_found = NULL;
	social.char_auto = NULL;
	social.others_auto = NULL;
	social.name = NULL;

    	temp = fread_word(fp);
    	if (!strcmp(temp,"#0"))
	    break;  /* done */

    	social.name = str_dup(temp);
    	fread_to_eol(fp);

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.char_no_arg = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue; 
	}
        else
	    social.char_no_arg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_no_arg = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.others_no_arg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
       	else
	    social.char_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.others_found = temp; 

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.vict_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.vict_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_not_found = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.char_not_found = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
	     social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.char_auto = temp;
         
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
	    social.others_auto = temp; 
	
	social_table[social_count] = social;
    	social_count++;
   }
  social_table[social_count].name = NULL;
  sprintf (buf, "%d socials read in.", social_count);
  log_string (buf);
  if (social_count >= MAX_SOCIALS)
    {
      log_string ("Too many socials.");
      exit (STATUS_ERROR);
    }
   return;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}
