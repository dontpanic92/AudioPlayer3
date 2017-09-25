#ifndef _CMUSICMANAGER_H
#define _CMUSICMANAGER_H
#include "CAudioPlayer.h"
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <io.h>
//#include <Windows.h>

using namespace std;

struct Lrc{
	double lrc_time;
	double next_time;
	wstring lrc;
};
struct PlayState{
	string Name;
	long now;
	long total;
	long volume;
	int state;
	int l_dB;
	int r_dB;
	short l_amplitude;
	short r_amplitude;
	char aq[10];
	vector<double> spectrum;
	Lrc lrc;
};

enum playtype {ONCE, ONCE_REPEAT, ORDER, SHUFFLE, ALL_REPEAT};

class CMusicManager{
	CAudioPlayer ap;
	CSpectrumAnalyzer SpectrumAnalyzer;
	vector<vector<wstring>> PlayList;
	int ListId, SongId;
	int type;
	PlayState ps;
	vector<Lrc> lrc;
	int bands;

	void LoadLrc(const char* filename);
	void GetLrc();
	void LoadPlayList();
	void SavePlayList(){}
public:
	CMusicManager() : ListId(0), SongId(0), type(0){
		ap.SetSpectrumAnalyzer(&SpectrumAnalyzer);
		srand(time(NULL));
		vector<wstring> temp;
		PlayList.push_back(temp);
		LoadPlayList();
	}
	~CMusicManager(){}
	int Play(int index){
		char temp[260];
		wsprintfA(temp,"%ls", PlayList[0][index]);
		return Play(temp);
	}
	int PlayNext();
	int Play(const char* filename){
		SpectrumAnalyzer.Stop();
		ap.Close();
		int ret = ap.Play(filename);
		if (ret == 0){
			LoadLrc(filename);
			SpectrumAnalyzer.Start(ap.GetChannelNum(), bands);
		}
		return ret;
	}
	void Pause(){ap.Pause();}
	void Resume(){ap.Resume();}
	void Stop(){ap.Stop();}
	void SetSpectrumBands(int bd){bands = bd;};
	PlayState GetState();
	void Seek(int seconds){
		ap.SetPosition(seconds);
	}
};
#endif

