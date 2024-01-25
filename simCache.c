#include <stdio.h>
#include <math.h>
#include "configCache.h"
#include "simCache.h"
#include "utilities.h"

int SimulateCache(SC_SIM_Cache* CacheArr, int CacheLevel, FILE* fd)
{
    // Excution initialization
    int memoryAccessCnt = 0;

    // do simulation
    while(1)
    {
        // Read Access Log
        char accessType; int addr;
        fscanf(fd, "%c %d\n", &accessType, &addr);

        //추가 코드 1: 입력 버퍼 비우기 
        fflush(fd);
        memoryAccessCnt++;
        if (feof(fd))
            break;

        // Memory Access
        switch (accessType)
        {
            case 'L':
                ReadFromCache(CacheArr, CacheLevel, addr, memoryAccessCnt);
                break;
            case 'S':
                WriteToCache(CacheArr, CacheLevel, addr, memoryAccessCnt);
                break;
            default :
                printf("Error: Invalid Access Type\n");
                break;
        }
    }
    return memoryAccessCnt ;
}

void ReadFromCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt)
{
    /* ------- Write your own code below  ------- */
    //An array for 32-bit binary address
    int binaryAddress[ADDR_SIZE] = {0, };

    //Line index, block offset, tag for each level
    int blockOffsetArr[3] = {0,}, lineIndexArr[3] = {0, }, tagArr[3] = {0,};

    //The level of cache that has hit for the first time
    int hitLevel = 0;

    //The index of the set that has hit
    int setIndex = 0;

    int replacement = 0;

    for(int i=0; i<SC_SIM_L1_ASSOCIATIVITY; i++)
    {
        queue[i] = i;
    }
    //Decimal addr -> 32-bit binary address
    //Each bit is written in the array in reversed order
    decToBin(addr, binaryAddress, ADDR_SIZE);

    //hit/miss와 관계없이 readCounter는 증가한다.
    CacheArr[hitLevel].profiler.readCounter++;

    //Hit level Judgement: find the first level of cache that has hit
    for(; hitLevel < CacheLevel; hitLevel++) {

        //Get line index, block offset, tag from the converted binary address
        blockOffsetArr[hitLevel] = getBlockOffset(CacheArr, binaryAddress, hitLevel);
        lineIndexArr[hitLevel] = getLineIndex(CacheArr, binaryAddress, hitLevel);
        tagArr[hitLevel] = getTag(CacheArr, binaryAddress, hitLevel);
        
        //Check if the cache has hit at the current level
        if(isHit(CacheArr, hitLevel, lineIndexArr[hitLevel], tagArr[hitLevel]) != -1) {

            //Code for cache hit
            //printf("Read: Cache hit at %d\n", hitLevel);
            
            //hit가 발생한 레벨에서 readHitCounter를 증가시킨다.
            CacheArr[hitLevel].profiler.readHitCounter++;
            break;

        }
        else {

            //Code for cache miss
            //printf("Read: Cache miss at %d\n", hitLevel);
            
        }

    }
    //printf("\n");

    //The write policy is identical for all levels.
    switch(CacheArr[0].writePolicy)
    {
        //Read with Write-through policy
        case SC_SIM_WRITE_THROUGH:

            //By replacement policy, determine the set index that will be replaced
            switch (CacheArr[0].replacementPolicy) {
                case SC_SIM_LRU:
                    replacement = findLRU(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                case SC_SIM_FIFO:
                    replacement = findFIFO(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                case SC_SIM_RANDOM:
                    replacement = findRandom(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                default:
                    break;
            }

            //Actions for each level of cache hit
            switch(hitLevel) {

                //Code for L1 cache hit...if access represents the whole instruction
                case 0:
                    //Read the data from the L1 cache.
                    readData(CacheArr, lineIndexArr[0], 0, tagArr[0]);
                    break;

                //Code for L2 cache hit...if access represents the whole instruction
                case 1:
                    //L1 miss
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;

                    //Read the data from the L2 cache.
                    readData(CacheArr, lineIndexArr[1], 1, tagArr[1]);

                    //Write the data into the L1 cache & Read the data from the L1 cache in parallel.
                    writeThroughData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    break;

                //Code for L3 cache hit...if access represents the whole instruction
                case 2:
                    //L1, L2 miss
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;
                    CacheArr[1].accessCycle += SC_SIM_L2_ACCESS_CYCLE;

                    //Read the data from the L3 cache.
                    readData(CacheArr, lineIndexArr[2], 2, tagArr[2]);

                    //Write the data into the L2 cache.
                    writeThroughData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                    //Write the data into the L1 cache & Read the data from the L1 cache in parallel.
                    writeThroughData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    break;

                //Code for cache misses at all levels
                default:
                    //L1, L2, L3 miss + Main memory access
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;
                    CacheArr[1].accessCycle += SC_SIM_L2_ACCESS_CYCLE;
                    CacheArr[2].accessCycle += SC_SIM_L3_ACCESS_CYCLE;

                    //Write the data into the L3 cache.
                    writeThroughData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                    //Write the data into the L2 cache.
                    writeThroughData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                    //Write the data into the L1 cache.
                    writeThroughData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    break;
            }
            break;
        
        //Read with Write-back policy
        case SC_SIM_WRITE_BACK:

            //By replacement policy, determine the set index that will be replaced
            switch (CacheArr[0].replacementPolicy) {
                case SC_SIM_LRU:
                    replacement = findLRU(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                case SC_SIM_FIFO:
                    replacement = findFIFO(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                case SC_SIM_RANDOM:
                    replacement = findRandom(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                default:
                    break;
            }

            //Actions for each level of cache hit
            switch(hitLevel) {

                //Code for L1 cache hit...if access represents the whole instruction
                case 0:

                    //Read the data from the L1 cache.
                    readData(CacheArr, lineIndexArr[0], 0, tagArr[0]);
                    break;

                //Code for L2 cache hit...if access represents the whole instruction
                case 1:

                    //L1 miss
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;  

                    //L1.dirty = 1
                    if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;

                        //L1 replacement - write from temp to main memory
                        CacheArr[2].profiler.mainMemoryAccessCnt++;

                        //Read the data from the L2 cache.
                        readData(CacheArr, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //L1.dirty = 0
                    else {
                        //Read the data from the L2 cache.
                        readData(CacheArr, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }
                    break;

                //Code for L3 cache hit...if access represents the whole instruction
                case 2:
                    //L1, L2 miss
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;
                    CacheArr[1].accessCycle += SC_SIM_L2_ACCESS_CYCLE;

                    //Consider (L1.dirty, L2.dirty).
                    //00 -> L1 miss + L2 miss + L3 read + L2 read + L1 read
                    if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0) {
                        //Read the data from the L3 cache.
                        readData(CacheArr, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //01 -> L1 miss + L2 miss + L2 save + L3 read + L2 read + L1 read
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1) {
                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;

                        //L2 replacement - write from temp to main memory
                        CacheArr[2].profiler.mainMemoryAccessCnt++;

                        //Read the data from the L3 cache.
                        readData(CacheArr, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //10 -> L1 miss + L1 save + L2 miss + L3 read + L2 read + L1 read
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;
                        //L1 replacement - write from temp to main memory
                        CacheArr[2].profiler.mainMemoryAccessCnt++;

                        //Read the data from the L3 cache.
                        readData(CacheArr, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //11 -> L1 miss + L1 save + L2 miss + L2 save + L3 read + L2 read + L1 read
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;
                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;
                        //L1 replacement - write from temp to main memory
                        //L2 replacement - write from temp to main memory
                        CacheArr[2].profiler.mainMemoryAccessCnt+=2;

                        //Read the data from the L3 cache.
                        readData(CacheArr, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }
                    break;

                //Code for cache misses at all levels
                default:
                    //L1, L2, L3 miss
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;
                    CacheArr[1].accessCycle += SC_SIM_L2_ACCESS_CYCLE;
                    CacheArr[2].accessCycle += SC_SIM_L3_ACCESS_CYCLE;

                    //Consider (L1.dirty, L2.dirty, L3.dirty).
                    //000 -> L1 miss + L2 miss + L3 miss + main memory + L3 write + L2 write + L1 write
                    if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 0) {
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt++;

                        //Write the data into the L3 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //001 -> L1 miss + L2 miss + L3 miss + (L3)main memory + main memory + L3 write + L2 write + L1 write
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 1) {
                        //L3 replacement - write to temp
                        CacheArr[2].accessCycle++;

                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+=2;

                        //Write the data into the L3 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //010 -> L1 miss + L2 miss + L3 miss + (L2)main memory + main memory + main memory + L3 write + L2 write + L1 write
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 0) {
                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+=2;

                        //Write the data into the L3 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //011 -> L1 miss + L2 miss + L3 miss + (L2)main memory + (L3)main memory + main memory + L3 write + L2 write + L1 write
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 1) {
                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;
                        //L3 replacement - write to temp
                        CacheArr[2].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+=3;

                        //Write the data into the L3 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //100 -> L1 miss + L2 miss + L3 miss + (L1)main memory + main memory + main memory + L3 write + L2 write + L1 write
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 0) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt += 2;

                        //Write the data into the L3 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //101 -> L1 miss + L2 miss + L3 miss + (L1)main memory + (L3)main memory + main memory + L3 write + L2 write + L1 write
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 1) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;
                        //L3 replacement - write to temp
                        CacheArr[2].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+=3;

                        //Write the data into the L3 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //110 -> L1 miss + L2 miss + L3 miss + (L1)main memory + (L2)main memory + main memory + L3 write + L2 write + L1 write
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 0) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;
                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+=3;

                        //Write the data into the L3 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //111 -> L1 miss + L2 miss + L3 miss + (L1)main memory + (L2)main memory + (L3)main memory + main memory + L3 write + L2 write + L1 write
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 1) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;
                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+=4;

                        //Write the data into the L3 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //Write the data into the L2 cache.
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //Write the data into the L1 cache.
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);

                        //Data from main memory
                        for(int i = 0; i < CacheLevel; i++) {
                            CacheArr[0].CacheLines[replacement][i].dirty = 0;
                            CacheArr[1].CacheLines[0][i].dirty = 0;
                            CacheArr[2].CacheLines[0][i].dirty = 0;
                        }
                    }
                    break;
            }
            break;
    }

    /* ------------------------------------------ */
    return;
}


void WriteToCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt)
{
    /* ------- Write your own code below  ------- */
    //An array for 32-bit binary address
    int binaryAddress[ADDR_SIZE] = {0, };

    //Line index, block offset, tag for each level
    int lineIndexArr[3] = {0,}, blockOffsetArr[3] = {0,}, tagArr[3] = {0,};

    //The level of cache that has hit for the first time
    int hitLevel = 0;

    int replacement = 0;

    //Decimal addr -> 32-bit binary address
    //Each bit is written in the array in reversed order
    decToBin(addr, binaryAddress, ADDR_SIZE);

    // hit/miss와 관계없이 writeCounter는 증가한다.
    CacheArr[hitLevel].profiler.writeCounter++;

    //Traverse CacheArr: Access L1 -> L2 -> L3 in order
    //index - 0: L1, 1: L2, 2: L3
    for(; hitLevel < CacheLevel; hitLevel++) {

        //Get set index, block offset, tag from the converted binary address
        lineIndexArr[hitLevel] = getLineIndex(CacheArr, binaryAddress, hitLevel);
        blockOffsetArr[hitLevel] = getBlockOffset(CacheArr, binaryAddress, hitLevel);
        tagArr[hitLevel] = getTag(CacheArr, binaryAddress, hitLevel);

        //Add the access cycle of the current level to the total access cycle
        
        //Check if the cache has the address at the current level
        if(isWriteHit(CacheArr, hitLevel, lineIndexArr[hitLevel], tagArr[hitLevel])) {

            //Code for cache hit
            //printf("Write: Cache hit at %d\n", hitLevel);

            //hit가 발생한 레벨에서 writeHitCounter를 증가시킨다.
            CacheArr[hitLevel].profiler.writeHitCounter++;
            replacement = isWriteHit(CacheArr, hitLevel, lineIndexArr[hitLevel], tagArr[hitLevel]);
            break;

        }
        else {

            //Code for cache miss
            //printf("Write: Cache miss at %d\n", hitLevel);
        }

    }
    //printf("\n");

    //The write policy is identical for all levels.
    switch(CacheArr[0].writePolicy) {

        //Write with Write-through policy
        case SC_SIM_WRITE_THROUGH:

            switch (CacheArr[0].replacementPolicy) {
                case SC_SIM_LRU:
                    replacement = findLRU(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                case SC_SIM_FIFO:
                    replacement = findFIFO(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                case SC_SIM_RANDOM:
                    replacement = findRandom(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                default:
                    break;
            }

            //Actions for each level of cache hit
            switch(hitLevel) {

                //Code for L1 cache hit...if access represents the whole instruction
                case 0:
                    //L1 hit : Read the data from the L1 cache.
                    readData(CacheArr, lineIndexArr[0], 0, tagArr[0]);

                    //Write the data into the L1 / L2 / L3 / Main memory.
                    writeThroughData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    writeThroughData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);
                    writeThroughData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);
                    CacheArr[2].profiler.mainMemoryAccessCnt++;

                    //bit configuration
                    break;

                //Code for L2 cache hit...if access represents the whole instruction
                case 1:
                    //L1 write miss : the required block is not in the L1 cache.
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;

                    //L2 hit : Read the data from the L2 cache.
                    readData(CacheArr, lineIndexArr[1], 1, tagArr[1]);
                    
                    //Write the data into the L1 cache.
                    writeThroughData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);

                    //Write the data into the L3 / Main memory, since the data is already written in L2.
                    writeThroughData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);
                    CacheArr[2].profiler.mainMemoryAccessCnt++;

                    //bit configuration
                    break;

                //Code for L3 cache hit...if access represents the whole instruction
                case 2:
                    //L1, L2 write miss
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;
                    CacheArr[1].accessCycle += SC_SIM_L2_ACCESS_CYCLE;

                    //L3 hit : Read the data from the L3 cache.
                    readData(CacheArr, lineIndexArr[2], 2, tagArr[2]);

                    //write the data into the L2 & L1 cache.
                    writeThroughData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);
                    writeThroughData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);

                    //Write the data into the Main memory.
                    CacheArr[2].profiler.mainMemoryAccessCnt++;

                    //bit configuration
                    break;

                //Code for cache misses at all levels
                default:
                    //L1, L2, L3 write miss
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;
                    CacheArr[1].accessCycle += SC_SIM_L2_ACCESS_CYCLE;
                    CacheArr[2].accessCycle += SC_SIM_L3_ACCESS_CYCLE;

                    //Read the data from the main memory.
                    CacheArr[2].profiler.mainMemoryAccessCnt++;

                    //Write the data into the L3 / L2 / L1 cache.
                    writeThroughData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);
                    writeThroughData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);
                    writeThroughData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);

                    break;

            }
            break;
        
        //Write with Write-back policy
        case SC_SIM_WRITE_BACK:

            switch (CacheArr[0].replacementPolicy) {
                case SC_SIM_LRU:
                    replacement = findLRU(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                case SC_SIM_FIFO:
                    replacement = findFIFO(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                case SC_SIM_RANDOM:
                    replacement = findRandom(CacheArr, hitLevel, lineIndexArr[hitLevel]);
                    break;
                default:
                    break;
            }

            //Actions for each level of cache hit
            switch(hitLevel) {   
            
                //Code for L1 cache hit...if access represents the whole instruction
                case 0:
                    //Write the data only into the L1 cache.
                    writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    break;
                
                //L2 hit
                case 1:
                    //L1 write miss
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;

                    //L1 dirty?
                    //dirty = 1 : L1 replacement
                    //L1 miss + register(1) + L2 write + L1 write
                    if(CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;

                        //L1 replacement - write from temp to main memory
                        CacheArr[2].profiler.mainMemoryAccessCnt++;

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //dirty = 0
                    //L1 miss + L2 write + L1 write
                    else {
                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }
                    break;
                
                //L3 hit
                case 2:
                    //L1, L2 write miss
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;
                    CacheArr[1].accessCycle += SC_SIM_L2_ACCESS_CYCLE;

                    //L1 0 / L2 0 : L1 miss+ L2 miss + L3 write + L2 write + L1 write
                    if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0) {
                        //L3 write
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //L1 1 / L2 0 : L1 miss + temp + memory + L2 miss + L3 write + L2 write + L1 write
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;

                        //memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt++;

                        //L3 write
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //L1 0 / L2 1 : L1 miss + L2 miss + temp + memory + L3 write + L2 write + L1 write
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1) {
                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;
                        
                        //memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt++;

                        //L3 write
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //L1 1 / L2 1 : L1 miss + temp + memory + L2 miss + temp + memory + L3 write + L2 write + L1 write
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;
                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;

                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+=2;

                        //L3 write
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    break;

                //Code for cache misses at all levels
                default:
                    //L1, L2, L3 miss
                    CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;
                    CacheArr[1].accessCycle += SC_SIM_L2_ACCESS_CYCLE;
                    CacheArr[2].accessCycle += SC_SIM_L3_ACCESS_CYCLE;

                    //Consider (L1.dirty, L2.dirty, L3.dirty).
                    //000 -> L1 miss + L2 miss + L3 miss + main memory + L3 write + L2 write + L1 write
                    if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 0) {
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt++;

                        //L3 write
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //001 -> L1 miss + L2 miss + L3 miss + (L3)main memory + main memory
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 1) {
                        //L3 replacement - write to temp
                        CacheArr[2].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+= 2;

                        //L3 write
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //010 -> L1 miss + L2 miss + L3 miss + (L2)main memory + main memory + main memory
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 0) {
                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+= 2;

                        //L3 write
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //011 -> L1 miss + L2 miss + L3 miss + (L2)main memory + (L3)main memory + main memory
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 0 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 1) {
                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;

                        //L3 replacement - write to temp
                        CacheArr[2].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+= 3;

                        //L3 write
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);
                    }

                    //100 -> L1 miss + L2 miss + L3 miss + (L1)main memory + main memory
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 0) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+= 2;

                        //L3 write
                        writeBackData(CacheArr, replacement, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, replacement, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);

                    }

                    //101 -> L1 miss + L2 miss + L3 miss + (L1)main memory + (L3)main memory + main memory
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 0 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 1) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;
                        //L3 replacement - write to temp
                        CacheArr[2].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+= 3;

                        //L3 write
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);

                    }

                    //110 -> L1 miss + L2 miss + L3 miss + (L1)main memory + (L2)main memory + main memory
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 0) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;

                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+= 3;

                        //L3 write
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);

                    }

                    //111 -> L1 miss + L2 miss + L3 miss + (L1)main memory + (L2)main memory + (L3)main memory + main memory
                    else if (CacheArr[0].CacheLines[replacement][lineIndexArr[0]].dirty == 1 && CacheArr[1].CacheLines[0][lineIndexArr[1]].dirty == 1 && CacheArr[2].CacheLines[0][lineIndexArr[2]].dirty == 1) {
                        //L1 replacement - write to temp
                        CacheArr[0].accessCycle++;
                        //L2 replacement - write to temp
                        CacheArr[1].accessCycle++;
                        
                        //main memory access
                        CacheArr[2].profiler.mainMemoryAccessCnt+= 4;

                        //L3 write
                        writeBackData(CacheArr, 0, lineIndexArr[2], 2, tagArr[2]);

                        //L2 write
                        writeBackData(CacheArr, 0, lineIndexArr[1], 1, tagArr[1]);

                        //L1 write
                        writeBackData(CacheArr, replacement, lineIndexArr[0], 0, tagArr[0]);

                        //Data from main memory
                        for(int i = 0; i < CacheLevel; i++) {
                            CacheArr[0].CacheLines[replacement][i].dirty = 0;
                            CacheArr[1].CacheLines[0][i].dirty = 0;
                            CacheArr[2].CacheLines[0][i].dirty = 0;
                        }

                    }
                    break;
            }
            break;
    }
    /* ------------------------------------------ */

    return;
}



