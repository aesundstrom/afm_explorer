#ifndef COGLIONAZZO
#define COGLIONAZZO

#include "highgui.h"
#include "cv.h"
#include "cxcore.h"

int cvHitAndMiss ( const IplImage *src, IplImage *dst,
                   const unsigned char *kernel );

void cvThin ( const IplImage *src, IplImage *dst );

void bubbleSort(short x[],short y[]);

#endif
