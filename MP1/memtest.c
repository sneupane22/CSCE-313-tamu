#include <stdlib.h>
#include <cstdlib>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "my_allocator.h"
#include "ackerman.h"

/* memtest [-b <blocksize>] [-s <memsize>]
-b <blocksize> defines the block size, in bytes. Default is 128 bytes.
-s <memsize> defines the size of the memory to be allocated, in bytes.
Default is 512kB.
*/
void show_help(){
        printf("usage is as follows\n");
        printf( "memtest [-b <blocksize>] [-s <memsize>]\n");
        printf("-b <blocksize> defines the block size, in bytes. Default is 128 bytes.\n");
        printf("-s <memsize> defines the size of the memory to be allocated, in bytes.\n");
        printf("Default is 512kB.\n\n");


}


int main(int argc, char ** argv)
{
    int c;
    unsigned int b=128;
    unsigned int m=5242880; //512 kB

    if (argc<3)
        show_help();

    while ((c = getopt(argc, argv, "b:s:")) != -1)
        switch (c)
        {
        case 'b':
              b=atoi(optarg);
            break;
        case 's':
               m=atoi(optarg);
            break;
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt)){
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                show_help();
            }else{
                fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
                show_help();
                        }
                return 1;
        default:
            abort ();
        }

    init_allocator(b,m);
    //test_list();
    ackerman_main();

    atexit((void(*)())release_allocator);//cast into correct function type
}
