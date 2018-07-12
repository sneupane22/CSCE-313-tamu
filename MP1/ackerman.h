
#ifndef _ackerman_h_                              /* include file only once */
#define _ackerman_h_

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
/* MODULE ackerman */
/*--------------------------------------------------------------------------*/

extern void ackerman_main();
/* Asks user for parameters n and m and computes the result of the
   (highly recursive!) ackerman function. During every recursion step,
   it allocates and de-allocates a portion of memory with the use of the
   memory allocator defined in module "my_allocator.H".
*/ 

#endif
