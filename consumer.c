/*
CS 420
Assignment 1: Shared Memory and Multi-Tasking
Group 20
Section #02
OSs Tested on: macOS, Linux
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

// Size of shared memory block
// Pass this to ftruncate and mmap
#define SHM_SIZE 4096

// Global pointer to the shared memory block
// This should receive the return value of mmap
// Don't change this pointer in any function
void* gShmPtr;

// You won't necessarily need all the functions below
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

int main()
{
    //Name of shared memory block.
    const char *name = "OS_HW1_20";
    int bufSize;
    int itemCnt;
    int in;
    int out;

    //Create shared memory.
    int shm_fd = shm_open(name, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    //Map memory to global ptr.
    gShmPtr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (gShmPtr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    bufSize = GetBufSize();
    itemCnt = GetItemCnt();
    in = GetIn();
    out = GetOut();

    printf("Consumer reading: bufSize = %d\n", bufSize);
    printf("Consumer reading: itemCnt = %d\n", itemCnt);
    printf("Consumer reading: in = %d\n", in);
    printf("Consumer reading: out = %d\n", out);

    for (int i = 0; i < itemCnt; i++) {
        //Buffer is empty, wait.
        while (in == out) {
            usleep(1000);
            in = GetIn();
        }
        int val = ReadAtBufIndex(out);
        printf("Consuming Item %d with value %d at Index %d\n", i, val, out);
        out = (out + 1) % bufSize;
        SetOut(out);
    }

    //Cleanup shared memory block.
    if (shm_unlink(name) == -1) {
        printf("Error removing %s\n", name);
        exit(-1);
    }

    return 0;
}

void SetIn(int val)
{
    SetHeaderVal(2, val);
}

void SetOut(int val)
{
    SetHeaderVal(3, val);
}

int GetHeaderVal(int i)
{
    int val;
    void* ptr = gShmPtr + i * sizeof(int);
    memcpy(&val, ptr, sizeof(int));
    return val;
}

void SetHeaderVal(int i, int val)
{
    void* ptr = gShmPtr + i * sizeof(int);
    memcpy(ptr, &val, sizeof(int));
}

int GetBufSize()
{
    return GetHeaderVal(0);
}

int GetItemCnt()
{
    return GetHeaderVal(1);
}

int GetIn()
{
    return GetHeaderVal(2);
}

int GetOut()
{
    return GetHeaderVal(3);
}

void WriteAtBufIndex(int indx, int val)
{
    void* ptr = gShmPtr + 4 * sizeof(int) + indx * sizeof(int);
    memcpy(ptr, &val, sizeof(int));
}

int ReadAtBufIndex(int indx)
{
    int val;
    void* ptr = gShmPtr + 4 * sizeof(int) + indx * sizeof(int);
    memcpy(&val, ptr, sizeof(int));
    return val;
}
