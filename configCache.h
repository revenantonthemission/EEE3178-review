/*============================================================*/
/*                      configCache.h                         */
/*============================================================*/
#ifndef CONFIG_CACHE_H
#define CONFIG_CACHE_H


#define _CRT_SECURE_NO_WARNINGS // VS Studio comppatibliity

#include <stdlib.h>
#include "utilities.h"
//추가 코드 3: 출력 사용을 위한 헤더파일
#include <stdio.h>


/*-------- Global parameters  --------*/
#define WORD_SIZE (8)   // [ bits ]
#define ADDR_SIZE (32)  // [ bits ]


/*-------- Cache parameters  --------*/
// cache access cycles
#define SC_SIM_L1_ACCESS_CYCLE (4)
#define SC_SIM_L2_ACCESS_CYCLE (10)
#define SC_SIM_L3_ACCESS_CYCLE (25)
#define SC_SIM_MEM_ACCESS_CYCLE (300)

// cache size
#define SC_SIM_L1_SIZE (1*1024)        // [ bytes ]
#define SC_SIM_L2_SIZE (64*1024)       // [ bytes ]
#define SC_SIM_L3_SIZE (256*1024)      // [ bytes ]

// cache block size
#define SC_SIM_L1_BLOCK_SIZE (4)       // [ bytes ]
#define SC_SIM_L2_BLOCK_SIZE (4)       // [ bytes ]
#define SC_SIM_L3_BLOCK_SIZE (4)       // [ bytes ]

typedef enum SC_SIM_WritePolicy_en
{
    SC_SIM_WRITE_THROUGH,
    SC_SIM_WRITE_BACK
}SC_SIM_WritePolicy;
/*-------------- you can edit the section below  ---------------*/
//                                                              //
//  This section is for step 3. You can edit this section       //
//  as you want.                                                //
//                                                              //
// cache write policy
#define SC_SIM_L1_WRITE_POLICY (SC_SIM_WRITE_BACK)
#define SC_SIM_L2_WRITE_POLICY (SC_SIM_WRITE_BACK)
#define SC_SIM_L3_WRITE_POLICY (SC_SIM_WRITE_BACK)
/*--------------------------------------------------------------*/

/*-------------- you can edit the section below  ---------------*/
//                                                              //
//  This section is for step 4. You can edit this section       //
//  as you want.                                                //
//                                                              //
// cache associativity
#define SC_SIM_L1_ASSOCIATIVITY (4)     // [ ways ]
#define SC_SIM_L2_ASSOCIATIVITY (1)     // [ ways ]
#define SC_SIM_L3_ASSOCIATIVITY (1)     // [ ways ]

int queue[SC_SIM_L1_ASSOCIATIVITY];

// cache replacement policy
typedef enum SC_SIM_ReplacementPolicy_en
{
    SC_SIM_LRU,
    SC_SIM_FIFO,
    SC_SIM_RANDOM,
    SC_SIM_NONE
}SC_SIM_ReplacementPolicy;
#define SC_SIM_L1_REPLACEMENT_POLICY (SC_SIM_RANDOM)
#define SC_SIM_L2_REPLACEMENT_POLICY (SC_SIM_RANDOM)
#define SC_SIM_L3_REPLACEMENT_POLICY (SC_SIM_RANDOM)
/*--------------------------------------------------------------*/


/*-------- Structure definitions --------*/

// This structure emulates the cache line.
// The emulated cache line has the information for cache control.
// Cache line instances will be dynamically allocated as the member of SC_SIM_Cache, and
// that allocation will be explained at SC_SIM_Cache structure definition.
typedef struct SC_SIM_CacheLine_st
{
    int tag;
    int valid;
    int dirty;
    /*-------------- you can edit the section below  ---------------*/
    //                                                              //
    //  This section is for Step 4. You can add some members        //
    //  that is required to implement the replacement policy.       //
    int priority;
    /*--------------------------------------------------------------*/

}SC_SIM_CacheLine;


// This structure acts as a profiler for cache performance evaluation.
// Some information about cache performance will be stored in this structure.
// And, it will be used to calculate the total access cycle and the global hit ratio
// at the function named calc_TotalAccessCycle and calc_GlobalHitRatio.
typedef struct SC_SIM_Profiler_st
{
    int readCounter;
    int readHitCounter;

    int writeCounter;
    int writeHitCounter;

    /*-------------- you can edit the section below  ---------------*/
    //                                                              //
    //  You can add some members that will be used for              //
    //  comprehensive profiling                                     //
    //The total number of main memory accesses
    int mainMemoryAccessCnt;
    /*--------------------------------------------------------------*/

}SC_SIM_Profiler;

// This structure emulates the cache.
// The cache structure has the information about cache line, cache profile data and the cache specification.
// Cache lines are dynamically allocated and the pointer member named 'CacheLines' points to those cache lines.
// Because multiple set-associative cache configuration should be supported, the member named 'CacheLines' is a pointer to pointer.
// The first index of the 'CacheLines' points to the set, and the second index points to the line in the set.
// So, after initialization, the 'CacheLines' will be a 2D array. and example of the 'CacheLines' is shown below.
//
//  someTagData = fooCache.CacheLines[4][7].tag;
//      In this example, the 'someTagData' will be the tag data of the 7th line in the 4th set of the cache named 'fooCache'
//
typedef struct SC_SIM_Cache_st
{
    // Cache memeory data and control bit storage
    SC_SIM_CacheLine** CacheLines;

    // profiler for cache performance evaluation
    SC_SIM_Profiler profiler;

    // Cache parameters (specification)
    int cacheSize;
    int blockSize;
    int associativity;
    int accessCycle;
    SC_SIM_WritePolicy writePolicy;
    SC_SIM_ReplacementPolicy replacementPolicy;
    
    // Derived parameters
    int num_of_lines;  // derived from cacheSize, blockSize, associativity

}SC_SIM_Cache;


/*-------- Function declarations --------*/
// cache life cycle functions
SC_SIM_Cache init_Cache(int cacheSize, int blockSize, SC_SIM_WritePolicy writePolicy, int associativity, SC_SIM_ReplacementPolicy replacementPolicy, int accessCycle);
void killCache(SC_SIM_Cache cache);

// cache profiler
int calc_TotalAccessCycle(SC_SIM_Cache* CacheArr, int CacheLevel);
float calc_GlobalHitRatio(SC_SIM_Cache* CacheArr, int CacheLevel);
/*--------------    Write your own code below     --------------*/
//                                                              //
//  You can add some profile functions for comprehensive        //
//  profiling                                                   //
float calc_CPI(SC_SIM_Cache* CacheArr, int CacheLevel, int instructionCount);
void calc_HitRatioForEachLevel(SC_SIM_Cache* CacheArr, int CacheLevel, int instructionCount);
/*--------------------------------------------------------------*/


#endif