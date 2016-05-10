CC=gcc

all: user.o
	$(CC) user.c -o user
	sudo chown root user
	sudo chmod u+s user

clean:
	rm *.o user
