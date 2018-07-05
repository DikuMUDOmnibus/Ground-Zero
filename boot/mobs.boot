#MOBS
]#define BEHAVIOR_LD 0
]#define BEHAVIOR_GUARD 1
]#define BEHAVIOR_SEEK 2
]#define BEHAVIOR_WANDER 3
]#define BEHAVIOR_PILLBOX 4
]#define PLR_AGGRO_ALL           (L)
]#define PLR_AGGRO_OTHER         (M)
]#define I			256
]#define J			512
]#define K		        1024
]#define L		 	2048
]#define M			4096
]#define TEAM_SECURITY                  0
]#define TEAM_NONE                      1
]#define TEAM_RED                       2
]#define TEAM_BLUE                      3
]#define TEAM_GREEN		       4
]#define TEAM_YELLOW		       5
START
guardian~
A GUARDIAN~
HP 40000-70000
SPEED_DELAY 0
BEHAV 1
ACT 4096
SEX 1
LOC "god-store"~
ITEMS 27 29 29 0

droid~
a droid~
HP 5000-5000
SPEED_DELAY 0
BEHAV 4
ACT 4096
SEX 0
LOC "god-store"~
ITEMS 34 35 0

tank~
a tank~
HP 29000-29000
SPEED_DELAY 0
BEHAV 6
ACT 4096
SEX 0
LOC "god-store"~
ITEMS 41 4 0

Elite Button Guard soldier~
An`` `3Elite `1Button`` `3Clan`` Soldier~
HP 5000-10000
SPEED_DELAY 0
BEHAVE 1
ACT 4096
SEX 1
LOC obj bigred~
ITEMS 8 9 12 15 10 26 26 26 0

mobile security droid msd~
A Mobile Security Droid~
HP 15000-20000
SPEED_DELAY 4
BEHAV 3
ACT 4096
SEX 0
LOC random 1~
ITEMS 22 23 0

postal worker~
a `9disgruntled `1US `@Postal Worker``~
HP 8000-20000
SPEED_DELAY 6
BEHAVE 2
ACT 4096
SEX 1
LOC random 0~
ITEMS 16 17 18 19 19 19 0

medic combat~
a `1Red Cross `8Combat Medic``~
HP 3000-5000
SPEED_DELAY 4
BEHAVE 3
ACT 0
SEX 1
LOC random 0~
ITEMS 20 21 21 21 3 2 0

operator radio~
a radio operator~
HP 4000-6000
SPEED_DELAY 4
BEHAVE 3
ACT 0
SEX 1
LOC random 0~
ITEMS 25 25 25 3 2 2 0

enforcer~
the enforcer~
HP 1000000-1000000
SPEED_DELAY 4
BEHAVE 0
ACT 0
SEX 1
LOC god-gen~
ITEMS 0

$~

#$
