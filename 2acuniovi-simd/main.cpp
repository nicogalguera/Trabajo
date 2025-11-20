/*
 * Main.cpp
 *
 *  Created on: Fall 2019
 */

#include <stdio.h>
#include <math.h>
#include <CImg.h>
#include <immintrin.h> // Required to use intrinsic functions

using namespace cimg_library;

// Data type for image components
typedef float data_t;

const char* SOURCE1_IMG      = "hojas_2.bmp";
const char* SOURCE2_IMG      = "background_bw_l_4.bmp";
const char* DESTINATION_IMG = "bailarina2.bmp";

// Filter argument data type
typedef struct {
	data_t *pRsrc1; // Pointers to the R, G and B components
	data_t *pGsrc1;
	data_t *pBsrc1;
	data_t *pRsrc2; 
	data_t *pGsrc2;
	data_t *pBsrc2;
	data_t *pRdst; // Pointers to the R, G and B components
	data_t *pGdst;
	data_t *pBdst;
	
	uint pixelCount; // Size of the image in pixels
} filter_args_t;

 
#define ITEMS_PER_PACKET (sizeof(__m256)/sizeof(data_t))

void filter (filter_args_t args) {

    // Number of packets
    int nPackets = args.pixelCount/ITEMS_PER_PACKET;

	// Elements in excess
	int inExcess = args.pixelCount%ITEMS_PER_PACKET;

	// Number of elements calculated with intrinsic functions
	int nVectorialCalculated = nPackets * ITEMS_PER_PACKET; 
	
    // 32 bytes (256 bits) packets. Used to stored aligned memory data
    __m256 vpRsrc1, vpGsrc1, vpBsrc1, vpRsrc2, vpGsrc2, vpBsrc2;
    
	for (int i = 0; i<nPackets; i++){
		// Unalainged load of pixel colors to avoid execution errors
		vpRsrc1 = _mm256_loadu_ps(args.pRsrc1 + ITEMS_PER_PACKET * i);
		vpGsrc1 = _mm256_loadu_ps(args.pGsrc1 + ITEMS_PER_PACKET * i);
		vpBsrc1 = _mm256_loadu_ps(args.pBsrc1 + ITEMS_PER_PACKET * i);
		vpRsrc2 = _mm256_loadu_ps(args.pRsrc2 + ITEMS_PER_PACKET * i);
		vpGsrc2 = _mm256_loadu_ps(args.pGsrc2 + ITEMS_PER_PACKET * i);
		vpBsrc2 = _mm256_loadu_ps(args.pBsrc2 + ITEMS_PER_PACKET * i);

		/* 
		Divide the algorithm in elemental operations to use intrinsic functions
		step1C = X(c)_i + 1
		step2C = 255 - Y(c)_i
		step3C = 256 * step2C
		step4C = step3C / step1C
		step5C = 255 - step4C
		vpCdst -> Limit the value of step5C to [0,255]
		C takes the values of R, G and B
		*/

		__m256 step1R = _mm256_add_ps(vpRsrc1, _mm256_set1_ps(1)); // Algorithm for R
		__m256 step2R = _mm256_sub_ps(_mm256_set1_ps(255), vpRsrc2);
		__m256 step3R = _mm256_mul_ps(_mm256_set1_ps(256), step2R);
		__m256 step4R = _mm256_div_ps(step3R, step1R);
		__m256 step5R = _mm256_sub_ps(_mm256_set1_ps(255), step4R);
		__m256 resultR = _mm256_max_ps(_mm256_set1_ps(0), _mm256_min_ps(step5R,_mm256_set1_ps(255)));
		_mm256_store_ps(args.pRdst + ITEMS_PER_PACKET * i, resultR);

		__m256 step1G = _mm256_add_ps(vpGsrc1, _mm256_set1_ps(1)); // Algorithm for G
		__m256 step2G = _mm256_sub_ps(_mm256_set1_ps(255), vpGsrc2);
		__m256 step3G = _mm256_mul_ps(_mm256_set1_ps(256), step2G);
		__m256 step4G = _mm256_div_ps(step3G, step1G);
		__m256 step5G = _mm256_sub_ps(_mm256_set1_ps(255), step4G);
		__m256 resultG = _mm256_max_ps(_mm256_set1_ps(0), _mm256_min_ps(step5G,_mm256_set1_ps(255)));
		_mm256_store_ps(args.pGdst + ITEMS_PER_PACKET * i, resultG);
		
		__m256 step1B = _mm256_add_ps(vpBsrc1, _mm256_set1_ps(1)); // Algorithm for B
		__m256 step2B = _mm256_sub_ps(_mm256_set1_ps(255), vpBsrc2);
		__m256 step3B = _mm256_mul_ps(_mm256_set1_ps(256), step2B);
		__m256 step4B = _mm256_div_ps(step3B, step1B);
		__m256 step5B = _mm256_sub_ps(_mm256_set1_ps(255), step4B);
		__m256 resultB = _mm256_max_ps(_mm256_set1_ps(0), _mm256_min_ps(step5B,_mm256_set1_ps(255)));
		_mm256_store_ps(args.pBdst + ITEMS_PER_PACKET * i, resultB);
	} 

	for(int i = 0; i<inExcess; i++){
		
		float res_r, res_g, res_b;	// auxiliar vars to avoid overflow

		res_r = 255.0f - (256.0f * (255.0f - *(args.pRsrc2 + nVectorialCalculated + i)) / (*(args.pRsrc1 + nVectorialCalculated + i) + 1.0f));
		*(args.pRdst + nVectorialCalculated + i) = fmaxf(0.0f, fminf(255.0f, res_r));	// Limit the pixel value to [0, 255]
		res_g = 255.0f - (256.0f * (255.0f - *(args.pGsrc2 + nVectorialCalculated + i)) / (*(args.pGsrc1 + nVectorialCalculated + i) + 1.0f));
		*(args.pGdst + nVectorialCalculated + i) = fmaxf(0.0f, fminf(255.0f, res_g));
		res_b = 255.0f - (256.0f * (255.0f - *(args.pBsrc2 + nVectorialCalculated + i)) / (*(args.pBsrc1 + nVectorialCalculated + i) + 1.0f));
		*(args.pBdst + nVectorialCalculated + i) = fmaxf(0.0f, fminf(255.0f, res_b));
	}
}


