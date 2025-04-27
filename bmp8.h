#ifndef BMP8_H
#define BMP8_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>



typedef struct {

    unsigned char header[54];
    unsigned char colorTable[1024];
    unsigned char * data;
    unsigned int width;
    unsigned int height;
    unsigned int colorDepth;
    unsigned int dataSize;

}t_bmp8;


//debugging
void bmp8_compare_da_images();

//utils
t_bmp8 * bmp8_loadImage(const char * filename);
void bmp8_saveImage(const char * filename, t_bmp8 * img);
void bmp8_free(t_bmp8 * img);
void bmp8_printInfo(t_bmp8 * img, bool verbose);


//data processing
void bmp8_negative(t_bmp8 * img);
void bmp8_brightness(t_bmp8 * img, int value);
void bmp8_threshold(t_bmp8 * img, int threshold);

void bmp8_applyFilter(t_bmp8 * img, float ** kernel, int kernelSize);


//histogramme stuff
unsigned int * bmp8_computeHistogram(t_bmp8 * img);
unsigned int* bmp8_computeCDF(unsigned int* hist, unsigned int nb_of_pixels);
void bmp8_equalize(t_bmp8 * img);


#endif //BMP8_H
