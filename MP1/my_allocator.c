
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include "my_allocator.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*


/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/
Header** list_ptr;
double list_size;//double for rounding up
double base;//must be double for correct division
uintptr_t mem_begin;//pointer to beginning of memory block for bit calculations
/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR MODULE MY_ALLOCATOR */

void test_list()  //testing functions
{

    for (int i=0; i<list_size*2; i+=2)
    {
        Header* ptr=list_ptr[i];
        printf("\nFREE LIST:%d\n",i/2);
        int sum=0;
        while (ptr!=NULL)
        {
            int test=ptr->size;
            sum+=test;
            printf("size of the memory block %d\n", test);
            if (ptr==list_ptr[i+1])
                break;
            ptr=ptr->next;
        }
        printf("total memory of list=%d\n",sum);
    }
}
void remove_header(int i)
{
    if (list_ptr[i]!=list_ptr[i+1])//remove from list
        list_ptr[i]=list_ptr[i]->next;
    else
    {
        list_ptr[i]=NULL;
        list_ptr[i+1]=NULL;
    }
}
int check_header(Header* ptr)  //
{

    if (ptr->symbol=='~')  //free
    {
        //  printf("free memory of size %d\n",ptr->size);
        return 1;
    }
    if (ptr->symbol=='!')  //used
    {
        // printf("used memory of size %d\n", ptr->size);
        return 0;
    }
    return -1;


}

int power(int n, int p)  //simple recursive exponent function
{
    if (p==0)
        return 1;
    else
        n=n*power(n,p-1);
    return n;
}

Header* block_break(int i) //break blocks
{
    unsigned int break_size=base*power(2,(list_size-1-(i/2)));//block size that you want to end with, cannot break smallest size
    // printf("breaking block size to index i=%d\n", i);
    Header* temp;
    i-=2;//increase index to next size(decrease due to array holding largest block size first)

    if (i<=list_size*2&&i>=0)
    {
        if (list_ptr[i]==NULL) //if no blocks break next size
            if (block_break(i)==NULL)
                return NULL;
        temp=list_ptr[i];

        remove_header(i);
        i+=2;
        temp->size=break_size;//change old header block size
        temp[break_size/sizeof(Header)]=temp[0];//copy new header to right half
        temp->next=&temp[break_size/sizeof(Header)];
        list_ptr[i]=temp;
        temp=temp->next;
        temp->next=NULL;
        list_ptr[i+1]=temp;
        check_header(temp);
        return temp;
    }
    else
        return NULL;
}

void block_join(Header* ptr)
{
    uintptr_t temp=(uintptr_t)ptr;//cast pointer to int for bit operations
    temp=temp-mem_begin;
    int i=0;
    i=log2(ptr->size);
    temp ^= 1 << i ;//toggle the log base 2 of size bit from the right
    temp=temp+mem_begin;
    Header* buddy=(Header*)temp; //cast back from int to Header pointer for operations

    if (check_header(buddy)==1&&buddy->size==ptr->size)
    {
        if((int)(buddy-ptr)<0)
            ptr=buddy;
        remove_header((list_size-1-(log2(ptr->size)-log2(base)))*2);
        ptr->size=(ptr->size*2);
        block_join(ptr);
    }

}

/*--------------------------------------------------------------------------*/

