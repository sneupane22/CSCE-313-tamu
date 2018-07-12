
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <string.h>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include "NetworkRequestChannel.H"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

	struct sockaddr_in serverIn;





int createClientConnection(const char * host, const char * portNo)
{
	struct sockaddr_in sockIn;
	memset(&sockIn, 0, sizeof(sockIn));
	sockIn.sin_family = AF_INET;

	if(struct servent * pse = getservbyname(portNo, "tcp"))//make port
		sockIn.sin_port = pse->s_port;

	else if ((sockIn.sin_port = htons((unsigned short)atoi(portNo))) == 0)
		cerr << "cant connect port\n";

	if(struct hostent * hn = gethostbyname(host))
		memcpy(&sockIn.sin_addr, hn->h_addr, hn->h_length);

	else if((sockIn.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
		cerr << "cant determine host <" << host << ">\n";

	int s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0)
        cerr << "cant create socket\n";

	if(connect(s, (struct sockaddr *)&sockIn, sizeof(sockIn)) < 0)
		cerr << "cant connect to " << host << ":" << portNo;

	return s;
}

int createServerConnection(const char * svc, int backlog)
{

	memset(&serverIn, 0, sizeof(serverIn));
	serverIn.sin_family = AF_INET;
	serverIn.sin_addr.s_addr = INADDR_ANY;

	if(struct servent * pse = getservbyname(svc, "tcp"))
		serverIn.sin_port = pse->s_port;

	else if((serverIn.sin_port = htons((unsigned short)atoi(svc))) == 0)
		cerr << "can't get port\n";


	int socknum  = socket(AF_INET, SOCK_STREAM, 0);

	if(socknum < 0)
		cerr << "can't create socket \n";

	if(bind(socknum, (struct sockaddr *)&serverIn, sizeof(serverIn)) < 0)
		cerr << "can't bind...\n";

	listen(socknum, backlog);//check for connections

	return socknum;
}


//Client
NetworkRequestChannel::NetworkRequestChannel(const string _server_host_name, const unsigned short _port_no)
{
	stringstream ss;
	ss << _port_no;
	string port = ss.str();

	fd = createClientConnection(_server_host_name.c_str(), port.c_str());

}
//Server
NetworkRequestChannel::NetworkRequestChannel(const unsigned short _port_no, void * (*connection_handler) (void *), int backlog)
{

	stringstream ss;
	ss << _port_no;
	string port = ss.str();

	int master = createServerConnection(port.c_str(), backlog);
	int serverSize = sizeof(serverIn);


	while(true)
	{
		int * slave = new int;

		pthread_t thread;
		pthread_attr_t attr;
		pthread_attr_init(&attr);


		*slave = accept(master,(struct sockaddr*)&serverIn, (socklen_t*)&serverSize);

		if(slave < 0)
		{
			delete slave;

			if(errno == EINTR)
                continue;//retry
			else cerr << "unknown error in accept()\n";
		}

		pthread_create(&thread, &attr, connection_handler, (void*)slave);


	}
	cout << "Connection complete\n";
}

NetworkRequestChannel::~NetworkRequestChannel()
{
	close(fd); // Closes the socket
}

/*--------------------------------------------------------------------------*/
/* NetworkRequestChannel Functions */
/*--------------------------------------------------------------------------*/
const int MAX_MSG = 255;

int NetworkRequestChannel::read_fd()
{
	return fd;
}



string NetworkRequestChannel::cread()
{
	char buf[MAX_MSG];

	if (read(fd, buf, MAX_MSG) < 0)
		cerr<<"Error reading\n";


	string s = buf;

	return s;
}

int NetworkRequestChannel::cwrite(string _msg)
{
	if (_msg.length() >= MAX_MSG) {
		cerr << "Message too long for Channel!\n";
		return -1;
	}

	const char * s = _msg.c_str();

	if (write(fd, s, strlen(s)+1) < 0)
		cerr<<"Error writing\n";


}

