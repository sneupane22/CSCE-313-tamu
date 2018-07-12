

#ifndef _semaphore_H_                   // include file only once
#define _semaphore_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <pthread.h>

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CLASS   S e m a p h o r e  */
/*--------------------------------------------------------------------------*/

class Semaphore {
private:
  /* -- INTERNAL DATA STRUCTURES
     You may need to change them to fit your implementation. */

  int         value;
  pthread_mutex_t m;
  pthread_cond_t  c;

public:

  /* -- CONSTRUCTOR/DESTRUCTOR */

  Semaphore(int _val){

    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&c, NULL);
    value=_val;
  }

  ~Semaphore(){
    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&c);
  }

  /* -- SEMAPHORE OPERATIONS */

  int P(){

  int errno; //errno to check standard library funtion errors
  if (errno=pthread_mutex_lock(&m)!=0)
       return errno;
  while (value<=0) //nasty bug missed the equal, reminder to test early
       if (errno=pthread_cond_wait(&c,&m)!=0)
            return errno;
  --value;
  if (errno=pthread_mutex_unlock(&m)!=0)
       return errno;
  return 0;
  }

  int V(){

  int errno; //errno to check standard library funtion errors
  if (errno=pthread_mutex_lock(&m)!=0)
       return errno;
  ++value;
  if (errno=pthread_cond_broadcast(&c)!=0)
       return errno;
  if (errno=pthread_mutex_unlock(&m)!=0)
       return errno;
    //   cout<<"broadcast sig\n";
  return 0;
  }
};


#endif


