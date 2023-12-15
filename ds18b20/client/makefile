all: main 
		rm -rf *.o
main: main.o socket.o get_time.o get_temp.o my_sqlite3.o logger.o pack_data.o 
		gcc main.o socket.o get_time.o get_temp.o my_sqlite3.o logger.o pack_data.o -L ~/moweida/sqlite3/lib/ -lsqlite3 -o main
main.o: src/main.c
		gcc -c src/main.c -I inc/
socket.o: src/socket.c
		gcc -c src/socket.c -I inc/
get_time.o: src/get_time.c
		gcc -c src/get_time.c -I inc/ 
get_temp.o: src/get_temp.c
		gcc -c src/get_temp.c -I inc/ 
my_sqlite3.o: src/my_sqlite3.c
		gcc -c src/my_sqlite3.c -I inc/ -I ~/moweida/sqlite3/include/  
logger.o: src/logger.c
		gcc -c src/logger.c -I inc/
pack_data.o: src/pack_data.c
		gcc -c src/pack_data.c -I inc/
clean:
	rm *.log *.db 



