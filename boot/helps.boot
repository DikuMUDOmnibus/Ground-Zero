#HELPS

-1 DIKU~
.                    Original game idea, concept, and design:

	  Katja Nyboe               [Superwoman] (katz@freja.diku.dk)
	  Tom Madsen              [Stormbringer] (noop@freja.diku.dk)
	  Hans Henrik Staerfeldt           [God] (bombman@freja.diku.dk)
	  Michael Seifert                 [Papi] (seifert@freja.diku.dk)
	  Sebastian Hammer               [Quinn] (quinn@freja.diku.dk)

		     Additional contributions from:

Michael Curran  - the player title collection and additional locations.
Ragnar Loenn    - the bulletin board.
Bill Wisner     - for being the first to successfully port the game,
		  uncovering several old bugs, uh, inconsistencies,
		  in the process.

And: Mads Haar and Stephan Dahl for additional locations.

Developed at: DIKU -- The Department of Computer Science
		      at the University of Copenhagen.

~


-1 MERC~
[Note: this entry may not be removed or altered.  See our license.txt.]

This mud is based on Merc 2.1, created by Furey, Hatchet, and Kahn.  Merc 2.1.
is available as Merc_21c.tar.Z from ferkel.ucsb.edu and ftp.math.okstate.edu.
E-mail to 'merc-request@kpc.com' to join the merc mailing list.

Thanks to ...
  ... Diku Mud for starting it all.
  ... The Free Software Foundation and DJ Delorie for kick-ass tools.
  ... Copper Mud and Alfa Mud for releasing their code and worlds.
  ... Aod of Generic for ... well, everything.  You're a hoopy frood, Aod.
  ... Alander for many ideas and contributions.
  ... John Brothers of Silly for permission to use Silly code and worlds.
  ... Zrin for administering the mailing list.
  ... Abaddon for proofreading our comm.c.
  ... Hind, Quin, Vic, Diavolo, Oleg, Trienne and others for porting help.
  ... Diavolo, Grodyn, Morgenes, and others for code and bug fixes.
  ... Raff, Doctor, VampLestat, Nirrad, Tyrst, PinkF, Chris, Glop for worlds.
  ... the players and imps of Mud Dome, Final Mud, Mud Lite, Vego Mud, Rivers
      of Mud, Ruhr Mud, and Mystic Realms for bug reports, ideas, new code,
      and hours of enjoyment.

Share and enjoy.
~


-1 SUMMARY~
MOVEMENT			    TANK OPERATION
north south east west up down       enter leave man
exits scan push track where
teleport

OBJECTS / COMBAT                    INFORMATION / COMMUNICATION
get drop give load unload           help credits commands
wear wield pull throw shoot         report score time where who
lock unlock open close              bug idea typo emote sadd slist srem
inventory equipment look            say tell red blue bounty kills
bury depress use donate             note password title sockets objectives

OTHER
ld delete teams death kill_points

and PLEASE do a help rules!!!!  There are almost no rules here, and the purpose
in reading this is more for you to come to terms with just how legal most
everything is on here so you won't think someone is cheating or something.
~

-1 CHARS~
[Updated 8 May 1998]

Helps ye cope with spam by allowing ye to specify how many characters of
output ye allow the game to send ye at once.
~

-1 DONATE~
[Updated 8 May 1998]

Yer team HEADQUARTERS is located at one of the corners of level 0.  Ye may
donate items to yer team with the DONATE command. This will put the items
in yer HEADQUARTERS where other members of yer team can get them. There is,
however, a limit of how many items may be donated at once.
~

0 rules~

This game will presumably have some rules.
~


0 scan track where~

SYNTAX: scan
SYNTAX: track <player_name>
SYNTAX: where
SYNTAX: where <plsyer_name>

Scan tells you the characters that are near you four squares in the cardinal
directions.

Track tells you the general direction that another player in the game is
relative to you.  (ie this allows you to TRACK them!  Hence the name! =)  )
Conveniently every player in the game has a transmission beacon attached to 
them that allows any other player to tell their general direction relative
to that player.  Unfortunately, being on differrent levels or in a tank
interferes with the signal so it can't be used ALL the time.

