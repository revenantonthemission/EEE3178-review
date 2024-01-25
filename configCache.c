#include "configCache.h"


SC_SIM_Cache init_Cache(int cacheSize, int blockSize, SC_SIM_WritePolicy writePolicy, int associativity, SC_SIM_ReplacementPolicy replacementPolicy, int accessCycle)
{
    // cache declaration
    SC_SIM_Cache cache;

    // fundamental parameters initialization
    cache.cacheSize = cacheSize;
    cache.blockSize = blockSize;
    cache.associativity = associativity;
    cache.writePolicy = writePolicy;
    cache.replacementPolicy = replacementPolicy;
    cache.accessCycle = accessCycle;

    // derived parameters initialization
    cache.num_of_lines = cacheSize / (blockSize * associativity);
    
    // cache memory allocation
    cache.CacheLines = (SC_SIM_CacheLine**)malloc( (int)sizeof(SC_SIM_CacheLine*) * cache.associativity);
    for (int i = 0; i < cache.associativity; i++)
    {
        cache.CacheLines[i] = (SC_SIM_CacheLine*)malloc( (int)sizeof(SC_SIM_CacheLine) * cache.num_of_lines);
    }

    // memory data initialization
    for (int setIdx = 0; setIdx < cache.associativity; setIdx++)
    {
        for (int LineIdx = 0; LineIdx < cache.num_of_lines; LineIdx++)
        {
            cache.CacheLines[setIdx][LineIdx].tag = 0;
            cache.CacheLines[setIdx][LineIdx].valid = 0;
            cache.CacheLines[setIdx][LineIdx].dirty = 0;
        }
    }

    // profiler initialization
    cache.profiler.readCounter = 0;
    cache.profiler.readHitCounter = 0;
    cache.profiler.writeCounter = 0;
    cache.profiler.writeHitCounter = 0;


    /*--------------      Write your own code below   --------------*/
    //                                                              //
    //  The initialization of member that you added in SC_SIM_Cache //
    // will be done here.                                           //
    for (int setIdx = 0; setIdx < cache.associativity; setIdx++)
    {
        for (int lineIdx = 0; lineIdx < cache.num_of_lines; lineIdx++)
        {
            cache.CacheLines[setIdx][lineIdx].priority = setIdx;
        }
    }
    cache.profiler.mainMemoryAccessCnt = 0;
    /*--------------------------------------------------------------*/


    return cache;
}

void killCache(SC_SIM_Cache cache)
{
    // cache memory deallocation
    for (int i = 0; i < cache.associativity; i++)
    {
        free(cache.CacheLines[i]);
    }
    free(cache.CacheLines);

    return;
}

int calc_TotalAccessCycle(SC_SIM_Cache* CacheArr, int CacheLevel)
{
    int TotalAccessCycle = 0;
    /*--------------     Write your own code below    --------------*/
    //                                                              //
    //  The code for calculating total access cycle should be       //
    //  written here.                                               //
    for(int i=0; i<CacheLevel; i++) {
        TotalAccessCycle += CacheArr[i].accessCycle;
    }
    TotalAccessCycle += CacheArr[2].profiler.mainMemoryAccessCnt * SC_SIM_MEM_ACCESS_CYCLE;
    /*--------------------------------------------------------------*/

    return TotalAccessCycle;
}

float calc_GlobalHitRatio(SC_SIM_Cache* CacheArr, int CacheLevel)
{
    /*--------------     Write your own code below    --------------*/
    //                                                              //
    //  The code for the calculation of global hit ratio should be  //
    //  written here.                                               //

    // global hit ratio = (hit from instructions, not operations) / (instruction count)
    
    float global_hit_ratio = 0;
    int hitCount = 0;
    int accessCount = 0;
    for(int i=0; i<CacheLevel; i++) {
        hitCount += CacheArr[i].profiler.readHitCounter + CacheArr[i].profiler.writeHitCounter;
        accessCount += CacheArr[i].profiler.readCounter + CacheArr[i].profiler.writeCounter;
    }
    global_hit_ratio = SAFE_DIVIDE(hitCount, accessCount);
    return global_hit_ratio;
    /*--------------------------------------------------------------*/
}

float calc_CPI(SC_SIM_Cache* CacheArr, int CacheLevel, int instructionCount) {
    float access_accuracy = 0;

    // access accuracy = (total access cycle) / (instruction count)
    access_accuracy = SAFE_DIVIDE(calc_TotalAccessCycle(CacheArr, CacheLevel), instructionCount);
    return access_accuracy;
}

void calc_HitRatioForEachLevel(SC_SIM_Cache* CacheArr, int CacheLevel, int instructionCount) {
    int hitCount = 0;
    float memoryRatio = 1;
    for (int i = 0; i < CacheLevel; i++)
    {
        hitCount = CacheArr[i].profiler.readHitCounter + CacheArr[i].profiler.writeHitCounter;
        printf("L%d hit: %f (%%)\n", i+1, SAFE_DIVIDE(hitCount, instructionCount)*100);
        memoryRatio -= SAFE_DIVIDE(hitCount, instructionCount);
    }
    printf("Main memory: %f (%%)\n", memoryRatio*100);
    
}

