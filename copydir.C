//HW 3 copy directory

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <errno.h>
#include <unistd.h>
#define BLKSIZE 1024

#define WRITE_FLAGS (O_WRONLY | O_CREAT | O_EXCL)
#define WRITE_PERMS (S_IRUSR | S_IWUSR)


//borrowed copy file from text book on page 130
using namespace std;

int copyfile(int fromfd, int tofd)
{
    char *bp;
    char buf[BLKSIZE];
    int bytesread, byteswritten;
    int totalbytes = 0;
    for ( ; ; )
    {
        while (((bytesread = read(fromfd, buf, BLKSIZE)) == -1) &&
                (errno == EINTR)) ;
// handle interruption by signal
        if (bytesread <= 0)
// real error or end-of-file on fromfd
            break;
        bp = buf;
        while (bytesread > 0)
        {
            while(((byteswritten = write(tofd, bp, bytesread)) == -1 ) &&
                    (errno == EINTR)) ;
// handle interruption by signal
            if (byteswritten <= 0)
// real error on tofd
                break;
            totalbytes += byteswritten;
            bytesread -= byteswritten;
            bp += byteswritten;
        }
        if (byteswritten == -1)
// real error on tofd
            break;
    }
    return totalbytes;
}



int main(int argc, char* argv[])
{



    if(argc < 2)
    {
        printf("Needs two file arguments source and destination\n");
        return 1;
    }

    char* s=argv[1];
    char* d=argv[2];
    int infd;
    int outfd;
    dirent* file;
    DIR* source = opendir(s);

    umask(0);//had to unmask chmod bits
    mkdir(d,0777); // create directory to copy files to


    std::string filename1=s; // used strings for ease of + operator
    std::string filename2=d;
    std::string temp;
    while((file=readdir(source))!=NULL) //while files are found
    {
        cout<<"found file\n";
       // char a[1024]; tests
       // getcwd(a,1024);
        temp=file->d_name;
        if(temp!="."||"..") //dont try to copy hidden directories
        {
            filename1=filename1+"/"+temp;
            cout<<"reading from "filename1<<endl;
            infd=open(filename1.c_str(),O_RDONLY);
            filename2=filename2+"/"+temp;
            outfd=open(filename2.c_str(),WRITE_FLAGS,WRITE_PERMS);
            cout<<"writing to "filename2<<endl;
            cout<<infd<<" in fd\n";
            cout<<outfd<<" out fd\n";

            copyfile(infd,outfd);//majority of the work
        }
        filename1=s;
        filename2=d;
    }



    printf(s);


}
