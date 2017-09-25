#ifndef _CSPECTRUMANALYZER_H
#define _CSPECTRUMANALYZER_H
#include <SDL.h>
#include <deque>
#include <vector>
#include "CCriticalSection.h"
#include "FastFourierTransform.h"
extern "C"{
#include <libavcodec/avfft.h>
}
#include "fftw/rfftw.h"
using namespace std;

struct SpectrumInfo{
	short l_amplitude, r_amplitude;
	int l_dB, r_dB;
	vector<double> spectrum;
};

class CSpectrumAnalyzer{
private:
	bool stop;
	deque<vector<short>> bufferQueue;
	vector<short> buffer;
	vector<double> output;

	int channel_num;
	int bands;
	CCriticalSection lock;
	void Analyze();
	static int AnalysisThread(void* param);
public:
	CSpectrumAnalyzer() :  channel_num(0), bands(0){}
	~CSpectrumAnalyzer(){stop = true;}
	void Start(int channels, int bd);
	vector<double> GetResult(){return output;} 
	void Stop(){stop = true;}
	void PushData(const short* data, const int size);
};
#endif