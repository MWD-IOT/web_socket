clean: main
	rm -rf *.o
main: socket_server_epoll.o my_sqlite3.o logger.o pack_data.o
	gcc socket_server_epoll.o my_sqlite3.o pack_data.o logger.o -lsqlite3 -o main
socket_server_epoll.o:src/socket_server_epoll.c 
	gcc -c src/socket_server_epoll.c -I inc/ -lsqlite3
my_slqite3.o:src/my_sqlite3.c
	gcc -c src/my_sqlite3.c -I inc/
pack_data.o:src/pack_data.c
	gcc -c src/pack_data.c -I inc/
logger.o:src/logger.c
	gcc -c src/logger.c -I inc/
run: 
	./main -p 12345
