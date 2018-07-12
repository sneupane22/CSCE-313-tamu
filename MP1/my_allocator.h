

#ifndef _my_allocator_h_                   // include file only once
#define _my_allocator_h_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/
typedef struct Header Header;

    struct Header{
    char symbol; //use for validation and determine if free
    int size;
    Header* next;
    };


typedef void * Addr;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* MODULE   MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/
int check_header(Header* ptr);
void test_list();

unsigned int init_allocator(unsigned int _basic_block_size,
			    unsigned int _length);
/* This function initializes the memory allocator and makes a portion of
   ’_length’ bytes available. The allocator uses a ’_basic_block_size’ as
   its minimal unit of allocation. The function returns the amount of
   memory made available to the allocator. If an error occurred,
   it returns 0.
*/

int release_allocator();
/* This function returns any allocated memory to the operating system.
   After this function is called, any allocation fails.
*/

Addr my_malloc(unsigned int _length);
/* Allocate _length number of bytes of free memory and returns the
   address of the allocated portion. Returns 0 when out of memory. */

int my_free(Addr _a);
/* Frees the section of physical memory previously allocated
   using ’my_malloc’. Returns 0 if everything ok. */

#endif