/* ------- Write your own code below  ------- */

//Get the decimal line index from the converted binary address array
int getLineIndex(SC_SIM_Cache* CacheArr, int* binaryAddress, int CacheLevel)
{
    int index = 0;
    int adjusted = 0;

    for(int j = (int)log2(CacheArr[CacheLevel].num_of_lines)-1; j >= 0; j--) {

        //Address: LSB -> ... -> MSB
        adjusted = j + (int)log2(CacheArr[CacheLevel].blockSize);
        index += binaryAddress[adjusted] * pow(2, adjusted);

    }
    return index;
}

//Get the decimal block offset from the converted binary address array
int getBlockOffset(SC_SIM_Cache* CacheArr, int* binaryAddress, int CacheLevel)
{
    int blockOffset = 0;

    for(int j = (int)log2(CacheArr[CacheLevel].blockSize)-1; j >= 0; j--) {

        //Address: LSB -> ... -> MSB
        blockOffset += binaryAddress[j] * pow(2, j);
    }
    return blockOffset;
}

//Get the decimal tag from the converted binary address array
int getTag(SC_SIM_Cache* CacheArr, int* binaryAddress, int CacheLevel)
{
    int tag = 0;
    for(int j = ADDR_SIZE-1; j >= (int)log2(CacheArr[CacheLevel].num_of_lines); j--) {

        //Address: LSB -> ... -> MSB
        tag += binaryAddress[j] * pow(2, j - (int)log2(CacheArr[CacheLevel].num_of_lines));
    }
    return tag;
}

