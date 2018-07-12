
#include <stdlib.h> //system() call
#include <iomanip> //setw
#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <errno.h>
#include <unistd.h>

#include "NetworkRequestChannel.H""
#include "Bounded_buffer.H"


/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/
int n_requests=10000; //program arguments and defaults
int w_threads=30;
int buff_size=500;
string HostName="localHost";
int port=10250;

int joe_counter=0; //request count
int jane_counter=0;
int john_counter=0;

vector<int> joe_histogram(100); //histogram
vector<int> jane_histogram(100);//100 spots for each possible number
vector<int> john_histogram(100);

Bounded_buffer* buffer; // basic bounded buffer to hold requests
Bounded_buffer* joe_buf; //statistics buffers
Bounded_buffer* jane_buf;//delay new operator untill complete program arguments
Bounded_buffer* john_buf;

vector<NetworkRequestChannel*> NetworkChannels;
int* person_id;

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

int* joe=new int(0);//pointers to ints for pthread_create arguments
int* jane=new int(1);//allows specification of thread ownership
int* john=new int(2);

/*--------------------------------------------------------------------------*/
/* Helper and thread functions */
/*--------------------------------------------------------------------------*/

void print_histgram(vector<int> data, string name)
{
    vector<int> smaller(10); //make the histogram have less bars ie 0-9 10-19 20-29 etc
    for (int j=0; j<smaller.size(); ++j)
    {
        for (int k=0; k<smaller.size(); ++k)
        {
            smaller[j]+=data[k+j*10];
        }
    }
    cout<<"\n    "<<name<<endl
        <<setw(7)<<"0-9"<<setw(7)<<"10-19"<<setw(7)<<"20-29"<<setw(7)<<"30-39"<<setw(7)<<"40-49"<<setw(7)<<"50-59"
        <<setw(7)<<"60-69"<<setw(7)<<"70-79"<<setw(7)<<"80-89"<<setw(7)<<"90-99"<<endl<<endl;

    for (int i=0; i<10; ++i)
    {
        cout<<setw(7)<<smaller[i];
    }
    cout<<endl<<endl;


}


void* request_thread(void* person_id)
{
    int req_id = *((int*)person_id);
    //    cout<<"working (1) "<< req_id<<endl;
    for(int i=0; i<n_requests; i++)
    {
        //  cout<<"working (2) "<< req_id<<endl;
        Response* r = new Response("dummy",req_id,0);
        // cout<<"working (3) "<< req_id<<endl;;
        if(req_id==0)
        {
            joe_counter++;
            r->str = "data Joe Smith";
            r->request_id = 0;
            r->count = joe_counter;
        }
        else if (req_id==1)
        {
            jane_counter++;
            r->str = "data Jane Smith";
            r->request_id = 1;
            r->count = jane_counter;

        }
        else if (req_id==2)
        {
            john_counter++;
            r->str = "data John Smith";
            r->request_id = 2;
            r->count = john_counter;
        }
        buffer->add_response(*r);
        delete r;
    }
    cout << "All requests completed for request_id="<< req_id<< endl;
    cout<<"Request thread exiting\n";
}

void* event_thread(void* c)
{


    person_id=new int[w_threads];

    fd_set readset; //read file descriptors
    int max = 0;
    int select_result;

    Response r= Response("", 0,0);
    bool finished = false;
    int wcount = 0;
    int rcount = 0;
    struct timeval te = {0,10};


    for(int i=0; i<w_threads; i++)
    {
        NetworkRequestChannel* channel = new NetworkRequestChannel(HostName, port);
        NetworkChannels.push_back(channel);
        person_id[i] = -1;//initialize to a - number so that it can not be mistaken for a real request
    }


    // fill all the channels with requests before trying to read from them
    for(int i=0; i<w_threads; i++)
    {
        r = buffer->retrieve_response();
        wcount++;
        NetworkChannels[i]->cwrite(r.str);
        person_id[i] = r.request_id;
    }



    while(!finished)
    {
        FD_ZERO(&readset);
        for(int i=0; i<w_threads; i++)
        {
            if(NetworkChannels[i]->read_fd() > max)
            {
                max = NetworkChannels[i]->read_fd();
            }
            FD_SET(NetworkChannels[i]->read_fd(), &readset);
        }

        select_result = select(max+1, &readset, NULL, NULL, &te);

        if(select_result)
        {
            for(int i=0; i<w_threads; i++)
            {

                if(FD_ISSET(NetworkChannels[i]->read_fd(), &readset))
                {
                    // cout << "reading\n";
                    string serv_resp = NetworkChannels[i]->cread();
                    rcount++;
                    switch(person_id[i])
                    {
                    case 0:
                        joe_buf->add_response(Response(serv_resp, 0, 0));
                        break;
                    case 1:
                        jane_buf->add_response(Response(serv_resp, 1, 0));
                        break;
                    case 2:
                        john_buf->add_response(Response(serv_resp, 2, 0));
                        break;
                    }

                    if(wcount < n_requests*3)//dont write more than is available might cause deadlock
                    {
                        r = buffer->retrieve_response();
                        wcount++;
                        // cout << "writing\n";
                        NetworkChannels[i]->cwrite(r.str);
                        person_id[i] = r.request_id;
                    }
                }
            }
        }


        if(rcount == n_requests*3) //if all reads complete break
        {
            break;
        }

    }

    // close request channels
    for(int i=0; i<w_threads; i++)
    {
        NetworkChannels[i]->cwrite("quit");
    }
}
/* not used, replaced by event handler
void* worker_thread(void* req_channel)
{

RequestChannel * channel = (RequestChannel*) req_channel;
Response r("",0,0);
int counter=0;
while(1)
{

r=buffer->retrieve_response(); // consume buffer
// communicate with data server

if(r.str == "kill")
{
break; // exit
}

string reply = channel->send_request(r.str);
r.str = reply;
if (r.request_id==0)
joe_buf->add_response(r);
else if (r.request_id==1)
jane_buf->add_response(r);
else if (r.request_id==2)
john_buf->add_response(r);

}
channel->send_request("quit");

} */

