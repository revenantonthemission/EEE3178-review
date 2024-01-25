#include <stdio.h>
#include <stdlib.h>
#include "Timer.h"
#include <immintrin.h>
#include <emmintrin.h>
#include <omp.h>
#include <assert.h>

// Experiment Iteration Number
#define EXPERIMENT_ITER_NUM (200)

// Image Size
#define IMG_xSize (500)
#define IMG_ySize (900)

// Image Processing Parameters
const int SmoothingKernal[5] = { 1, 2, 3, 2, 1 };
const int SobelFilter[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} }; // dimension : [ vertical ][ horizontal ]
const int Threshold = 25;

// Function Prototypes
void EdgeDetect(int* inputPtr, int* outputPtr, int xSize, int ySize);
void EdgeDetect_SIMD(int* inputPtr, int* outputPtr, int xSize, int ySize);
int verify(int* cOutput, int* SSEOutput, int xSize, int ySize);
void CacheFlush(__m128i* src, int countVect);

//
// 		Main Function starts here
//
int main()
{
	// file IO
	FILE* input_fp, * output_fp;					   // Input Image
	int* inputPtr, * out_SISD_Ptr, * out_SIMD_Ptr; // Output Image
	int ErrorChk;


	// performance measurement
	float dCpuTime;
	CPerfCounter counter;

	// stroage arrangement (dynamic allocation)
	int buffer_size;
	buffer_size = IMG_xSize * IMG_ySize;
	printf("buffer_size : %d \n", buffer_size * sizeof(int));

	inputPtr = new int[buffer_size];
	out_SISD_Ptr = new int[buffer_size];
	out_SIMD_Ptr = new int[buffer_size];


	/*************************************************************************************
	 * Read the input image
	 *************************************************************************************/
	ErrorChk = fopen_s(&input_fp, "RicciRoad.raw", "rb");
	assert(ErrorChk == 0); // check if the file is opened successfully

	ErrorChk = fread(inputPtr, IMG_xSize * 4, IMG_ySize, input_fp);
	assert(ErrorChk != 0); // check if the file is read successfully

	fclose(input_fp);

	/*****************************************************
	 * Call generic C Edge dectection
	 *****************************************************/

	dCpuTime = 0.0f;
	for (int loopCount = 0; loopCount < EXPERIMENT_ITER_NUM; loopCount++)
	{
		CacheFlush((__m128i*)inputPtr, buffer_size / 16);
		CacheFlush((__m128i*)out_SISD_Ptr, buffer_size / 16);
		counter.Reset();
		counter.Start();

		EdgeDetect(inputPtr, out_SISD_Ptr, IMG_xSize, IMG_ySize);

		counter.Stop();
		dCpuTime += counter.GetElapsedTime();
	}
	printf("C Performance (ms) = %f \n", dCpuTime / (double)EXPERIMENT_ITER_NUM * 1000.0);


	ErrorChk = fopen_s(&output_fp, "out_SISD.raw", "wb");
	assert(ErrorChk == 0); // check if the file is opened successfully

	ErrorChk = fwrite(out_SISD_Ptr, IMG_xSize * 4, IMG_ySize, output_fp);
	assert(ErrorChk != 0); // check if the file is written successfully

	fclose(output_fp);

	/*****************************************************
	 * Call SIMD Edge dectection
	 *****************************************************/

	dCpuTime = 0.0f;
	for (int loopCount = 0; loopCount < EXPERIMENT_ITER_NUM; loopCount++)
	{
		CacheFlush((__m128i*)inputPtr, buffer_size / 16);
		CacheFlush((__m128i*)out_SIMD_Ptr, buffer_size / 16);
		counter.Reset();
		counter.Start();

		EdgeDetect_SIMD(inputPtr, out_SIMD_Ptr, IMG_xSize, IMG_ySize);

		counter.Stop();
		dCpuTime += counter.GetElapsedTime();
	}

	printf("SIMD Performance (ms) = %f \n", dCpuTime / (double)EXPERIMENT_ITER_NUM * 1000.0);

	ErrorChk = fopen_s(&output_fp, "out_SIMD.raw", "wb");
	assert(ErrorChk == 0); // check if the file is opened successfully

	ErrorChk = fwrite(out_SIMD_Ptr, IMG_xSize * 4, IMG_ySize, output_fp);
	assert(ErrorChk != 0); // check if the file is written successfully

	fclose(output_fp);


	/*****************************************************
	 * Excution verification
	 *****************************************************/
	int error = verify(out_SISD_Ptr, out_SIMD_Ptr, IMG_xSize, IMG_ySize);

	if (error != 0)
		printf("Verify Failed \n");
	else
		printf("Verify Successful \n");

	/* free the allocated memories */
	delete [] inputPtr;
	delete [] out_SISD_Ptr;
	delete [] out_SIMD_Ptr;

	return 0;
}

