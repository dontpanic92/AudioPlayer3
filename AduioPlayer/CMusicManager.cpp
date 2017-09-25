#include "CMusicManger.h"

void CMusicManager::LoadLrc(const char* filename){
	char lrcname[260];
	strcpy_s(lrcname, filename);
	char* pos = strrchr(lrcname, '.');
	*(++pos) = 'l';
	*(++pos) = 'r';
	*(++pos) = 'c';
	*(++pos) = '\0';

	lrc.clear();

	locale &loc=locale::global(locale(locale(),"",LC_CTYPE));
	wifstream lrcFile(lrcname);
	if (lrcFile.fail()){
		char* pos = strrchr(lrcname, '\\');
		char name[260];
		strcpy_s(name,(++pos));
		sprintf_s(pos, 200 ,"lyric\\%s", name);
		lrcFile.open(lrcname);
		if (lrcFile.fail()){
			strcpy_s(lrcname, filename);
			char* pos = strrchr(lrcname, '\\');
			sprintf_s(pos, 200 ,"\\..\\lyric\\%s", name);
			lrcFile.open(lrcname);
		}
	}
	locale::global(loc);

	wstring temp;
	while(getline(lrcFile, temp)){
		bool readMinute = false, readSecond = false, skip = false;
		Lrc nowLrc;
		string minute, second;
		for (int i = 0; i < temp.size(); i++){
			if (temp[i] == '['){
				readMinute = true;
				continue;
			}else if (temp[i] == ']'){
				readMinute = false;
				readSecond = false;
				continue;
			}
			if (readMinute){
				if (temp[i] == ':'){
					readMinute = false;
					readSecond = true;
					continue;
				}else if (temp[i] < '0' || temp[i] > '9'){
					skip = true;
					break;
				}
				minute += temp[i];
				continue;
			}else if (readSecond){
				if ((temp[i] < '0' || temp[i] > '9') && temp[i] != '.'){
					skip = true;
					break;
				}
				second += temp[i];
				continue;
			}else{
				nowLrc.lrc = temp.substr(i, temp.size());
				break;
			}
		}
		if (skip)
			continue;
		int iminute = atoi(minute.c_str());
		double isecond = atof(second.c_str());
		nowLrc.lrc_time = iminute * 60 + isecond;
		lrc.push_back(nowLrc);
	}
	lrcFile.close();
}

void CMusicManager::GetLrc(){
	if (lrc.size() <= 0){
		Lrc fakeLrc;
		fakeLrc.lrc = L"* No Lyric *";
		fakeLrc.lrc_time = 0;
		fakeLrc.next_time = 0;
		ps.lrc = fakeLrc;	
		return;
	}
	double now = ap.GetClock();
	double nearestLrcTime = ap.GetTotalLength();
	int index = -2;
	for (int i = 0; i < lrc.size(); i++){
		if (lrc[i].lrc_time > now){
			index = i - 1;
			break;
		}
	}
	if (index == -1){
		ps.lrc.lrc = L"";
		ps.lrc.lrc_time = 0;
		ps.lrc.next_time = lrc[0].lrc_time;
		return;
	}
	if (index == -2){
		index = lrc.size() - 1;
		ps.lrc = lrc[index];
		ps.lrc.next_time = ps.lrc.lrc_time + 10;
		return;
	}
	ps.lrc = lrc[index];
	ps.lrc.next_time = lrc[index + 1].lrc_time;
}
PlayState CMusicManager::GetState(){
	if (ap.GetName())
		ps.Name = ap.GetName();
	else
		ps.Name = "";
	ps.now = ap.GetPosition();
	ps.total = ap.GetTotalLength();
	ps.volume = ap.GetVolume();
	if ((ps.state = ap.GetStatus()) == FINISHED){
		PlayNext();
	}
	ps.l_dB = ap.GetldB();
	ps.r_dB = ap.GetrdB();
	ps.l_amplitude = ap.GetlAmp();
	ps.r_amplitude = ap.GetrAmp();
	_itoa_s(ap.GetQueueSize() / 1024, ps.aq, 10);
	GetLrc();
	ps.spectrum = SpectrumAnalyzer.GetResult();
	return ps;
}
void CMusicManager::LoadPlayList(){
	locale &loc=locale::global(locale(locale(),"",LC_CTYPE));
	wifstream ifile("PlayList.list");
	locale::global(loc);

	ifile>>type;ifile.get();

	wstring temp;
	while (getline(ifile, temp)){
		PlayList[0].push_back(temp);
	}
	ifile.close();

}
int CMusicManager::PlayNext(){
	if (type == SHUFFLE){
		int index = rand() % PlayList[0].size();
		return Play(index);
	}
	
	return 1;
}