//Verify cache hit
int isHit(SC_SIM_Cache* CacheArr, int CacheLevel, int lineIndex, int tag)
{
    //Cache hit: the valid bit is 1 & the tag is identical
    //(Step 4) Set-associative cache
    int setIndex = 0;

    switch(CacheArr[0].replacementPolicy) {
        case SC_SIM_LRU:
            //LRU
            setIndex = findLRU(CacheArr, CacheLevel, lineIndex);
            break;
        case SC_SIM_FIFO:
            //FIFO
            setIndex = findFIFO(CacheArr, CacheLevel, lineIndex);
            break;
        case SC_SIM_RANDOM:
            setIndex = findRandom(CacheArr, CacheLevel, lineIndex);
            break;
        default:
            break;
    }
    if (CacheArr[CacheLevel].CacheLines[setIndex][lineIndex].valid == 1 && CacheArr[CacheLevel].CacheLines[setIndex][lineIndex].tag == tag) return 1;

    //If the cache miss, return -1
    return -1;
}

//Verify cache write hit
int isWriteHit(SC_SIM_Cache* CacheArr, int CacheLevel, int lineIndex, int tag)
{
    //Write hit: the tag is identical
    //(Step 4) Set-associative cache
    int setIndex = 0;

    switch(CacheArr[0].replacementPolicy) {
        case SC_SIM_LRU:
            //LRU
            setIndex = findLRU(CacheArr, CacheLevel, lineIndex);
            break;
        case SC_SIM_FIFO:
            //FIFO
            setIndex = findFIFO(CacheArr, CacheLevel, lineIndex);
            break;
        case SC_SIM_RANDOM:
            setIndex = findRandom(CacheArr, CacheLevel, lineIndex);
            break;
        default:
            break;
    }

    if (CacheArr[CacheLevel].CacheLines[setIndex][lineIndex].tag == tag) return 1;
    //If the cache miss, return -1
    return -1;
}