void* stat_thread(void* person_id)
{
    int req_id = *((int*)person_id);

    Response r("dummy",-1,-1);

    for(int i=0; i<n_requests; i++)
    {
        if (req_id==0)
        {
            r = joe_buf->retrieve_response();
            joe_histogram[atoi(r.str.c_str())]+=1; //increment corresponding integer of the histogram
        }
        else if (req_id ==1)
        {
            r = jane_buf->retrieve_response();
            jane_histogram[atoi(r.str.c_str())]+=1;
        }
        else if (req_id==2)
        {
            r = john_buf->retrieve_response();
            john_histogram[atoi(r.str.c_str())]+=1;

        }
    }
    cout << "stat thread finished " << req_id<< endl;
}


/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[])
{
    string temp;
    int c=0;
    while((c = getopt (argc, argv, "n:w:b:"))!=-1 )
        switch(c)
        {
        case 'n':
            n_requests=atoi(optarg);
            break;
        case 'w':
            w_threads=atoi(optarg);
            break;
        case 'b':
            buff_size=atoi(optarg);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'h':
            temp = optarg;
            if(temp != "")
                HostName = temp;
            break;
        case '?':
            cout<<"Unknown option aborting\n\n";
            abort();
        default:
            cout<<"cmd line error\n";
            cout<<"options -n=# -w=# -b=#\n\n";
            abort();
        }
    // have arguments now initialize
    pthread_t request_threads[3];// one for each person 0=joe 1=jane 2=john
    pthread_t event_handler;
    // pthread_t worker_threads[w_threads]; //removed for event handler
    pthread_t stat_threads[3];

    buffer= new Bounded_buffer(buff_size);
    joe_buf= new Bounded_buffer(buff_size);
    jane_buf= new Bounded_buffer(buff_size);
    john_buf= new Bounded_buffer(buff_size);


    // pid_t server; //start server
    //  if (server=fork()==0)
    //     execl("./dataserver",NULL,NULL);
    // else{


    cout << "CLIENT STARTED:" << endl;


    /*  cout << "
    Establishing control channel... " << flush;
    RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
    cout << "done." << endl; */

    timeval begin, end;
    gettimeofday(&begin, NULL);

    cout<<"Creating request threads\n";
    pthread_create(&request_threads[0], NULL, request_thread, (void*)joe);
    pthread_create(&request_threads[1], NULL, request_thread, (void*)jane);
    pthread_create(&request_threads[2], NULL, request_thread, (void*)john);



    cout << "Creating event handler\n";
    pthread_create(&event_handler, NULL, event_thread, NULL);

    /*  cout << "Creating worker threads\n";   //removed for Event handler update
    for(int i=0; i<w_threads; i++) {
    // cout<<i<<endl;
    string reply = chan.send_request("newthread");
    RequestChannel* channel = new RequestChannel(reply,
    RequestChannel::CLIENT_SIDE);
    pthread_create(&worker_threads[i], NULL, worker_thread, channel);
    } */



    cout << "Creating stat threads\n";
    pthread_create(&stat_threads[0], NULL, stat_thread, (void*)joe);
    pthread_create(&stat_threads[1], NULL, stat_thread, (void*)jane);
    pthread_create(&stat_threads[2], NULL, stat_thread, (void*)john);



    cout<<"waiting for threads to finish\n";
    pthread_join(request_threads[0],NULL);//wait for request threads to finish
    pthread_join(request_threads[1], NULL);
    pthread_join(request_threads[2], NULL);

    /*Response k("kill", -5, -5); //load buffer with kill requests
    cout<<"killing workers."<< endl;
    for (int i=0; i<w_threads;++i)
    buffer->add_response(k);
    for (int i=0; i<w_threads;++i) //wait for worker threads to exit
    pthread_join(worker_threads[i], NULL); */

    pthread_join(event_handler,NULL); //wait for event handler to finish

    pthread_join(stat_threads[0], NULL); // wait for stat threads to exit
    pthread_join(stat_threads[1], NULL);
    pthread_join(stat_threads[2], NULL);

    gettimeofday(&end,NULL);

    sleep(1); //wait for server prints to close
    print_histgram(joe_histogram, "Joe Smith");
    print_histgram(jane_histogram, "Jane Smith");
    print_histgram(john_histogram, "John Smith");
    cout<<"Total number of requests: "<<n_requests*3<<endl;
    cout<<"Total number of worker threads: "<<w_threads<<endl;
    cout<<"Total request time: "<<end.tv_sec-begin.tv_sec<<" sec "<<end.tv_usec-begin.tv_usec<<" musec"<<endl;

    //Clean Up
    for (int i=0;i<NetworkChannels.size();++i)
        delete NetworkChannels[i];
    return 0;
}