Where gives the general area that players around you are in.
You may specify a particular character with this command and it will only list
their name and general location.
~

-1 PUSH~
[Updated 8 May 1998]

Syntax :
	push	<player_name>	<direction>
	push	<obj_name>	<direction>

Ye can PUSH a person who is in the same room as yerself in any given
direction (assuming there is an EXIT in this direction) with this command.

PUSH may also be used on barrels to move them around and create really,
really big explosions.
~


0 enter leave man tank~

Tanks are destructive vehicles that sit around in the game until someone
gets in one and goes and blows some people up.
If you happen to get close enough to one without getting killed, here is how
you enter it and try to drive the thing.

Upon seeing the tank, type enter tank and you will go inside.
(if tank is the first item in the room, enter or ent also works by itself)

Inside the tank there are 3 stations.  One that controls the direction the
turret faces.  One that controls the movement of the vehicle and one that
controls the tanks shields (you can only shield the tank in one direction at
a time, so you need to specify where you think attacks will come from).
In order for the tank to become operational, you need to have all 3 stations
manned by people on the same team, and there cannot be anyone else inside
the tank other than those three people.  A forth person from the same team
or another team will prevent the functioning of the tank.

Once the last person has manned a position, the tank will turn on and you
can begin operating it.

For detailed info on operating a tank see help driving.
~


0 teleport emergency~

Occasionally you will find that you are about to get killed and might
wanna get outta dodge in a hurry.  The command teleport was put in for
this very occation.  You just type teleport and you are redeployed
randomly to somewhere on level 0.

To prevent this from being abused (ie transporting right before you get
killed) you must be at full health for it to work.

Also, teleport cannot be used _all_ the time.  The teleport systems 
must be activated and operational.  If they are, and you type teleport, 
you will.  If they aren't you will recieve a message accross your 
comm-badge informing you of some technical problems with the 
transportation systems.  (see radio as well)

NOTE: teleport emergency will teleport you at anytime...but will take off
      5900 hp in doing so because making use of the teleporter when it is
      offline leads to a bumpy ride.

~

0 driving~
At this point we assume that you have managed to get the tank operational.
If you have not done this, see help tank.

Manning all of the positions involves using your direction keys (ie n,e,s,w,u,d)
as if you were walking around.
This is an especially familiar setup if you are manning the drive position
because it is basically the same as walking around the game yourself.
As the driver, you just navigate the tank around the game.
As the one manning the turret, you need to point the turret in the direction
you want to shoot.  The tank will fire automatically when it has someone
to shoot at, but you need to point the turret in the direction of the
approaching enemies or it won't fire.
If you are the one manning the shield, you need to shield in the direction
that attacks are coming from.  If someone is firing at the tank from the east,
you need to shield east.  If someone is in the room with you, you need to shield
down.  The tank does not have shields that deal well with explosions, so if
it rolls over a mine, someone puts c4 in the room with it, etc.  Don't expect
shielding down to lessen the damage to the tank.
The tank does heal damage as time passes though (amazing what futuristic tech
can do right?).

How do you get out of a tank?
Man a position that someone else is manning and it will turn off.

And what happens when the tank blows up?
All occupants die!
~

0 COLOR~ 
For those of you that have color, this game supports a type
of color code that is very much like ansi.  However it won't mess up
the screens of those that don't have color.  To turn color off type
color.  To see what the default colors for things like HPs are, type
color show. To see what colors are available, type color list.  Make a
note of the number next to the item you want to change in color show
and the number next to the color you want in color list. Then to
configure your colors type "color set <item number> <color number>" like
"color set 8 12" 

For those of you who are bored and not killing each other, "help colorlist"
explains how to put colors in your says/emotes/titles/sadds/life

~

0 colorlist~

To add color to your life and the lives of those trying to kill you is
a simple, and very thoughtful thing for you to do.

The unshifted tilde (a backwards ') is used to turn color on/off.
Because it is invisable to u, " will replace it in my example.

Syntax say/emote/title/etc "#<text>  (remember its not really a ")
Multiple colors can be used by placing a "# before the color change.

