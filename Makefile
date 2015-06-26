CC = gcc
LIBS=-lreadline -lhistory 
shellSim: apue.h shellSim.o
	$(CC) shellSim.o -o shellSim $(LIBS)
shellSim.o:shellSim.c
	$(CC) -c shellSim.c

clean:
	@echo "cleanning project"
	rm -rf *.o shellSim
	@echo "clean completed"

.PHONY:clean