int verify(int* COutput, int* SIMDOutput, int xSize, int ySize)
{
	for (int i = 0; i < xSize * ySize; i++)
	{
		if (COutput[i] != SIMDOutput[i])
		{
			printf("COutput[%d] = %d SIMDOutput[%d]=%d \n", i, COutput[i], i, SIMDOutput[i]);
			return (1);
		}
	}
	return (0);
}

void CacheFlush(__m128i* src, int countVect)
{
	int j;
	for (j = 0; j < countVect; j++)
	{
		_mm_clflush(src + j);
	}
}

/*********************************************************
 * EdgeDetect operation using general C code (SISD)
 *********************************************************/
void EdgeDetect(int* inputPtr, int* outputPtr, int xSize, int ySize)
{
	//Load Data to SSE Registers
	// Smoothing by Lateral smoothing filter
	// fill the boundary with zeros

	for (int y = 0; y < ySize; y++)
	{
		for (int x = 0; x < xSize; x++)
		{
			if (x < 2 || xSize - 2 <= x)
			{
				outputPtr[y * xSize + x] = 0;
			}
			else
			{
				// Smoothing by Lateral smoothing filter
				int sum = 0;
				for (int i = 0; i < 5; i++)
				{
					sum += inputPtr[y * xSize + x - 2 + i] * SmoothingKernal[i];
				}
				outputPtr[y * xSize + x] = sum / 9;
			}
		}
	}

	int *tempPtr = new int[ xSize * ySize ];
	// Sobel filter (2D convolution)
	// fill the boundary with zeros
	for (int y = 0; y < ySize; y++)
	{
		for (int x = 0; x < xSize; x++)
		{
			if (y < 1 || ySize - 1 <= y || x < 1 || xSize - 1 <= x)
			{
				tempPtr[y * xSize + x] = 0;
			}
			else
			{
				// Sobel filter (2D convolution)
				int sum = 0;
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						sum += outputPtr[(y - 1 + i) * xSize + (x - 1 + j)] * SobelFilter[i][j];
					}
				}
				tempPtr[y * xSize + x] = sum;

			}
		}
	}


	// Thresholding
	for (int x = 0; x < xSize; x++)
	{
		for (int y = 0; y < ySize; y++)
		{
			if (tempPtr[y * xSize + x] > Threshold)
				outputPtr[y * xSize + x] = 255;
			else
				outputPtr[y * xSize + x] = 0;
		}
	}

	delete[] tempPtr;

}







/*********************************************************
 * Optimize EdgeDetectSIMD operation usingintel MMX/SSE/SSE2/AVX/?
 *********************************************************/
