#pragma once
#include <cmath>
#include <cstdlib>

#ifndef INCLUDE_FASTTOURIERTRANSFORM
#define INCLUDE_FASTTOURIERTRANSFORM

/************************************************************************/
/* CFastFourierTransform                                                */
/************************************************************************/
#define PI_2 6.283185F
#define PI   3.1415925F
class CFastFourierTransform
{
private:
	double* xre;
	double* xim;
	double* mag;
	double* fftSin;
	double* fftCos;
	int* fftBr;
	int ss;
	int ss2;
	int nu;
	int nu1;

	int BitRev(int j, int nu);
	void PrepareFFTTables();
public:
	CFastFourierTransform(int pSampleSize);
	~CFastFourierTransform(void);

	double* Calculate(double* pSample, size_t pSampleSize);
};

#endif