//Read the data in the appropriate block index of the cache
void readData(SC_SIM_Cache* CacheArr, int lineIndex, int CacheLevel, int tag)
{
    int setIndex = 0;

    switch (CacheLevel) {
        case 0:
            //L1 read
            CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;
            break;
        case 1:
            //L2 read
            CacheArr[1].accessCycle += SC_SIM_L2_ACCESS_CYCLE;
            break;
        case 2:
            //L3 read
            CacheArr[2].accessCycle += SC_SIM_L3_ACCESS_CYCLE;
            break;
    }

    switch(CacheArr[0].replacementPolicy) {
        case SC_SIM_LRU:
            //LRU
            setIndex = findLRU(CacheArr, CacheLevel, lineIndex);
            updateLRU(CacheArr, CacheLevel, setIndex, lineIndex);
            break;
        case SC_SIM_FIFO:
            //FIFO
            setIndex = findFIFO(CacheArr, CacheLevel, lineIndex);
            updateFIFO(CacheArr, CacheLevel, setIndex, lineIndex);
            break;
        case SC_SIM_RANDOM:
            setIndex = findRandom(CacheArr, CacheLevel, lineIndex);
            updateRandom(CacheArr, CacheLevel, lineIndex);
            break;
        default:
            break;
    }
}

//Write the data in the appropriate block index of the cache (write-through)
void writeThroughData(SC_SIM_Cache* CacheArr, int setIndex, int lineIndex, int CacheLevel, int tag)
{

    switch (CacheLevel) {
        case 0:
            //L1 write
            CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;
            break;
        case 1:
            //L2 write
            CacheArr[1].accessCycle += SC_SIM_L2_ACCESS_CYCLE;
            break;
        case 2:
            //L3 write
            CacheArr[2].accessCycle += SC_SIM_L3_ACCESS_CYCLE;
            break;
    }

    CacheArr[CacheLevel].CacheLines[setIndex][lineIndex].tag = tag;
    CacheArr[CacheLevel].CacheLines[setIndex][lineIndex].valid = 1;

    switch(CacheArr[0].replacementPolicy) {
        case SC_SIM_LRU:
            //LRU
            updateLRU(CacheArr, CacheLevel, setIndex, lineIndex);
            break;
        case SC_SIM_FIFO:
            //FIFO
            updateFIFO(CacheArr, CacheLevel, setIndex, lineIndex);
            break;
        case SC_SIM_RANDOM:
            updateRandom(CacheArr, CacheLevel, lineIndex);
            break;
        default:
            break;
    }
}

