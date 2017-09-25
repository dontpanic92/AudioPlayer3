#ifndef _CCONSOLEUI_H
#define _CCONSOLEUI_H
//#include <Windows.h>
#include <iostream>
#include "stdafx.h"
#include "CMusicManger.h"
#include "CConsoleWindowManager.h"

#define MSG_UPSTATE WM_USER


class CConsoleUI{
private:
	DWORD MainId;
	HANDLE hTimeThread;
	bool abort;
	PlayState state;
	COORD MousePos;
	CMusicManager* pMusicManager;
	CConsoleWindowManager* pWndManager;
	HCONWND hWnd;

	static int MainWndProc(int,int,int,void*);
	static int OpenFileProc(int,int,int,void*);

	void DrawAll();
	void DrawTitle();
	void DrawdBInfo();
	void DrawButton(){COORD c = {0,0};MouseProc(&c,0);};

	void UpdateInfo();
	void UpdateName();
	void UpdatedBInfo();
	void UpdateSpectrum();
	void UpdateTime();
	void UpdateLrc();

	void PlayIt(const char* filename);
	void MouseProc(COORD* coord, int leftDown);
public:
	CConsoleUI(CMusicManager* p) : pMusicManager(p),abort(false), MainId(0),hTimeThread(NULL){
		AllocConsole();
		FILE* s;
		freopen_s(&s,"CONIN$", "r+t", stdin);
		freopen_s(&s,"CONOUT$", "w+t", stdout);
		SetConsoleTitle(TEXT("Audio Player"));
		pWndManager = new CConsoleWindowManager();
		memset(&state, 0, sizeof(PlayState));
		MousePos.X = MousePos.Y = 0;
		pMusicManager->SetSpectrumBands(25);
	}
	int WaitForReturn();
	~CConsoleUI(){
		if (hTimeThread){
			TerminateThread(hTimeThread, 0);
		}
		if (pWndManager){
			delete pWndManager;
		}
		FreeConsole();
	}
};
#endif