void EdgeDetect_SIMD(int* inputPtr, int* outputPtr, int xSize, int ySize)
{
	/*********************************************************
	 * Write your code here (modify the general C code below)
	 *********************************************************/
	
	 // Smoothing by Lateral smoothing filter 
	 // fill the boundary with zeros

	__m256i smooth[5], temp;
	__m256 div = _mm256_set1_ps(9.0), sum;
	for (int i = 0; i < 5; i++)
	{
		smooth[i] = _mm256_set1_epi32(SmoothingKernal[i]);
	}
	for (int y = 0; y < ySize; y++)
	{
		_mm_stream_si32(&outputPtr[y * xSize], 0);
		_mm_stream_si32(&outputPtr[y * xSize + 1], 0);
		for (int x = 0; x + 11 < xSize; x += 8)
		{
			sum = _mm256_setzero_ps();
			for (int i = 0; i < 5; i++)
			{
				temp = _mm256_loadu_si256((__m256i*)(inputPtr + y * xSize + x + i));
				temp = _mm256_mullo_epi32(temp, smooth[i]); 
				sum = _mm256_add_ps(sum, _mm256_cvtepi32_ps(temp));
			}
			sum = _mm256_div_ps(sum, div);
			sum = _mm256_floor_ps(sum);
			_mm256_storeu_si256((__m256i*)(outputPtr + y * xSize + 2 + x), _mm256_cvtps_epi32(sum));

		}
		_mm_stream_si32(&outputPtr[y * xSize + xSize - 2], 0);
		_mm_stream_si32(&outputPtr[y * xSize + xSize - 1], 0);
	}

	 // 2D Convolution
	int* tempPtr = new int[xSize * ySize];
	
	__m256i sobel[6], conv, zeros = _mm256_setzero_si256(), tray;
	
	//Sobel Filter
	sobel[0] = _mm256_set1_epi32(SobelFilter[0][0]);
	sobel[1] = _mm256_set1_epi32(SobelFilter[0][2]);
	sobel[2] = _mm256_set1_epi32(SobelFilter[1][0]);
	sobel[3] = _mm256_set1_epi32(SobelFilter[1][2]);
	sobel[4] = _mm256_set1_epi32(SobelFilter[2][0]);
	sobel[5] = _mm256_set1_epi32(SobelFilter[2][2]);

	for (int x = 0; x + 8 < xSize; x+=8)
	{
		_mm256_storeu_si256((__m256i*)(tempPtr + x), zeros);
		_mm256_storeu_si256((__m256i*)(tempPtr + (ySize - 1) * xSize + x), zeros);
	}
	_mm256_storeu_si256((__m256i*)(tempPtr + xSize - 8), zeros);
	_mm256_storeu_si256((__m256i*)(tempPtr + ySize * xSize - 8), zeros);
	
	for (int y = 0; y + 2 < ySize; y++)
	{
		_mm_stream_si32(tempPtr + y * xSize, 0);
		//2D Convolution
		for (int x = 0; x + 10 < xSize; x += 8)
		{
			conv = _mm256_setzero_si256();
			for (int i = 0; i < 3; i++)
			{
				tray = _mm256_loadu_si256((__m256i*)(outputPtr + (y + i) * xSize + x));
				tray = _mm256_mullo_epi32(tray, sobel[2 * i]);
				conv = _mm256_add_epi32(conv, tray);

				tray = _mm256_loadu_si256((__m256i*)(outputPtr + (y + i) * xSize + x + 2));
				tray = _mm256_mullo_epi32(tray, sobel[2 * i + 1]);
				conv = _mm256_add_epi32(conv, tray);
			}
			_mm256_storeu_si256((__m256i*)(tempPtr + (y + 1) * xSize + x + 1), conv);
		}
		int sumt[2] = { 0, };
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				sumt[0] += outputPtr[(ySize - 2 + i) * xSize + (xSize - 2 - 1 + j)] * SobelFilter[i][j];
				sumt[1] += outputPtr[(ySize - 2 + i) * xSize + (xSize - 2 + j)] * SobelFilter[i][j];
			}
		}

		tempPtr[(ySize-1) * xSize - 3] = sumt[0];
		tempPtr[(ySize-1) * xSize - 2] = sumt[1];
		_mm_stream_si32(tempPtr + y * xSize + xSize - 1, 0);
	}

	// Thresholding
	__m256i ThresholdVector = _mm256_set1_epi32(Threshold), adjust = _mm256_set1_epi32(0x000000FF);
	int x = 0;
	for (x = 0; x < xSize * ySize; x += 8)
	{
		__m256i data = _mm256_loadu_si256((__m256i*)(tempPtr + x));
		__m256i compare = _mm256_and_si256(adjust, _mm256_cmpgt_epi32(data, ThresholdVector));
		_mm256_storeu_si256((__m256i*)(outputPtr + x), compare);
	}
	x -= 8;
	while (x < xSize * ySize)
	{
		outputPtr[x] = tempPtr[x] > (Threshold + 1) ? 255 : 0;
		x++;
	}

	delete[] tempPtr;
	return;
}
