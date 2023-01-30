
#Use GNU compiler
CC = g++ -g

all: lab4 

lab4.o: lab4.cc
	$(CC) -c lab4.cc

lab4: lab4.o
	$(CC) -o lab4 lab4.o	

clean:
	rm -f lab4 *.o

