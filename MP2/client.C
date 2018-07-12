
#include <stdlib.h>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>


#include <errno.h>
#include <unistd.h>

#include "reqchannel.H"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

string int2string(int number)
{
    stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

string process_request(const string & _request) {

  if (_request.compare(0, 5, "hello") == 0) {
    return "hello to you too";
  }
  else if (_request.compare(0, 4, "data") == 0) {
    return (int2string(rand() % 100));
  }
  else {
    return "unknown request";
  }

}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[])
{
    pid_t cpid;
    if (cpid=fork()==0) //create a new child process do things with it
        system("./dataserver");//run dataserver using system commands
    else
    {
        cout << "CLIENT STARTED:" << endl;

        cout << "Establishing control channel... " << flush;
        RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
        cout << "done." << endl;

        /* -- Start sending a sequence of requests */

        timeval begin, end;
        gettimeofday(&begin,NULL); //server test
        string reply1 = chan.send_request("hello");
        gettimeofday(&end,NULL);

        timeval begin2, end2; //function test
        gettimeofday(&begin2,NULL);
        string temp= process_request("hello");
        gettimeofday(&end2,NULL);



        cout << "Reply to request 'hello' is '" << reply1 << "'" << endl;

        timeval begin3, begin4,end3, end4;


        gettimeofday(&begin3,NULL);
        string reply2 = chan.send_request("data Joe Smith"); //server test
         gettimeofday(&end3,NULL);

        gettimeofday(&begin4,NULL);
         string temp2= process_request("data Joe Smith"); //function test
         gettimeofday(&end4,NULL);

        cout << "Reply to request 'data Joe Smith' is '" << reply2 << "'" << endl;

        string reply3 = chan.send_request("data Jane Smith");
        cout << "Reply to request 'data Jane Smith' is '" << reply3 << "'" << endl;

        for(int i = 0; i < 100; i++)
        {
            string request_string("data TestRandom" + int2string(i));
            string reply_string = chan.send_request(request_string);
            cout << "reply to request " << i << ":" << reply_string << endl;;
        }

        string reply4 = chan.send_request("quit");
        cout << "Reply to request 'quit' is '" << reply4 << endl;

        cout<<"\n Server \"hello\" case seconds "<<end.tv_sec-begin.tv_sec<<" musec "<<end.tv_usec-begin.tv_usec<<endl;

        cout<<"\n Function \"hello\" case seconds "<<end2.tv_sec-begin2.tv_sec<<" musec "<<end2.tv_usec-begin2.tv_usec<<endl;

        cout<<"\n Server \"data Joe Smith\" case seconds "<<end3.tv_sec-begin3.tv_sec<<" musec "<<end3.tv_usec-begin3.tv_usec<<endl;

        cout<<"\n Function \"data Joe Smith\" case seconds "<<end4.tv_sec-begin4.tv_sec<<" musec "<<end4.tv_usec-begin4.tv_usec<<endl;


        usleep(100000);
        return 0;
    }
}