//Write the data in the appropriate block index of the cache (write-back)
void writeBackData(SC_SIM_Cache* CacheArr, int setIndex, int lineIndex, int CacheLevel, int tag)
{
    switch (CacheLevel) {
        case 0:
            //L1 write
            CacheArr[0].accessCycle += SC_SIM_L1_ACCESS_CYCLE;
            break;
        case 1:
            //L2 write
            CacheArr[1].accessCycle += SC_SIM_L2_ACCESS_CYCLE;
            break;
        case 2:
            //L3 write
            CacheArr[2].accessCycle += SC_SIM_L3_ACCESS_CYCLE;
            break;
        default:
            break;
    }

    CacheArr[CacheLevel].CacheLines[setIndex][lineIndex].tag = tag;
    CacheArr[CacheLevel].CacheLines[setIndex][lineIndex].valid = 1;
    CacheArr[CacheLevel].CacheLines[setIndex][lineIndex].dirty = 1;

        switch(CacheArr[0].replacementPolicy) {
        case SC_SIM_LRU:
            //LRU
            updateLRU(CacheArr, CacheLevel, setIndex, lineIndex);
            break;
        case SC_SIM_FIFO:
            //FIFO
            updateFIFO(CacheArr, CacheLevel, setIndex, lineIndex);
            break;
        case SC_SIM_RANDOM:
            updateRandom(CacheArr, CacheLevel, lineIndex);
            break;
        default:
            break;
    }
    
}

