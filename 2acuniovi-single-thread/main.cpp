/*
 * Main.cpp
 *
 *  Created on: Fall 2019
 */

#include <stdio.h>
#include <math.h>
#include <CImg.h>

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

/***********************************************
 * 
 * Algorithm. Image filter.
 * In this example, the algorithm is a components swap
 *
 * TO BE REPLACED BY YOUR ALGORITHM
 * 		
 * *********************************************/
void filter (filter_args_t args) {
    for (uint i = 0; i < args.pixelCount; i++) {
		float res_r, res_g, res_b;	// auxiliar vars to avoid overflow

		res_r = 255.0f - (256.0f * (255.0f - *(args.pRsrc2 + i)) / (*(args.pRsrc1 + i) + 1.0f));
		*(args.pRdst + i) = fmaxf(0.0f, fminf(255.0f, res_r));	// Limit the pixel value to [0, 255]
		res_g = 255.0f - (256.0f * (255.0f - *(args.pGsrc2 + i)) / (*(args.pGsrc1 + i) + 1.0f));
		*(args.pGdst + i) = fmaxf(0.0f, fminf(255.0f, res_g));
		res_b = 255.0f - (256.0f * (255.0f - *(args.pBsrc2 + i)) / (*(args.pBsrc1 + i) + 1.0f));
		*(args.pBdst + i) = fmaxf(0.0f, fminf(255.0f, res_b));
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
		exit(-2);
	} 

	// Calculating image size in pixels
	filter_args.pixelCount = width1 * height1;
	
	// Allocate memory space for destination image components
	pDstImage = (data_t *) malloc (filter_args.pixelCount * nComp1 * sizeof(data_t));
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
	
	// Free memory
	free(pDstImage);

	return 0;
}
