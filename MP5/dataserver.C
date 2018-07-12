

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <string.h>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include "NetworkRequestChannel.H"

using namespace std;

static int nthreads = 0;
int MAX_MSG = 255;



/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* FUNCTIONS */
/*--------------------------------------------------------------------------*/

string int2string(int n)
{
   stringstream ss;
   ss << n;
   return ss.str();
}

string server_read(int * fd){
	char buf[MAX_MSG];

	read(*fd, buf, MAX_MSG);
	string s = buf;
	return s;
}

int server_write(int * fd, string m){
	if(m.length() >= MAX_MSG)
		cerr << "Message too lbig\n";

	if(write(*fd, m.c_str(), m.length()+1) < 0)
		cerr << "Write Error\n";

}


void process_hello(int * fd, const string & _request)
{
	server_write(fd, "hello to you too");
}

void process_data(int * fd, const string &  _request)
{
	usleep(1000 + (rand() % 5000));

	server_write(fd, int2string(rand() % 100));
}

void process_request(int * fd, const string & _request)
{

  if (_request.compare(0, 5, "hello") == 0)
    process_hello(fd, _request);

  else if (_request.compare(0, 4, "data") == 0)
    process_data(fd, _request);


}

void * connection_handler(void * arg)
{
	int * fd = (int*)arg;

	if(fd == NULL)
		cout << "err file descriptor is null\n";
    cout<<"New connection\n";
	while(1)
	{
		string request = server_read(fd);

		if (request.compare("quit") == 0)
		{
			server_write(fd, "bye");
			usleep(8000);
			break;
		}

		process_request(fd, request);
	}
	cout<<"Connection Closed\n";

}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {


	int backLog=100;
	unsigned short portN= 10250;
	unsigned short pN = 0;
	int bLog = 0;

    int c = 0;
	while((c = getopt(argc, argv, "p:b:")) != -1){
		switch(c){
			case 'p':
				pN = atoi(optarg);
				break;
			case 'b':
				bLog = atoi(optarg);
				break;
		}
	}

	if(pN != 0)
		 portN=pN;

	if(bLog != 0)
		backLog = bLog;


	cout << "SERVER STARTED ON PORT: " << portN << endl;

	NetworkRequestChannel server(portN, connection_handler, backLog);

	server.~NetworkRequestChannel();

}

