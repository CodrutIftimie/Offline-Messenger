all: serverNew.cpp 
	g++ -Wall -lpthread -o server.exec serverNew.cpp -lmysqlclient -I/usr/include/mysql

clean: 
	$(RM) server.exec
