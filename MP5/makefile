# makefile

all: dataserver client


NetworkRequestChannel.o: NetworkRequestChannel.H NetworkRequestChannel.C
	g++ -c -g NetworkRequestChannel.C
	
dataserver: dataserver.C NetworkRequestChannel.o
	g++ -g -o dataserver dataserver.C NetworkRequestChannel.o -lsocket -lnsl


client: client.C NetworkRequestChannel.o
	g++ -g -o client client.C NetworkRequestChannel.o -lsocket -lnsl
