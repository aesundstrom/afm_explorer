#include "highgui.h"
#include "cv.h"
#include "cxcore.h"

//______________________________________________________________________________
//_________INIZIO CVTHIN________________________________________________________
static const char HIT  = 0x01;
static const char MISS = 0x02;

static const int numkernels = 8;
static const unsigned char kernels[][ 9 ] =
{
  {  2,  2,  2,
	 0,  1,  0,
	 1,  1,  1 },
  {  0,  2,  2,
	 1,  1,  2,
	 0,  1,  0 },
  {  1,  0,  2,
	 1,  1,  2,
	 1,  0,  2 },
  {  0,  1,  0,
	 1,  1,  2,
	 0,  2,  2 },
  {  1,  1,  1,
	 0,  1,  0,
	 2,  2,  2 },
  {  0,  1,  0,
	 2,  1,  1,
	 2,  2,  0 },
  {  2,  0,  1,
	 2,  1,  1,
	 2,  0,  1 },
  {  2,  2,  0,
	 2,  1,  1,
	 0,  1,  0 }
};


static bool applykernel ( const unsigned char *sptr, int widthstep,
						  const unsigned char *kernel )
{
  bool match = true;
  const unsigned char *ptr = sptr - widthstep - 1;
  int idx = 0;

  for( int j = 0 ; match && j < 3 ; ++j )
  {
	for( int i = 0 ; match && i < 3 ; ++i )
	{
      match = ( ! kernel[ idx ] ||
                (   *ptr && ( kernel[ idx ] & HIT  ) ) ||
                ( ! *ptr && ( kernel[ idx ] & MISS ) ) );

	  ++idx;
	  ++ptr;
	}
	ptr += widthstep - 3;
  }

  return match;
}


int cvHitAndMiss ( const IplImage *src, IplImage *dst,
                   const unsigned char *kernel )
{
  int count = 0;
  const unsigned char *sptr;
  unsigned char *dptr;
  for( int j = 1 ; j < src->height - 1 ; ++j )
  {
	sptr = ( unsigned char * )src->imageData + j * src->widthStep;
	dptr = ( unsigned char * )dst->imageData + j * dst->widthStep;

	for( int i = 1 ; i < src->width - 1 ; ++i )
	{
	  if( applykernel( sptr, src->widthStep, kernel ) )
		*dptr = 0;
	  else
		*dptr = *sptr;

	  if( *dptr )
		++count;

	  ++sptr;
	  ++dptr;
	}
  }

  return count;
}


void cvThin ( const IplImage *src, IplImage *dst )
{
  if( src->depth != IPL_DEPTH_8U  ||
	  src->nChannels != 1         ||
	  dst->depth != IPL_DEPTH_8U  ||
	  dst->nChannels != 1         ||
	  src->width != dst->width    ||
	  src->height != dst->height  )
	return;

  int result, lastresult;
  result = cvHitAndMiss( src, dst, kernels[ 0 ] );

  int iter = 1;
  int lastmod = 0;
  int k = 1;
  IplImage *a = dst;
  IplImage *b = dst;

  while( iter < lastmod + numkernels + 1 )
  {
	lastresult = result;
	result = cvHitAndMiss( a, b, kernels[ k ] );
	if( result != lastresult )
	  lastmod = iter;

	++k;
	if( k >= numkernels )
	  k = 0;

	++iter;
  }
}
void bubbleSort(short x[],short y[]) {
   for (int j=1;j<9;j++)
	   for (int i=0;i<9-j;i++)
		   if (x[i]>x[i+1])	{
			   int temp=x[i];
			   x[i]=x[i+1];
			   x[i+1]=temp;
			   temp = y[i];
			   y[i]=y[i+1];
			   y[i+1]=temp;
		   }
}
//_____FINE CVTHIN______________________________________________________________
//______________________________________________________________________________
