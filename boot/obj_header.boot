#OBJ_HEADER
]/* item types */
]#define ITEM_MISC			 0
]#define ITEM_WEAPON                     1
]#define ITEM_ARMOR                      2
]#define ITEM_EXPLOSIVE                  3
]#define ITEM_AMMO			 4
]#define ITEM_TEAM_VEHICLE		 5
]#define ITEM_TEAM_ENTRANCE		 6
]
]/* ammo types */
]#define AMMO_9MM                        0
]#define AMMO_UZI                        1
]#define AMMO_MINIGUN                    2
]#define AMMO_45MM                       3
]#define AMMO_SHOTGUN                    4
]#define AMMO_PULSE_RIFLE                5
]#define AMMO_GREN_LAUNCHER              6
]#define AMMO_SLINGSHOT                  7
]#define AMMO_FLAMETHROWER               8
]#define AMMO_POSTAL     		 9
]#define AMMO_ENERGY_PACK		 10
]#define AMMO_AKA			 11
]#define AMMO_ROCKET_LAUNCHER		 12
]#define AMMO_FLARE			 13
]#define AMMO_TANK_GUN			 14
]
]/* general flags */
]#define GEN_CAN_DEPRESS		(A)
]#define GEN_CAN_BURY			(B)
]#define GEN_CONDITION_MONITOR		(C)
]#define GEN_BURNS_ROOM			(D)
]#define GEN_NO_AMMO_NEEDED		(E)
]#define GEN_CHANCE_EXPLODE_WITH_USE	(F)
]#define GEN_DETECT_MINE		(G)
]#define GEN_ANTI_BLIND			(H)
]#define GEN_OBJECTIVE			(I)
]#define GEN_DARKS_ROOM		       	(J)
]#define GEN_EXTRACT_ON_IMPACT		(K)
]#define GEN_SEE_IN_DARK		(L)
]#define GEN_SEE_FAR		        (M)
]
]/* extract flags */
]#define EXTRACT_EXPLODE_ON_EXTRACT	(A)
]#define EXTRACT_BLIND_ON_EXTRACT	(B)
]#define EXTRACT_BURN_ON_EXTRACT	(C)
]#define EXTRACT_STUN_ON_EXTRACT	(D)
]#define EXTRACT_DARK_ON_EXTRACT	(E)
]
]/* usage flags */
]#define USE_HEAL			(A)
]#define USE_AIRSTRIKE			(C)
]#define USE_EVAC			(D)
]#define USE_MAKE_DROID			(E)
]#define USE_MAKE_SEEKER_DROID		(F)
MAX 206

#OBJECTS
ITEM_TYPE 3
WEAR 1
RANGE 3 
GEN 0
EXTRACT A
a `3grenade``~
DAM_CH 3500 2500 1300
DAM_STR 2000 1000 100
USE 0
explosive grenade~
a `3grenade``~
a `3grenade`` lies here.~
REPOP 100 20-20
CHSH 500 500
 
ITEM_TYPE 4
WEAR 1
AMMO_TYPE 0
AMOUNT_AMMO 15
DAM_CH 200 0 0
DAM_ST 1 0 0
GEN 0
EXTRACT 0
USE 0
ammo 9mm clip~
a clip of standard 9mm rounds~
A 9mm clip lies here.~
REPOP 100 30-30
CHSH 10 10

ITEM_TYPE 1
WEAR 40961
RPS 1
AMMO_TYPE 0
RANGE 4
GEN 0
EXTRACT 0
USE 0
gun 9mm pistol~
a 9mm pistol~
a 9mm pistol lies here~
REPOP 100 15-15
CHSH 6000 6000
 
ITEM_TYPE 4
WEAR 1
AMMO_TYPE 14
AMOUNT_AMMO 1
DAM_CH 3500 0 0
DAM_ST 1 0 0
GEN 0
EXTRACT 0
USE 0
ammo tank~
some tank ammo~
A high impact tank shell lies here.~
REPOP 0
CHSH 1 1

ITEM_TYPE 0
WEAR 0
GEN D
DAM_CH 500 300 0
DAM_ST 100 100 0
EXTRACT 0
USE 0
fire~
a `1fire``~
This room is `1ON FIRE``!!!~
REPOP 0
CHSH 30000 30000

ITEM_TYPE 3
WEAR 0
RANGE 0
GEN 0
EXTRACT AC
~
DAM_CH 1500 1000 0
DAM_ST 100 100 0
BURN 150
USE 0
napalm~
a mass of napalm~
A mass of napalm lies here!~
REPOP 0
CHSH 20 20

ITEM_TYPE 6
WEAR 0
GEN 0
EXTRACT 0
USE 0
~
~
~
REPOP 0
CHSH 30000 30000

ITEM_TYPE 2
WEAR 17
ARMOR_VAL 10
GEN 0
EXTRACT 0
USE 0
armor paint camoflage facepaint cans button~
`` `3Elite `1Button`` `3Clan`` facepaint~
some `1red`` and `3green`` paint lies here.~
REPOP 0
CHSH 1 1


