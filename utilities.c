#include "utilities.h"


/*-------------- Write your own code below ---------------*/
//Convert decimal to binary
void decToBin(int dec, int* bin, int binSize) {
    //returns the reversed binary representation of the decimal number
    int i = 0;
    while(dec > 0) {
        bin[i] = dec % 2;
        dec /= 2;
        i++;
    }
    while(i < binSize) {
        bin[i] = 0;
        i++;
    }
}

/*--------------------------------------------------------*/