void updateFIFO(SC_SIM_Cache* cacheArr, int cacheLevel, int setIndex, int lineIndex) {
    int temp = setIndex;
    for(int i=0; i<cacheArr[cacheLevel].associativity-1; i++) {
        queue[i] = queue[i+1];
    }
    queue[cacheArr[cacheLevel].associativity-1] = temp;

}

int findFIFO(SC_SIM_Cache* cacheArr, int cacheLevel, int lineIndex) {
   return queue[0];
}

// updateLRU 함수: 해당 라인의 우선순위를 최상위로 변경, 나머지는 한 단계씩 후퇴
void updateLRU(SC_SIM_Cache* cacheArr, int cacheLevel, int setIndex, int lineIndex) {

    for(int i = 0; i < cacheArr[cacheLevel].associativity; i++) {
         if(cacheArr[cacheLevel].CacheLines[i][lineIndex].priority < cacheArr[cacheLevel].CacheLines[setIndex][lineIndex].priority) {
            cacheArr[cacheLevel].CacheLines[i][lineIndex].priority++;
        }
    }
    //
    cacheArr[cacheLevel].CacheLines[setIndex][lineIndex].priority = 0;
}

// findLRU 함수: 가장 오래된 라인 인덱스 반환
int findLRU(SC_SIM_Cache* cacheArr, int cacheLevel, int lineIndex) {

    int oldest = 0;
    for (int i = 1; i < cacheArr[cacheLevel].associativity; i++) {
        if (cacheArr[cacheLevel].CacheLines[i][lineIndex].priority > cacheArr[cacheLevel].CacheLines[oldest][lineIndex].priority) {
            oldest = i;
        }
    }
    return oldest;
}

//Random replacement
void updateRandom(SC_SIM_Cache* cacheArr, int cacheLevel, int lineIndex) {
    srand(time(0));
    for(int i = 0; i < cacheArr[cacheLevel].associativity; i++) {
        cacheArr[cacheLevel].CacheLines[i][lineIndex].priority = rand() % cacheArr[cacheLevel].associativity;
    }
}
int findRandom(SC_SIM_Cache* cacheArr, int cacheLevel, int lineIndex) {
    int highest = 0;
    for(int i = 1; i < cacheArr[cacheLevel].associativity; i++) {
        if(cacheArr[cacheLevel].CacheLines[i][lineIndex].priority > cacheArr[cacheLevel].CacheLines[highest][lineIndex].priority) {
            highest = i;
        }
    }
    return highest;
}

/* ------------------------------------------ */
