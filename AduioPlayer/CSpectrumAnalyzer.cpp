#include "CSpectrumAnalyzer.h"

//#include <Windows.h>
#include <iostream>
using namespace std;
void CSpectrumAnalyzer::Start(int channels, int bd){
	channel_num = channels;
	bands = bd;
	stop = false;
	SDL_CreateThread(AnalysisThread, this);
}
int CSpectrumAnalyzer::AnalysisThread(void* param){
	CSpectrumAnalyzer* pMe = (CSpectrumAnalyzer*)param;
	pMe->Analyze();
	return 0;
}

/*void CSpectrumAnalyzer::Analyze(){
	while(!stop){
		SDL_Delay(10);
		if(bufferQueue.size() <= 0){
			continue;
		}

		vector<short> in = bufferQueue.back();
		bufferQueue.pop_back();
		if (in.empty()){
			continue;
		}

	int i, i_start, x, y1, y, ys, delay, n, nb_display_channels;
    int ch, channels, h, h2, bgcolor, fgcolor;
    int16_t time_diff;
    int rdft_bits, nb_freq;

	for(rdft_bits=1; (1<<rdft_bits)<2*in.size(); rdft_bits++)
        ;
    nb_freq= 1<<(rdft_bits-1);
        int data_used= (2*nb_freq);
		channels = channel_num;
        n = 2 * channels;

            RDFTContext* fft  = av_rdft_init(rdft_bits, DFT_R2C);
            FFTSample* rdft_data= new FFTSample[4*nb_freq];
        
            FFTSample *data;
                data = rdft_data ;
                i = 0;
                for(x = 0; x < 2*nb_freq; x++) {
                    double w= (x-nb_freq)*(1.0/nb_freq);
                    data[x]= in[i] / 32767.0;//in[i]*(1.0-w*w);
                    i += channels;
					if (i >= in.size())
                        i -= in.size();
                }
                av_rdft_calc(fft, data);

				vector<double> buffer;
		for (int a = 0,  bd = 0; bd < bands; a += (INT)((double)in.size() / bands) / 2, bd++) {
			double wFs = 0;
			for (int b = 0; b < (INT)((double)in.size() / bands) / 2; b++) {
				//wFs += pIn[a + b];
				double w= 1/sqrt((double)nb_freq);
				wFs += sqrt(w*sqrt(data[2*(a+b)+0]*data[2*(a+b)+0] + data[2*(a+b)+1]*data[2*(a+b)+1]));
			}

			wFs /= ((double)in.size() / bands) /2;
			if (wFs < 0.02)
				wFs *= 10.0;
			if (wFs > 1.0)
				wFs = 1.0;
			buffer.push_back(wFs);
		}
		output = buffer;
	
			delete[] rdft_data;
			av_rdft_end(fft);
	} 
}*/
void CSpectrumAnalyzer::Analyze(){
	while(!stop){
		SDL_Delay(10);
		//if(bufferQueue.size() <= 0){
			//continue;
		//}

		lock.Lock();
		//vector<short> in = bufferQueue.back();
		//bufferQueue.pop_back();
		vector<short> in = buffer;
		lock.Unlock();
		if (in.empty()){
			continue;
		}

		long fftsamples;
		fftsamples = 1024 / channel_num;


		//int inputsamples = in.size() / channel_num / 2;
		//if (fftsamples >= inputsamples) {
			//fftsamples = inputsamples;
		//}

		//fftsamples /= 2;
		//fftsamples *= 2;


		rfftw_plan plan = rfftw_create_plan(fftsamples, FFTW_FORWARD, FFTW_ESTIMATE);
		double  *pIn = NULL;
		double *pOut = NULL;
		vector<double> buffer;

		pIn = new double[fftsamples];
		pOut = new double[fftsamples];
		for(int _inIndex = 0, inIndex = 0; _inIndex < fftsamples; _inIndex++, inIndex += channel_num) {
			if (channel_num == 2)
				pIn[_inIndex] = (in[inIndex] + in[inIndex+1]) / 32767.0 / 2.0;
			else
				pIn[_inIndex] = in[inIndex] / 32767.0;
		}
		rfftw_one(plan, pIn, pOut);
		//CFastFourierTransform FFT(fftsamples);
		//pOut = FFT.Calculate(pIn, fftsamples);
		for (int a = 0,  bd = 0; bd < bands; a += (INT)((double)fftsamples / bands) /2, bd++) {
			double wFs = 0;
			for (int b = 0; b < (INT)((double)fftsamples / bands) /2; b++) {
				wFs += pOut[a + b];
			}
			
		//if(wFs <= 0.01F)
			//wFs *= 10.0F;
		//else
			//wFs *= PI;
			wFs /= ((double)fftsamples / bands) /2;
			if (wFs < 0.02)
				wFs *= 10.0;
			if (wFs > 1.0)
				wFs = 1.0;
			buffer.push_back(wFs);
		}
		output = buffer;
		delete[] pIn;
		delete[] pOut;
		rfftw_destroy_plan(plan);
	}
}


void CSpectrumAnalyzer::PushData(const short* data, const int size){
	vector<short> buf;
	//int s = ((size > 1024) ? 1024 : size);
	for (int i = 0; i < size; i++){
		buf.push_back(data[i]);
	}

	lock.Lock();
	//if (bufferQueue.size() > 1)
		//bufferQueue.pop_back();
	//bufferQueue.push_front(buf);
	//bufferQueue.clear();
	//bufferQueue.push_back(buffer);
	buffer = buf;
	lock.Unlock();
}