ITEM_TYPE 2
WEAR 9
ARMOR_VAL 50
GEN 0
EXTRACT 0
USE 0
armor combat button~
a`` `3Elite `1Button`` `3Clan`` armor~
some `1red`` and `3green`` combat armor lies here.~
REPOP 0
CHSH 1000 1000

ITEM_TYPE 1
WEAR 40961
RPS 1
AMMO_TYPE 0
RANGE 4
GEN 0
EXTRACT 0
USE 0
automatic pistol~
an automatic pistol~
an odd pistol lies here.~
REPOP 0
CHSH 10 10


ITEM_TYPE 2
WEAR 9
ARMOR_VAL 20
GEN 0
EXTRACT 0
USE 0
tunic scouting button clan~
an`` `3Elite `1Button`` `3Clan`` scouting tunic~
a `1red`` and`` `3green`` tunic lies here.~
REPOP 0
CHSH 10 10


ITEM_TYPE 2
WEAR 5
ARMOR_VAL 4
GEN 0
EXTRACT 0
USE 0
button chain golden~
a `1red button`` on a `!golden chain``~
a golden chain lies here.~
REPOP 0
CHSH 100 100


ITEM_TYPE 2
WEAR 3
ARMOR_VAL 5
GEN 0
EXTRACT 0
USE 0
ring red signet~
a `1RED team`` signet ring~
a signet ring lies in the dust here.~
REPOP 0
CHSH 10000 10000



ITEM_TYPE 2
WEAR 3
ARMOR_VAL 5
GEN 0
EXTRACT 0
USE 0
ring blue signet~
a `5BLUE team`` signet ring~
a signet ring lies in the dust here.~
REPOP 0
CHSH 10000 10000



ITEM_TYPE 2
WEAR 3
ARMOR_VAL 10
GEN 0
EXTRACT 0
USE 0
ring button signet~
an`` `3Elite `1Button`` `3Clan`` signet ring~
a signet ring lies in the dust here.~
REPOP 0
CHSH 10000 10000


ITEM_TYPE 2
WEAR 9
ARMOR_VAL 35
GEN 0
EXTRACT 0
USE 0
postal uniform combat~
a `1US `@Postal `!Combat `@Uniform``~
a `@blue `!combat `@uniform``~
REPOP 0
CHSH 100 100