`` closes the color, returning it to b/w. (MUST be at end of color titles)

Each # does something different.  Trial and error is the best way to find 
which number code does which color.  Here are some examples...

1 is `1red`` @ is `@blue`` and 9 is `9black.``

~

0 sockets~

SYNTAX: sockets
SYNTAX: socket <char_name>

Yes folks!  That's right!  All mortals can use that covetted imm command 
"sockets" in this game.
This command gives you the site that a person is telnetting from when they
come to play this game.

Why, you ask, is this a mortal command now?
Well, I am curious as to why it was an imm command before.
What does it hurt for you all to know where eachother are from?  You can spot
multisessioning cheating yourself now.  If you notice that there are 
consistantly assholes pouring out of a particular site, we can get rid of them,
etc.
Seems like a good move to let everyone have the command.
~

-1 DAMAGE~
If ye have a weapon, wield it to kill someone. When ye get another weapon
try it on.

There are many different types of weapons right now on GZ.  Guns and
the like, do damage to 1 target at a time, and u must aim the weapon
at that person (ie, "kill JoeSchmuck").  You will also auto-attack
anyone that opens up at you (makes sense right?).

(See also "kill" "kill all")

HINT: The closer you are to an explosion the more it hurts! . . . DUH!
~

-1 LD LINK_DEAD LINK_DEATH~
[Updated 8 May 1998]

There is no nice little quit command here because of all the concerns about
people quitting right when they are about to die to avoid losing
KILL_POINTS. There is instead a command called LD which is short for
LINK_DEATH.

This command will leave yer character in the game but disconnect ye so that
ye can go to work/school/eat/sleep/whatever.

By the way, this command is only used by amateurs and weenies. Real players
finish the game.
~


0 DEATH~

Some info about dying in this game.
~

-1 TICK~
[Updated 8 May 1998]

Game actions are based upon interval timers, including combat and hitpoint
regeneration.

~

0 !~
[Updated 8 May 1998]

Syntax:
	!

! repeats the last command ye typed. Convenient aint it?
~


0 NORTH SOUTH EAST WEST UP DOWN~
Syntax: north
Syntax: south
Syntax: east
Syntax: west
Syntax: up
Syntax: down

Use these commands to walk in a particular direction.
~

0 EXITS~
Syntax: exits

Tells you the visible exits of the room you are in.
~

0 DROP GET GIVE TAKE~
Syntax: drop <object>
Syntax: get  <object>
Syntax: give <object> <character>

DROP drops an object on the ground.

GET gets an object from the ground.
TAKE is a synonym for get.

GIVE gives an object to another character.

DROP, GET and TAKE understand the object names 'ALL' for all objects and
'ALL.object' for all objects with the same name.
 Don't forget to put the  .  between the all and the <object>.

See also: LOAD UNLOAD
~

0 LOAD UNLOAD~

You can't shoot an empty gun, so don't forget to load your ammo!

Load by itself will load the weapon that you are wielding with whatever
ammo is available.  If you want to load it with something in particular,
you may type load hollow for example to load hollow tip bullets.

You can UNLOAD a clip to replace with a better ammo, or a full clip.
~

0 EQUIPMENT INVENTORY GEAR~
Syntax: equipment
Syntax: inventory

EQUIPMENT lists your equipment (armor, weapons, and held items).
INVENTORY lists your inventory.
GEAR is a way cooler word for EQUIPMENT.
~

-1 FLEE RETREAT~

If ye are attacked either kill the bastard or run like hell!
(i.e. there is no flee command)
~

0 EXAMINE LOOK~
Syntax: look
Syntax: look    <object>
Syntax: look    <character>
Syntax: look    <direction>
Syntax: look    <keyword>
Syntax: look in <weapon>
Syntax: examine <weapon>

LOOK looks at something and sees what you can see.

EXAMINE is a combined 'LOOK weapon' and  'LOOK IN weapon'.
~

0 REST SLEEP STAND WAKE~
Syntax: rest
Syntax: sleep
Syntax: stand
Syntax: wake

These commands change your position.  When you REST or SLEEP, you 
regenerate hit points.
However, you are weak as a baby when you REST, and SLEEP is the
closest thing to DEATH!

Use STAND or WAKE to stand at attention.  You can also WAKE other slacking 
(SLEEPING) characters.
~

0 GTELL REPLY SAY TELL~

Syntax: say   <message>
Syntax: tell  <character> <message>

All of these commands send messages to other players.  GTELL is
replaced with team channels (see help on blue and red).

SAY sends a message to all awake players in your room.  The single quote 
is a synonym for SAY.

TELL sends a message to one awake player anywhere in the world.

REPLY sends a message to the last player who sent you a TELL.  REPLY will work
even if you can't see the player, and without revealing their identity.  This
is handy for talking to hidden or switched immortal players.
~

0 NOTE NOTES~
The notes replace the bulletin boards and mail systems found on many
other muds.  The following options are usable:

reading notes:
note list       : show all notes (unread notes are marked with an N)
note read       : either by number, or read next to read unread notes
note remove     : removes a note that you wrote or are the target of

writing new notes:
note to         : sets the to line for a new note (name, immortal, or all)
note subject    : sets the subject for a new note
note +          : adds a line to a new note (i.e. note + hi guys!)
note clear      : erase a note in progress
note show       : shows the note you are working on
note post       : posts a new note. This MUST be done to post a note.

When you log in, you will be informed if there are unread notes waiting. Type
note read to scroll through them one at a time.
~

0 EMOTE SOCIAL~
Syntax :
	emote <action>

EMOTE is used to express emotions or actions.  Besides EMOTE, there are
several dozen built-in social commands, such as CACKLE, HUG, and THANK.
Socials can be added with the sadd command as well.

~

-1 REMOVE WEAR WIELD~
[Updated 8 May 1998]
Syntax :
	remove	<object>
	wear	<object>
	wear	all
	wield	<object>

WEAR is for armor.
WIELD is for weapons.

WEAR ALL will attempt to WEAR or WIELD each suitable item in yer inventory.

REMOVE will take any object from yer equipment and put it back into yer
inventory.
~


-1 MAP BATTLEMAP~
[Updated 8 May 1998]
While ye are in yer base, ye may type :

look map

and it will show ye a map of the city including locations of mobs, the enemy,
tanks, napalm, smoke, etc.
~


0 RADIOS~

Nope these radios don't blast NIN like they should, but they are still 
handy items to grab.  Right now there are two kinds of radios.

air assult radios.  Fun.  "use radio <x-room#> <y-room#> 0" and that 
room, anyone in it, the rooms near it, the people near it, all suddenly 
(oops!) are rained upon buy napalm (makes a great bathe.  Takes all that 
icky dirt and hair right off).  Can only be used or top level (can planes 
fly underground real good?  No.)

emergency evac radio.  In a jam?  Need to split?  Wanna relocate _real_ 
quick?  "use radio <x-room#> <y-room#> <lvl#>" and SHABAM there you are.  

The teleport command only works at certain times.  If you listen, you will 
hear some really nice annoucer man tell you over your comm-badge if the 
trasports are working or not.  If you missed his announcement, try it, he'll 
tell you if you can or not.

~

0 pull bury~

In order to successfully use an explosive you must activate it so that it will
detonate and hopefully kill some moron who is in the room with it.

If the explosive is a type of grenade then you need to type
			"pull <name>"
for example pull grenade will do just fine if you have a grenade in your 
inventory.  After you have pulled the pin you have 10 seconds to get rid of
it before it blows up in your hands (see toss, throw, give, and drop).

If the explosive is a type of mine you need to first drop it so that it
is on the ground then type "bury <obj name>" or "bury" if its the first 
item in teh room.

This will bury the device.  The next person to enter the room will trigger it,
so walk away and try not to go into that square again for a while.

NOTE: Note only "grenade" items are pulled.  So are dynamite, molotov 
cocktails, c4 explozives, etc.  syntax is still "pull <obj name>"
~


0 shoot kill~
Ahhhhhhh . . . so you want to kill someone eh?
What a surprise!

this game works on a basis of seek and destroy for killing people.  If a
player is in the game you just need to type
			"kill <whoever>"
				or
			"shoot <whoever>"
and the game will automatically have you shoot them the next time they are in
range and you have a weapon working correctly (ie wielding one that is loaded)

If you want to be aggro to everyone not on your team, you may type
			"kill all"
this toggles the aggro setting.  The default is off.

If for some strange reason you don't want to have this seek and destroy set
for a particular player, simply name your new target.  If you want to be
non-aggro to absolutely everyone in the game (boggle) then you may type
			"kill"
			  or
			"shoot"
with no name specified.  Wuss only command.
~

-1 THROW TOSS~
[Updated 8 May 1998]

Syntax :
	throw	<obj_name>	<distance_&_direction>
	throw	<obj_name>	<player_name>

This command is used to throw an object up to 2 squares away from yerself.
There is really no practical value to it unless ye are throwing a live
explosive. Thus it is usually used right after the pull command. You are,
however, not limited to only throwing live explosives.
~


0 use~

SYNTAX: use <object_name>
SYNTAX: use <object_name> <argument>
When in doubt as to how to use an item, try the USE command!!!
Works with radios among other things.
~

0 depress~

When you find the BIG RED BUTTON which just so happens to be hooked to the
nuclear device which will destroy everyone in the city and give you and 
your team credit for their deaths, you might get the urge to push it to 
start the countdown sequence.

To do this you type "depress button" and WOW it starts the sequence.  
All you have to do now is stop everyone else from getting to it and 
pushing it themselves.  Don't you hate people who push your buttons?

You tried that you say? It tells you "You lack the keys needed to activate the 
device."? Oh, well, I forgot to mention that you need the keys (type "help
keys" for more info). Grin...
~


-1 BUG IDEA TYPO~
[Updated 8 May 1998]

Syntax :
	idea	<message>
	type	<message>

These commands will take yer message and record it into a file as feedback
to the mud implementors.

~

0 CREDITS~
Presumably someone deserves credit for this game.  List them here.

Credits for GZBase can be found in help GZBase.
Credits for Diku can be found in help Diku.
~

0 GZCREDITS~
The responsible parties for turning ROM into Ground Zero which was used as
a base for this game are as follows:

                               Coding Stuff
                               ------------
                            Cory Hilke (`9Randar``)

                  Object/Weapon/Armor/Mobs Design and Creation
                  --------------------------------------------
                            Lee Hughes (`9Tarrant``)
			Grant Radcliff (`9Pernicies``)
			     Brian(`9Jester``)
                                   Matt
                                   Sin
                            Cory Hilke (`9Randar``)
				 Rachael

                                Help Files
                                ----------
                            Cory Hilke (`9Randar``)
			Grant Radcliff (`9Pernicies``)
                                   Matt
			     Brian(`9Jester``)

                               Openning Ansi
                               -------------
                                 Ghestahgt
                                 Insidious
                                  Deviant

Ideas and Premise : Cory Hilke, Luchnivar Robinson, Lee Hughes,
                    Caleb Kemere, Philip Su, Grant Radcliff and Matt.

Original ROM code and its predicessors' credits can be found in help Diku.
~


-1 COMMANDS REPORT SCORE TIME~
[Updated 8 May 1998]

Syntax :
	commands
	report
	score
	time

COMMANDS shows ye all the commands in the game.

REPORT announces yer current statistics to anyone who is in the same room
as yerself.

SCORE tells ye a little about yourself (hours played, armor absorption,
etc.)

TIME shows the time this game was last started and the current system
time.
~


0 WHO~
Syntax : who

WHO shows the people currently in the game, and what team they're on.

The "players that might give you some trouble" are the characters who are 
connected to the game.  The "players that won't know what hit them" are those 
that have lost link.
You are STRONGLY encouraged to kill the LD ones cuz they just take up space
and their equipment could be put to much better use than sitting on them when
they are link dead.

"who immortal" lists all immortals playing
"who blue" and who red list the characters on the red and blue teams.
"who hunter" lists players who are on neither (immortals and bounty hunters).
~


-1 WHERE~
[Updated 8 May 1998]

Syntax :
	where
	where	<character>

WHERE without an argument tells ye the location of visible players in the same
area as ye are.

WHERE with an argument tells ye the location of one character with
that name anywhere in the world.
~


-1 HELP~
[Updated 8 May 1998]

Syntax :
	help
	help	<keyword>

HELP without any arguments shows a one-page command summary.

HELP <keyword> shows a page of help on that keyword.
~


-1 DESCRIPTION~
[Updated 8 May 1998]

If anyone is close enough to look at ye -- WASTE 'EM!
~


-1 PASSWORD~
[Updated 8 May 1998]

Syntax :
	password	<old-password>	<new-password>

PASSWORD changes the password for yer character. The first argument must be
yer old password. The second argument is yer new password.

The PASSWORD command is protected against being snooped or logged.
~


-1 TITLE~
[Updated 8 May 1998]

Syntax :
	title	<string>

The game supplies a default title when yer character is created. Ye can
use TITLE to set your title to pretty much anything of yer choosing.
~


-1 WIMPY~
[Updated 8 May 1998]

************ Only the strong survive ***********
************* There can be only one ************
~


-1 OPEN CLOSE LOCK UNLOCK~
[Updated 8 May 1998]

Syntax :
	open	<object|direction>
	close	<object|direction>
	lock	<object|direction>
	unlock	<object|direction>

OPEN and CLOSE : open and close a door.

LOCK and UNLOCK : lock and unlock a closed object or door.
Most doors can be locked on either or both sides.
~


0 greeting1~

put ansi greeting 1 here.

~

0 greeting2a~

ansi greeting 2 is
~

0 greeting2b~
in 2 parts to cut down on lag.

~


-1 motd disclaimer~

here is the motd

Add a disclaimer here if your game is violent like GZ was.

<Hit return to enter the game.>
~


-1 REGISTER ACCOUNT~
[Updated 17 May 1998]

To help us keep a track of people playing around here, we would like ye to
register yerself under an account. Ye can play without registering, but
then ye are prevented from using explosives, writing notes, and various
other things that make this mud great. The account system will not be ever
used as a mass mailing.

To register, send email to to :	`1someone@somwhere.net``

With the subject heading :	`1account request``

And included in the body of the mail there should be only one line of
text :				`1newaccount <yer full email> <yer password>``

For example, my email is chewy@owlnet.rice.edu and if i wanted my password
to be 12345, i would have :	`1newaccount chewy@owlnet.rice.edu 12345``

The above set up for account registration is so that registering yer
account can be done efficiently. If ye dont think the above instructions
are clear complain to me via email. If ye fail to follow the above
instructions to the letter, tough.

When ye receive a reply email that says :	`1account added.``

Log into this game, and when it asks for yer account name, then ye type
in :						`1<yer full email>``
And yer account password is then whatever ye wrote down in yer account
request email.

Thank you for your support.

~


-1 storyline~

You have some story for this game don't you??

~


-1 admin god immortal enforcer~
Talk about etiquite toward imms in here . . .

The enforcer character is a mob that has limitted trust.  Certain
players on the game are able to become this mob in order to stop a
spammer or otherwise penalize someone who is doing something illegal.

If you feel that the enforcer or an administrator is doing something
they shouldn't or something unfair, you may write a note to someimp
explaining what occurred and what you feel was unjust/unfair/illegal
about it.

Many people want to know what the procedure is for deciding on new
admins.  This is coverred in help new_admins.
The rules that the administration enforces can be found in help rules.

~


-1 new_admins~
How do people become imms here?

~

-1 premise~
Is there some premise that is unique here (hopefully)?
~

-1 imotd~
Messages just to imms.
<You know the drill.  Hit return.>
~

-1 follow~
Syntax: follow <argument>

To follow someone type follow and then their name.
Follow by itself turns on and off whether people can follow you.
Your default follow state is off.
Even when on, only teammates can follow you.
~

0 $~


#$