int main() {
	// Open file and object initialization
	CImg<data_t> srcImage1(SOURCE1_IMG);
	CImg<data_t> srcImage2(SOURCE2_IMG);

	filter_args_t filter_args;
	data_t *pDstImage; // Pointer to the new image pixels


	/***************************************************
	 *   - Prepare variables for the algorithm
	 *   - This is not included in the benchmark time
	 */
//

//

	srcImage1.display(); // Displays the source image1
	uint width1 = srcImage1.width();// Getting information from the source image
	uint height1 = srcImage1.height();	
	uint nComp1 = srcImage1.spectrum();// source image number of components
	         // Common values for spectrum (number of image components):
				//  B&W images = 1
				//	Normal color images = 3 (RGB)
				//  Special color images = 4 (RGB and alpha/transparency channel)
	srcImage2.display(); // Displays the source image1
	uint width2 = srcImage2.width();// Getting information from the source image2
	uint height2 = srcImage2.height();	
	uint nComp2 = srcImage2.spectrum();

	// Check if both images are the same size
	if (width1 != width2 || height1 != height2){
		perror("Images sizes aren't the same.");
		exit(-3);
	} 

	// Check if both images have the same number of components
	if (nComp1 != nComp2) {
    	perror("Images have different number of components.");
    	exit(-4);
	}

	// Check if the images are RGB
	if (nComp1 != 3) {
    	perror("This filter only supports RGB images (3 components).");
    	exit(-5);
	}


	// Calculating image size in pixels
	filter_args.pixelCount = width1 * height1;
	
	// Allocate memory space for destination image components aligned to 32 bytes
	pDstImage = (data_t *) _mm_malloc(filter_args.pixelCount * nComp1 * sizeof(data_t), sizeof(__m256));
	if (pDstImage == NULL) {
		perror("Allocating destination image");
		exit(-2);
	}

	// Pointers to the componet arrays of the source images
	filter_args.pRsrc1 = srcImage1.data(); // pRcomp points to the R component array
	filter_args.pGsrc1 = filter_args.pRsrc1 + filter_args.pixelCount; // pGcomp points to the G component array
	filter_args.pBsrc1 = filter_args.pGsrc1 + filter_args.pixelCount; // pBcomp points to B component array
	filter_args.pRsrc2 = srcImage2.data(); 
	filter_args.pGsrc2 = filter_args.pRsrc2 + filter_args.pixelCount; 
	filter_args.pBsrc2 = filter_args.pGsrc2 + filter_args.pixelCount; 

	// Pointers to the RGB arrays of the destination image
	filter_args.pRdst = pDstImage;
	filter_args.pGdst = filter_args.pRdst + filter_args.pixelCount;
	filter_args.pBdst = filter_args.pGdst + filter_args.pixelCount;


	/***********************************************
	 *   - Measure initial time
	 */
	struct timespec tStart, tEnd;
	double dElapsedTimeS;

	if (clock_gettime(CLOCK_REALTIME, &tStart) == -1){
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}

	/************************************************
	 * Algorithm.
	 */
	filter(filter_args);




	/************************************************
	 *   - Measure the end time
	 *   - Calculate the elapsed time
	 */
	if (clock_gettime(CLOCK_REALTIME, &tEnd) == -1)
	{
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}


	// Show the elapsed time
	dElapsedTimeS = (tEnd.tv_sec - tStart.tv_sec);
        dElapsedTimeS += (tEnd.tv_nsec - tStart.tv_nsec) / 1e+9;
	printf("Elapsed time    : %f s.\n", dElapsedTimeS);


	// Create a new image object with the calculated pixels
	// In case of normal color images use nComp=3,
	// In case of B/W images use nComp=1.
	CImg<data_t> dstImage(pDstImage, width1, height1, 1, nComp1);

	// Store destination image in disk
	dstImage.save(DESTINATION_IMG); 

	// Display destination image
	dstImage.display();
	
	// Free memory (with intrinsic function)
	_mm_free(pDstImage);

	return 0;
}
