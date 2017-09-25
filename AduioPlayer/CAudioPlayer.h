#ifndef _CAUDIOPLAYER_H
#define _CAUDIOPLAYER_H
#include "CSpectrumAnalyzer.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <SDL.h>
#include "libswresample/swresample.h"
}

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"SDL.lib")

enum _AudioErr{
	ERR_NO_OPEN = 1,
	ERR_OPEN_FAILED, 
	ERR_STREAM_NOT_FOUND, 
	ERR_UNSUPPORT_CODEC, 
	ERR_INVALID_RATE_CH,
	ERR_OPEN_CODEC, 
	ERR_SDL
};

struct PacketQueue{
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	int abort_request;
	SDL_mutex* mutex;
	SDL_cond* cond;
};

enum _status{PLAY, PAUSE, STOP, FINISHED};

class CAudioPlayer{
private:
	AVFormatContext* pFormatCtx;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;
	PacketQueue queue;
	int audioStream;
	SDL_AudioSpec wanted_spec, spec;
	SDL_Thread* hPlayThread;
	AVPacket flush_pkt;
	struct VideoState{
		SwrContext *swr_ctx;
		AVSampleFormat audio_src_fmt;
		AVSampleFormat audio_tgt_fmt;
		int audio_src_channels;
		int audio_tgt_channels;
		int64_t audio_src_channel_layout;
		int64_t audio_tgt_channel_layout;
		int audio_src_freq;
		int audio_tgt_freq;	
		AVFrame* pFrame;
		AVPacket audio_pkt_temp;
		AVPacket audio_pkt;
		uint8_t *audio_buf;
		uint8_t *audio_buf1;
		int audio_hw_buf_size;
		DECLARE_ALIGNED(16,uint8_t,audio_buf2)[AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];
		uint8_t silence_buf[1024];
		unsigned int audio_buf_size; /* in bytes */
		int audio_buf_index; /* in bytes */
		int audio_write_buf_size;
		double audio_current_pts;
		double audio_current_pts_drift;
		double audio_clock;
		int paused;
		int seek_req;
		int64_t seek_pos;
		double seek_second;
		short l_amplitude, r_amplitude;
		int l_dB, r_dB;
		int refresh_clock;
	}as;
	int64_t audio_callback_time;
	int volume;
	CSpectrumAnalyzer* pSpectrumAnalyzer;

	int stream_component_open();
	void stream_component_close();
	void InitPacketQueue();
	void FlushPacketQueue();
	void AbortPacketQueue();
	void EndPacketQueue();
	int PutPacket(AVPacket* pkt);
	int GetPacket(AVPacket *pkt, int block);
	int AudioDecodeFrame(double *pts_ptr);
	static int PlayThread(void* param);
	static void CallBack(void *opaque, Uint8 *stream, int len);
	static int decode_inte(void* param){
		CAudioPlayer* p = (CAudioPlayer*)param;
		return p->queue.abort_request;
	}
	int status;
public:
	CAudioPlayer():pFormatCtx(NULL), pCodecCtx(NULL), pCodec(NULL),
			pSpectrumAnalyzer(NULL), audioStream(-1), status(STOP), volume(SDL_MIX_MAXVOLUME){
		av_register_all();
		av_init_packet(&flush_pkt);
		flush_pkt.data = (uint8_t*)"FLUSH";
	}
	~CAudioPlayer(){}
	int Open(const char* filename);
	int Play(){
		if (!pFormatCtx)
			return ERR_NO_OPEN;
		hPlayThread = SDL_CreateThread(PlayThread, this);
		return 0;
	}
	int Play(const char* filename){
		if (pFormatCtx)
			Close();
		int ret;
		if ((ret = Open(filename)) != 0){
			Close();
			return ret;
		};
		return Play();
	}
	int GetStatus(){return status;}
	void Close();
	void Pause(){status = PAUSE; as.paused = 1;}
	void Resume(){status = PLAY; as.paused = 0;}
	void Stop() {Close();}
	void Flush(){FlushPacketQueue();};
	int GetVolume() {return int((double) volume / 128 * 100);}
	void SetVolume(int num){
		int temp = int((double) num / 100 * 128);
		if (temp > 128)
			volume = SDL_MIX_MAXVOLUME;
		else if (temp < 0)
			volume = 0;
		else volume = temp;
	}
	void IncreaseVolume(int vol) {SetVolume(volume + vol);}
	void DecreaseVolume(int vol) {SetVolume(volume - vol);}
	void IncreasePosition(int pos) {
		SetPosition(GetPosition() + pos);
	}
	void DecreasePosition(int pos) {
		SetPosition(GetPosition() - pos);
	}
	long GetTotalLength(){
		if (pFormatCtx){
			return long(pFormatCtx->streams[audioStream]->duration * pFormatCtx->streams[audioStream]->time_base.num / pFormatCtx->streams[audioStream]->time_base.den); 
		}
		return 0;
	}
	long GetPosition() {
		if (pFormatCtx){ 
			if (!as.refresh_clock)
				return long(as.audio_clock);
			else
				return long(as.seek_second);
		}else 
			return 0;
	}
	void SetPosition(int second) {
		SetPosition((double)second / (double)GetTotalLength());
	}
	void SetPosition(double percent) {
		if (!pFormatCtx)
			return;
		if (percent > 1)
			percent = 1;
		if (percent < 0)
			percent = 0;
		as.refresh_clock = 2;
		int64_t ts;
		ts = int64_t(percent * pFormatCtx->duration);
		as.seek_pos = ts;
		as.seek_second = ts / 1000000.0;
		as.seek_req = 1;
	}
	int GetldB(){if (pFormatCtx)return as.l_dB;else return -96;}
	int GetrdB(){if (pFormatCtx)return as.r_dB;else return -96;}
	short GetlAmp(){if (pFormatCtx)return as.l_amplitude;else return 0;}
	short GetrAmp(){if (pFormatCtx)return as.r_amplitude;else return 0;}
	int GetChannelNum(){if (pFormatCtx)return spec.channels;else return 0;}
	int GetQueueSize(){if (pFormatCtx)return queue.size;else return 0;}
	double GetClock(){if (pFormatCtx) return as.audio_clock; else return 0;}
	double GetDuration(){if (pFormatCtx)return (pFormatCtx->streams[audioStream]->duration * (double)pFormatCtx->streams[audioStream]->time_base.num / (double)pFormatCtx->streams[audioStream]->time_base.den);else return 0;}
	char* GetName(){if (pFormatCtx) return pFormatCtx->filename;else return NULL;}
	void SetSpectrumAnalyzer(CSpectrumAnalyzer* pSA){pSpectrumAnalyzer = pSA;};
};

#endif
