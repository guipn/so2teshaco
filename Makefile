C_FLAGS = -Wall -Wextra

debug:	
	rm -rf *.o
	gcc $(C_FLAGS) -D debug *.c -o shell
	perl cdoc.pl
	rm -rf *~

deploy:	
	rm -rf *.o
	gcc $(C_FLAGS) *.c -o shell
	perl cdoc.pl
	rm -rf *~

compress:	
	    tar -cf shell.tar main.c memory.h memory.c str.h str.c builtincmd.h builtincmd.h sig.h sig.c jobs.c jobs.h io.h io.c
	    rm *.o

compress2:	
	    tar -cf shell.tar *
	    rm *.o
