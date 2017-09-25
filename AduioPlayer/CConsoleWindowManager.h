#ifndef _CCONSOLEWINDOWMANAGER_H
#define _CCONSOLEWINDOWMANAGER_H
//#include <Windows.h>
#include "stdafx.h"

#define MAX_WINDOW_NUM 3
#define CONSOLE_SIZE_X 80
#define CONSOLE_SIZE_Y 25

typedef int HCONWND;
typedef int (*PCONSOLEWNDPROC)(int,int,int,void*);

#define CWM_PAINT			(WM_USER)
#define CWM_KEYDOWN			(WM_USER + 1)
#define CWM_KEYUP			(WM_USER + 2)
#define CWM_MOUSEMOVE		(WM_USER + 3)
#define CWM_LBUTTONDOWN		(WM_USER + 4)
#define CWM_LBUTTONUP		(WM_USER + 5)

struct ConsoleWndInfo{
	COORD pos;
	COORD size;
	bool captureInput;
	void* userData;
};

class CConsoleTool{
public:
	static void ButtonStateSwitch(const char* caption,const COORD coord, int state);
};

class CMessageBox{
public:
	bool exit;
	CMessageBox(){exit = false;}
	static int Proc(int Event,int Param1, int Param2, void* userData);
	void ShowMessage(const wchar_t* Msg);
};

class CConsoleWindowManager{
	HANDLE hCon;
	HANDLE hIn;
	HANDLE hClockThread;
	HANDLE hInputThread;

	bool clockTerm;
	int WndNum;	
	PCONSOLEWNDPROC WndProc[MAX_WINDOW_NUM];
	ConsoleWndInfo WndInfo[MAX_WINDOW_NUM];
	CRITICAL_SECTION cs;

	int oldNum;
	static DWORD WINAPI ClockThread(LPVOID lpParam);
	static DWORD WINAPI InputThread(LPVOID lpParam);
public:
	CConsoleWindowManager() : hCon(NULL), WndNum(0),oldNum(0), hClockThread(NULL), clockTerm(false){
		ReopenConsole();
		memset(WndProc, NULL, sizeof(DWORD) * MAX_WINDOW_NUM);
		hClockThread = CreateThread(NULL, 0, ClockThread, this, 0, NULL);
		hInputThread = CreateThread(NULL, 0, InputThread, this, 0, NULL);
	}
	~CConsoleWindowManager(){
		clockTerm = true;
		WaitForSingleObject(hClockThread, 100);
		TerminateThread(hClockThread, 0);
		TerminateThread(hInputThread, 0);
	}
	void ReopenConsole(){
		hCon = GetStdHandle(STD_OUTPUT_HANDLE);
		hIn = GetStdHandle(STD_INPUT_HANDLE);
	}

	HCONWND CreateConsoleWindow(PCONSOLEWNDPROC Proc, void* userData){
		COORD pos = {0, 0};
		COORD size = {80, 25};
		ConsoleWndInfo info = {pos, size, true, userData};
		return CreateConsoleWindow(info, Proc);
	}
	HCONWND CreateConsoleWindow(ConsoleWndInfo info, PCONSOLEWNDPROC Proc){	
		if (WndNum >= MAX_WINDOW_NUM){
			return 0;
		}
		WndNum++;
		WndInfo[WndNum-1] = info;
		WndProc[WndNum-1] = Proc;
		return WndNum;
	}
	
	void CloseConsoleTopWindow(){
		if (WndNum <= 0)
			return;
		WndProc[WndNum-1] = NULL;
		WndNum--;
	}

	void ShowMessage(const wchar_t* msg){
		CMessageBox MsgBox;
		PCONSOLEWNDPROC pOriFun = WndProc[WndNum-1];
		ConsoleWndInfo info = WndInfo[WndNum-1];
		void* pThis = WndInfo[WndNum-1].userData;

		WndInfo[WndNum-1].userData = &MsgBox;
		WndInfo[WndNum-1].captureInput = true;
		WndProc[WndNum-1] = CMessageBox::Proc;
		MsgBox.ShowMessage(msg);
		WndInfo[WndNum-1] = info;
		WndProc[WndNum-1] = pOriFun;
	}

};
#endif
