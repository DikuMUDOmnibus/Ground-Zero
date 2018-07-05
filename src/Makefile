CC      = gcc
PROF    = 
NOCRYPT =
C_FLAGS = -O -w -pipe $(PROF) $(NOCRYPT)
L_FLAGS = -O $(PROF)

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o comm.o \
          db.o handler.o interp.o save.o fight.o\
          update.o memory.o

all: ground0 parent

ground0: $(O_FILES) parent
	rm -f ground0
	$(CC) $(L_FLAGS) -lnsl -lsocket -o ground0 $(O_FILES)

parent: parent.c
	$(CC) -o parent -g $(C_FLAGS) parent.c

act_comm.o: act_comm.c ground0.h
	$(CC) -c -g $(C_FLAGS) act_comm.c

act_info.o: act_info.c ground0.h
	$(CC) -c -g $(C_FLAGS) act_info.c

act_move.o: act_move.c ground0.h
	$(CC) -c -g $(C_FLAGS) act_move.c

act_obj.o: act_obj.c ground0.h
	$(CC) -c -g $(C_FLAGS) act_obj.c

act_wiz.o: act_wiz.c ground0.h
	$(CC) -c -g $(C_FLAGS) act_wiz.c

comm.o: comm.c ground0.h
	$(CC) -c -g $(C_FLAGS) comm.c

db.o: db.c ground0.h db.h memory.h
	$(CC) -c -g $(C_FLAGS) db.c

db2.o: db2.c ground0.h
	$(CC) -c -g $(C_FLAGS) db2.c

handler.o: handler.c ground0.h
	$(CC) -c -g $(C_FLAGS) handler.c

interp.o: interp.c ground0.h interp.h
	$(CC) -c -g $(C_FLAGS) interp.c

save.o: save.c ground0.h
	$(CC) -c -g $(C_FLAGS) save.c

fight.o: fight.c ground0.h
	$(CC) -c -g $(C_FLAGS) fight.c

update.o: update.c ground0.h
	$(CC) -c -g $(C_FLAGS) update.c

memory.o: memory.c ground0.h memory.h
	$(CC) -c -g $(C_FLAGS) memory.c
