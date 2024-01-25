#ifndef SIM_CACHE_H
#define SIM_CACHE_H

#define _CRT_SECURE_NO_WARNINGS // VS Studio comppatibliity


#include <stdio.h>
#include <math.h>
#include "configCache.h"

//Included for bool type
#include <stdbool.h>

// Simulation function
int SimulateCache(SC_SIM_Cache* CacheArr, int CacheLevel, FILE* fd);

// Cache Access functions (core)
void ReadFromCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt);
void WriteToCache(SC_SIM_Cache* CacheArr, int CacheLevel, int addr, int memoryAccessCnt);

// Other calculation functions
/* ------- Write your own code below  ------- */

//Get the decimal line index from the converted binary address array
int getLineIndex(SC_SIM_Cache* CacheArr, int* binaryAddress, int CacheLevel);

//Get the decimal block offset from the converted binary address array
int getBlockOffset(SC_SIM_Cache* CacheArr, int* binaryAddress, int CacheLevel);

//Get the decimal tag from the converted binary address array
int getTag(SC_SIM_Cache* CacheArr, int* binaryAddress, int CacheLevel);

//Verify cache hit
int isHit(SC_SIM_Cache* CacheArr, int CacheLevel, int lineIndex, int tag);

//Verify cache write hit
int isWriteHit(SC_SIM_Cache* CacheArr, int CacheLevel, int lineIndex, int tag);

//Read the data in the appropriate block index of the cache
void readData(SC_SIM_Cache* CacheArr, int lineIndex, int CacheLevel, int tag);

//Write the data in the appropriate block index of the cache (write-through)
void writeThroughData(SC_SIM_Cache* CacheArr, int setIndex, int lineIndex, int CacheLevel, int tag);

//Write the data in the appropriate block index of the cache (write-back)
void writeBackData(SC_SIM_Cache* CacheArr, int setIndex, int lineIndex, int CacheLevel, int tag);

//FIFO Replacement Policy
void updateFIFO(SC_SIM_Cache* cacheArr, int cacheLevel, int setIndex, int lineIndex);

int findFIFO(SC_SIM_Cache* cacheArr, int cacheLevel, int lineIndex);

//LRU Replacement Policy
void updateLRU(SC_SIM_Cache* cacheArr, int cacheLevel, int setIndex, int lineIndex);

int findLRU(SC_SIM_Cache* cacheArr, int cacheLevel, int lineIndex);

//Random Replacement Policy
void updateRandom(SC_SIM_Cache* cacheArr, int cacheLevel, int lineIndex);

int findRandom(SC_SIM_Cache* cacheArr, int cacheLevel, int lineIndex);
/* ------------------------------------------ */

#endif