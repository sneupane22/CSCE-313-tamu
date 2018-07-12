
#include <pthread.h>
#include <queue>
#include <string>
#include "semaphore.H"

using namespace std;

class Response{
  public:

    string str;      //data server response
    int request_id;  //client request
    int count;       //request number


    Response(string s, int r, int c) {
      str = s;
      request_id = r;
      count=c;
    }
};

class Bounded_buffer {
  private:
    int size;
    Semaphore * lock;
    Semaphore * full;
    Semaphore * empty;
    queue<Response> q;

  public:
   Bounded_buffer(int _size) {
        size = _size;
        lock = new Semaphore(1);
        full = new Semaphore(0);
        empty = new Semaphore(size);
}

~Bounded_buffer() {
  delete lock; //delete operator to offset new operator in the constructor
  delete full;
  delete empty;
}

void add_response(Response _response) {

  empty->P();                  //P() decreases
  lock->P();                   // Crit Section
  q.push(_response);
  lock->V();                   // V() increases
  full->V();
}


Response retrieve_response() {
  full->P();
  lock->P();                   //Crit Section
  Response r=q.front();
  q.pop();
  lock->V();
  empty->V();
  return r;
}

};