ITEM_TYPE 2
WEAR 1025
ARMOR_VAL 20
GEN 0
EXTRACT 0
USE 0
bag mail~
a `1US `@Postal Service`` Mail/Ammo bag~
a `@blue`` bag in on the ground here.~
REPOP 0
CHSH 10 10


ITEM_TYPE 1
WEAR 40961
RPS 1
AMMO_TYPE 9
RANGE 4
GEN 0
EXTRACT 0
USE 0
postal assault rifle~
a `1US `@Postal Service`` standard issue assault rifle~
an assault rifle is here.~
REPOP 0
CHSH 100 100

ITEM_TYPE 4
WEAR 1
AMMO_TYPE 9
AMOUNT_AMMO 20
DAM_CH 900 0 0
DAM_ST 10 0 0
GEN 0
EXTRACT 0
USE 0
postal assault rifle rounds~
a clip of `1US `@Postal Service`` assault rifle rounds~
a clip of assault rifle rounds is here.~
REPOP 0
CHSH 100 100



ITEM_TYPE 2
WEAR 9
ARMOR_VAL 22
GEN 0
EXTRACT 0
USE 0
white medical~
a `8white `1red cross`` uniform~
a `8white`` uniform lies here.~
REPOP 0
CHSH 100 100

ITEM_TYPE 0
WEAR 1
GEN 0
EXTRACT 0
USE A
DAM_CH 2000 0 0
DAM_ST 0 0 0
med pack medical medkit kit first~
a `1red cross `8first aid`` kit~
a small `8white`` box with a `1red x`` lies here.~
REPOP 95 6-12
CHSH 5000 5000

ITEM_TYPE 4
WEAR 1
AMMO_TYPE 2
AMOUNT_AMMO 100
DAM_CH 250 0 0
DAM_ST 10 0 0
GEN 0
EXTRACT 0
USE 0
belt 100 ammo~
100 rounds of belted ammo~
a 100 round ammo belt has been left here~
REPOP 90 1-3
CHSH 2000 2000

ITEM_TYPE 1
WEAR 40961
RPS 4
AMMO_TYPE 2
RANGE 4
GEN 0
EXTRACT 0
USE 0
50 gun heavy machinegun~
a .50 calibre machinegun~
a heavy machinegun lies here~
REPOP 80 1-3
CHSH 8000 8000

ITEM_TYPE 0
WEAR 0
GEN J
EXTRACT 0
USE 0
darkness~
pure `9darkness``~
This room is filled with `9darkness``.~
REPOP 0
CHSH 30000 30000

ITEM_TYPE 0
WEAR 1
GEN 0
EXTRACT 0
USE C
radio airstrike airforce~
a radio to your airforce~
a radio lies in the dust here.~
REPOP 75 1-3
CHSH 1000 1000

ITEM_TYPE 4
WEAR 1
AMMO_TYPE 0
AMOUNT_AMMO 30
DAM_CH 450 0 0
DAM_ST 1 0 0
GEN 0
EXTRACT 0
USE 0
ammo button 9mm clip~
button clan 9mm ammo~
a clip of 9mm ammo lies here.~
REPOP 0
CHSH 10 10

ITEM_TYPE 1
WEAR 40961
RPS -3
AMMO_TYPE 6
RANGE 4
GEN 0
EXTRACT 0
USE 0
grenade ares launcher~
an `1ARES``(tm) `3grenade`` launcher~
a powerful looking grenade launcher is here.~
REPOP 0
CHSH 7000 7000

ITEM_TYPE 1
WEAR 40961
RPS -3
AMMO_TYPE 6
RANGE 4
GEN 0
EXTRACT 0
USE 0
cannon panther assault~
a `9panther`` assault cannon~
There is a FUCKIN HUGE cannon here.~
REPOP 5 1-1
CHSH 8000 8000

ITEM_TYPE 4
WEAR 1
AMMO_TYPE 6
AMOUNT_AMMO 4
DAM_CH 2999 2000 500
DAM_ST 0 0 0
GEN 0
EXTRACT A
a mini-grenade~
DAM_CH 2999 2000 500
DAM_ST 0 0 0
USE 0
explosive ammo clip mini-grenages~
a clip of `3mini-grenades``~
a clip of `3mini-grenades`` for a `3grenade`` launcher is here.~
REPOP 85 1-3
CHSH 500 500

