/*
CS 420
Assignment 1: Shared Memory and Multi-Tasking
Group 20
Section #02
OSs Tested on: macOS, Linux
*/

/*
To compile and run your code, make sure that gcc
is installed on your system. As an example, on a
system like Ubunto you can run the command:
    gcc --version
to see if gcc is installed and which version(s).
If it is not installed, usually with a couple of
simple commands you can install it. Like:
    sudo apt update
    sudo apt install build-essential
After you implemented your code(s), you can
compile the codes using commands:
    gcc producer.c -lrt -o producer
    gcc consumer.c -lrt -o consumer
To test it, run it with some sample arguments:
    ./producer 5 100 10
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Size of shared memory block
// Pass this to ftruncate and mmap
#define SHM_SIZE 4096

// Global pointer to the shared memory block
// This should receive the return value of mmap
// Don't change this pointer in any function
void* gShmPtr;

void Producer(int, int, int);
void InitShm(int, int);
void SetBufSize(int);
void SetItemCnt(int);
void SetIn(int);
void SetOut(int);
void SetHeaderVal(int, int);
int GetBufSize();
int GetItemCnt();
int GetIn();
int GetOut();
int GetHeaderVal(int);
void WriteAtBufIndex(int, int);
int ReadAtBufIndex(int);
int GetRand(int, int);

int main(int argc, char* argv[])
{
    pid_t pid;
    int bufSize; // Bounded buffer size
    int itemCnt; // Number of items to be produced
    int randSeed;  // Seed for the rng.

    //Input validation as required.
    if(argc != 4){
            printf("Invalid number of command-line arguments\n");
            exit(1);
    }
    bufSize = atoi(argv[1]);
    itemCnt = atoi(argv[2]);
    randSeed = atoi(argv[3]);

    //Ensure both values are > 0.
    if (bufSize <= 0 || itemCnt <= 0) {
        printf("Invalid buffer size or item count\n");
        exit(1);
    }

    InitShm(bufSize, itemCnt);

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork Failed\n");
        exit(1);
    }
    else if (pid == 0) {
        //Child.
        printf("Launching Consumer \n");
        execlp("./consumer","consumer",NULL);
    }
    else {
        //Parent is non 0 pid.
        printf("Starting Producer\n");
        Producer(bufSize, itemCnt, randSeed);
        printf("Producer done and waiting for consumer\n");
        wait(NULL);
        printf("Consumer Completed\n");
    }

    return 0;
}

void InitShm(int bufSize, int itemCnt)
{
    const char *name = "OS_HW1_20";

    // Write code here to create a shared memory block and map it to gShmPtr

    // Open shared memory block with name.
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    //Global ptr to memory.
    gShmPtr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (gShmPtr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    SetBufSize(bufSize);
    SetItemCnt(itemCnt);
    SetIn(0);
    SetOut(0);
}

void Producer(int bufSize, int itemCnt, int randSeed)
{
    int in = 0;
    int out = 0;

    srand(randSeed);

    for (int i = 0; i < itemCnt; i++) {
        while ((in + 1) % bufSize == out) {
            usleep(1000);
            out = GetOut();
        }
        int val = GetRand(0, 2500);
        WriteAtBufIndex(in, val);
        printf("Producing Item %d with value %d at Index %d\n", i, val, in);
        in = (in + 1) % bufSize;
        SetIn(in);
    }

    printf("Producer Completed\n");
}

// Set the value of shared variable "bufSize"
void SetBufSize(int val)
{
    SetHeaderVal(0, val);
}

// Set the value of shared variable "itemCnt"
void SetItemCnt(int val)
{
    SetHeaderVal(1, val);
}

// Set the value of shared variable "in"
void SetIn(int val)
{
    SetHeaderVal(2, val);
}

// Set the value of shared variable "out"
void SetOut(int val)
{
    SetHeaderVal(3, val);
}

// Get the ith value in the header
int GetHeaderVal(int i)
{
    int val;
    void* ptr = gShmPtr + i * sizeof(int);
    memcpy(&val, ptr, sizeof(int));
    return val;
}

// Set the ith value in the header
void SetHeaderVal(int i, int val)
{
    //Same pointer math as 'Get' provided.
    void* ptr = gShmPtr + i * sizeof(int);
    memcpy(ptr, &val, sizeof(int));
}

// Get the value of shared variable "bufSize"
int GetBufSize()
{
    return GetHeaderVal(0);
}

// Get the value of shared variable "itemCnt"
int GetItemCnt()
{
    return GetHeaderVal(1);
}

// Get the value of shared variable "in"
int GetIn()
{
    return GetHeaderVal(2);
}

// Get the value of shared variable "out"
int GetOut()
{
    return GetHeaderVal(3);
}

// Write the given val at the given index in the bounded buffer
void WriteAtBufIndex(int indx, int val)
{
    void* ptr = gShmPtr + 4 * sizeof(int) + indx * sizeof(int);
    memcpy(ptr, &val, sizeof(int));
}

// Read the val at the given index in the bounded buffer
int ReadAtBufIndex(int indx)
{
    int val;
    void* ptr = gShmPtr + 4 * sizeof(int) + indx * sizeof(int);
    memcpy(&val, ptr, sizeof(int));
    return val;
}

// Get a random number in the range [x, y]
int GetRand(int x, int y)
{
    int r = rand();
    r = x + r % (y - x + 1);
    return r;
}
