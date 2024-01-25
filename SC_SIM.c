/*============================================================*/
/*                         SC_SIM.c                           */
/*                                                            */
/*    This cache simulator is designed for project 1          */
/*   of EEE3178(Computer Architecture) of Sogang university   */
/*                                                            */
/*   - The main function is defined in this file              */
/*                                                            */
/*============================================================*/

#include "simCache.h"

/*-------------- you can edit the section below  ---------------*/
//                                                              //
//  This section is for Step 2. You can edit this section       //
//  as you want.                                                //
//                                                              //
#define CacheLevel (3)
/*--------------------------------------------------------------*/

int main()
{
    // Cache Initialization
    SC_SIM_Cache L1Cache, L2Cache, L3Cache;
    L1Cache = init_Cache(SC_SIM_L1_SIZE, SC_SIM_L1_BLOCK_SIZE, SC_SIM_L1_WRITE_POLICY, SC_SIM_L1_ASSOCIATIVITY, SC_SIM_L1_REPLACEMENT_POLICY, SC_SIM_L1_ACCESS_CYCLE);
    L2Cache = init_Cache(SC_SIM_L2_SIZE, SC_SIM_L2_BLOCK_SIZE, SC_SIM_L2_WRITE_POLICY, SC_SIM_L2_ASSOCIATIVITY, SC_SIM_L2_REPLACEMENT_POLICY, SC_SIM_L2_ACCESS_CYCLE);
    L3Cache = init_Cache(SC_SIM_L3_SIZE, SC_SIM_L3_BLOCK_SIZE, SC_SIM_L3_WRITE_POLICY, SC_SIM_L3_ASSOCIATIVITY, SC_SIM_L3_REPLACEMENT_POLICY, SC_SIM_L3_ACCESS_CYCLE);

    /*-------------- you can edit the section below  ---------------*/
    //                                                              //
    //  This section is for Step 2. You can edit this section       //
    //  as you want.                                                //
    //                                                              //
    SC_SIM_Cache CacheArr[CacheLevel] = { L1Cache , L2Cache, L3Cache };
    /*--------------------------------------------------------------*/

    // Access trace file Loading
    FILE* fd = fopen("Conv2D_trace.txt", "r");

    // do simulation
    int memoryAccessCnt = SimulateCache(CacheArr, CacheLevel, fd);

    // print the result
    printf("Total Execution Cycles: %d\n", memoryAccessCnt);
    printf("Total hit ratio: %f (%%) \n", calc_GlobalHitRatio(CacheArr, CacheLevel) * 100);
    printf("Total access cycle: %d\n", calc_TotalAccessCycle(CacheArr, CacheLevel));
    printf("CPU cycles per Instruction: %f (cycles) \n", calc_CPI(CacheArr, CacheLevel, memoryAccessCnt));
    calc_HitRatioForEachLevel(CacheArr, CacheLevel, memoryAccessCnt);

    // terminate the execution
    fclose(fd);
    killCache(L1Cache);
    killCache(L2Cache);
    killCache(L3Cache);

    return 0;
}