unsigned int init_allocator(unsigned int _basic_block_size, unsigned int _length)
{
    unsigned int remainder=_length%_basic_block_size;
    unsigned int total=_length-remainder; //take away memory that will not fit into multiple of basic block size
    if (remainder!=0)
        total=total+_basic_block_size; //increment to allocate an extra block for the left over bytes
    _length=total;
    list_size=log2(total)-log2(_basic_block_size); //determine number of lists
    list_size=round(list_size+.5);//add .5 to always round up
    base=_basic_block_size;//for simplified use in other function
    Header** free_list=(Header**)malloc(2*list_size*sizeof(Header*)); //free list arrray

    free_list[0]=(Header*)malloc(total); //tell os to get continuous chunk of memory and store on free_list
    mem_begin=(uintptr_t)free_list[0];
    for (int i=1; i<list_size*2; ++i)
        free_list[i]=NULL; //initalize pointers to Null
    Header* ptr=free_list[0]; //set pointer that will be used to create headers to beginning of list
    Header head1;
    head1.symbol='~';
    head1.next=NULL;
   // printf("%f list size for the memory\n",list_size);
    for (int i=0; i<list_size*2; i+=2)
    {
        head1.size=(_basic_block_size*power(2,(list_size-((i+2)/2))));
     //   printf("\n%d working iteration %d  power is %d\n",total, i/2,power(2,(list_size-(((i+2)/2)))));
        if (total>=(_basic_block_size*power(2,(list_size-(((i+2)/2)))))) //so that the first pointer only gets initialized once
            free_list[i]=ptr; //else ptr is equal to null
        while (total>=(_basic_block_size*power(2,(list_size-(((i+2)/2))))))
        {

            ptr[0]=head1;
            ptr->next=&ptr[(_basic_block_size*power(2,(list_size-(((i+2)/2)))))/sizeof(Header)];
            ptr=ptr->next;
            total=total-_basic_block_size*power(2,(list_size-(((i+2)/2))));
       //     printf("total mem unallocated %d\n",total);
            if (ptr->size==free_list[i]->size)
                free_list[i+1]=ptr;
            else
                free_list[i+1]=free_list[i];
        }
    }

    list_ptr=free_list;

    return  _length; //if successful return amount of memory made available
}

extern Addr my_malloc(unsigned int _length)
{
    /* This preliminary implementation simply hands the call over the
       the C standard library!
       Of course this needs to be replaced by your implementation.
    */
    // determine block index/size needed remember all blocks have a header
    printf("allocating %d\n",_length);
    int index=0;
    while ((_length+sizeof(Header))/(base*power(2,index))>1)
        ++index;
    if(index>=list_size)
    {
        printf("my_malloc error, request exceeds biggest memory block\n");
        return NULL;
    }

    index=list_size-1-index;//set index to reverse order
    if (list_ptr[index*2]==NULL){
        printf("breaking blocks to listptr i=%d\n",index*2);
        if (block_break(index*2)==NULL) //break blocks if it is unsuccessful a
        {
            printf("error!!! block break unsuccessful no mem left returning NULL\n");
            return (void*)NULL;
        }
    }
    if (check_header(list_ptr[index*2])==0)
        printf("error in my malloc!!! acessing used memory\n");
    if (check_header(list_ptr[index*2])==-1)
        printf("error in my malloc!!! invalid header symbol\n");

    list_ptr[index*2]->symbol='!'; //symbol to signify used memory
    Addr my_addr=&list_ptr[index*2][1];//give address to free mem do not include header

    remove_header(index*2);
    return my_addr;
}

extern int my_free(Addr _a)
{
    Header* my_addr=(Header*)_a;
    int* eraser;
    eraser=(int*)my_addr;//pointer to erase memory
    //printf("freeing block! size\n");
    my_addr=&my_addr[-1]; //shift pointer from used mem to header
    //int sz=my_addr->size;
    //printf(" %d\n",sz);
    if (check_header(my_addr)==1)
        printf("my_free error block already free!\n");
    if (check_header(my_addr)==-1)
        printf("my_free error not a header!\n");
    my_addr->symbol='~';//set symbol to free
    block_join(my_addr);
    int i=0;
    while((my_addr->size-sizeof(Header))>(i*sizeof(int)))
    {
        eraser[i]&=0;
        ++i;
    }
    i=0;

    while (my_addr->size>(base*power(2,i))) //find index
        ++i;
    i=list_size-1-i;//inverse index to access list

    if (list_ptr[i*2]==NULL) //add to list
    {
        list_ptr[i*2]=my_addr;
        list_ptr[i*2+1]=my_addr;
    }
    else
    {
        my_addr->next=list_ptr[i*2];
        list_ptr[i*2]=my_addr;
    }


// free(_a);
    return 0;
}
int release_allocator()
{
    /* This function returns any allocated memory to the operating system.
       After this function is called, any allocation fails.
    */
    free((void*)list_ptr);
    free((void*)mem_begin);
    printf("deallocated memory\n");
    return 0;
}
