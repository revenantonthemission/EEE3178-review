#ifndef UTILITIES_H
#define UTILITIES_H

#define _CRT_SECURE_NO_WARNINGS // VS Studio comppatibliity

#include <stdlib.h>
#include <math.h>


// some useful macros
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define ABS(x) ((x)>0?(x):-(x))
#define SIGN(x) ((x)>0?1:-1)
#define EPISILON (float)(1e-6)
#define SAFE_DIVIDE(a,b) ( (float)a/ ((float)b + EPISILON ) )


/*-------------- Write your own code below ---------------*/
//추가 코드 2: 난수 생성을 위한 헤더파일
#include <time.h>
//------------------variable declarations--------------------

//------------------function declarations--------------------
//Convert decimal to binary
void decToBin(int dec, int* bin, int binSize);
/*--------------------------------------------------------*/


#endif