ITEM_TYPE 2
WEAR 9
ARMOR_VAL 22
GEN 0
EXTRACT 0
USE 0
tunic red~
a `1red`` scouting tunic~
a `1red`` tunic lies here.~
REPOP 0
CHSH 100 100

ITEM_TYPE 2
WEAR 9
ARMOR_VAL 22
GEN 0
EXTRACT 0
USE 0
tunic blue~
a `@blue`` scouting tunic~
a `@blue`` tunic lies here.~
REPOP 0
CHSH 100 100


ITEM_TYPE 1
WEAR 40961
RPS 1
AMMO_TYPE 10
RANGE 4
GEN 0
EXTRACT 0
USE 0
gun huge cannon electron~
a huge `2elctron cannon``~
a huge cannon sits here, arcing with electricity.~
REPOP 0
CHSH 100 100


ITEM_TYPE 4
WEAR 1
AMMO_TYPE 10
AMOUNT_AMMO 4
DAM_CH 2500 0 0
DAM_ST 4000 0 0
GEN 0
EXTRACT 0
USE 0
cell electron pack energy~
an `@energy pack`` for an electron cannon~
an `@energy pack`` for an electron cannon~
REPOP 10 1-2
CHSH 1000 1000


ITEM_TYPE 1
WEAR 40961
RPS 3
AMMO_TYPE 1
RANGE 4
GEN 0
EXTRACT 0
USE 0
gun 9mm uzi~
a 9mm uzi~
a 9mm uzi lies here~
REPOP 95 2-4
CHSH 6000 6000

ITEM_TYPE 4
WEAR 1
AMMO_TYPE 1
AMOUNT_AMMO 30
DAM_CH 150 0 0
DAM_ST 0 0 0
GEN 0
EXTRACT 0
USE 0
ammo 9mm uzi clip~
a 9mm uzi clip~
a 9mm uzi clip lies here~
REPOP 95 8-10
CHSH 6000 6000

ITEM_TYPE 1
WEAR 40961
RPS -1
AMMO_TYPE 12
RANGE 4
GEN 0
EXTRACT 0
USE 0
rpg rocket propelled grenade~
a rocket propelled grenade~
a `9HUGE`` `1RPG`` lies here.~
REPOP 5 1-1
CHSH 5000 5000

ITEM_TYPE 4
WEAR 1
AMMO_TYPE 12
AMOUNT_AMMO 1
DAM_CH 3200 2500 1000
DAM_ST 0 0 0
GEN 0
EXTRACT 0
USE 0
rocket ammo~
a rocket~
a rocket for an `1RPG`` lies here.~
REPOP 75 1-4
CHSH 8000 8000

ITEM_TYPE 2
WEAR 9
ARMOR_VAL 60
GEN 0
EXTRACT 0
USE 0
mega vest body armor~
a vest `@MEGA`` body armor~
a vest of `@MEGA`` body armor lies here.~
REPOP 5 1-1
CHSH 8000 8000

ITEM_TYPE 2
WEAR 33
ARMOR_VAL 35
GEN 0
EXTRACT 0
USE 0
mega armor leg plates~
some `@MEGA`` armor leg plates~
a pair of `@MEGA`` armor leg plates lie here.~
REPOP 5 1-1
CHSH 8000 8000

ITEM_TYPE 2
WEAR 257
ARMOR_VAL 35
GEN 0
EXTRACT 0
USE 0
mega armor arm plates~
some `@MEGA`` armor arm plates~
a pair of `@MEGA`` armor arm plates lie here.~
REPOP 5 1-1
CHSH 8000 8000

ITEM_TYPE 1
WEAR 40961
RPS -1
AMMO_TYPE 14
RANGE 4
GEN 0
EXTRACT 0
USE 0
tank gun~
a tank gun~
a tank gun lies here.~
REPOP 0
CHSH 6000 6000

$

#$

