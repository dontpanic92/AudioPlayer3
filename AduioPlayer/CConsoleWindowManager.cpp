#include "CConsoleWindowManager.h"

void CConsoleTool::ButtonStateSwitch(const char* caption,const COORD coord, int state){
	CHAR_INFO buffer[20];
	memset(buffer, 0, sizeof(buffer));
	if (state == 0){
			for (int i = 0; i < strlen(caption); i++){
				buffer[i].Char.UnicodeChar = caption[i];
				buffer[i].Attributes = 135 | COMMON_LVB_GRID_HORIZONTAL;
			}
		buffer[0].Attributes |= COMMON_LVB_GRID_LVERTICAL;
	}else if (state == 1){
		for (int i = 0; i < strlen(caption); i++){
			buffer[i].Char.UnicodeChar = caption[i];
			buffer[i].Attributes = 247 | COMMON_LVB_UNDERSCORE;
		}
		buffer[strlen(caption)-1].Attributes |= COMMON_LVB_GRID_RVERTICAL;
	}else if (state == 2){
		for (int i = 0; i < strlen(caption); i++){
			buffer[i].Char.UnicodeChar = caption[i];
			buffer[i].Attributes = 143 | COMMON_LVB_UNDERSCORE;
		}
		buffer[strlen(caption)-1].Attributes |= COMMON_LVB_GRID_RVERTICAL;	
	}
	COORD bufferOrigin = {0,0}, bufferSize = {strlen(caption), 1};
	SMALL_RECT rectToWrite = {coord.X, coord.Y, coord.X + bufferSize.X, coord.Y};
	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),buffer, bufferSize, bufferOrigin, &rectToWrite);
}

int CMessageBox::Proc(int Event,int Param1, int Param2, void* userData){
	CMessageBox* pMe = (CMessageBox*)userData;
	const char caption[11] = "    OK    ";
	const COORD buttonCoord = {32, 13};
	COORD* coord = (COORD*)Param1;
	int leftDown = 0;
	switch (Event){
		case CWM_MOUSEMOVE:
			leftDown = 0;
			break;
		case CWM_LBUTTONDOWN:
			leftDown = 1;
		break;
		case CWM_LBUTTONUP:
			leftDown = 2;
		break;
		case CWM_KEYDOWN:
			if (Param1 == VK_RETURN)
				pMe->exit = true;
		default:
			return 0;
	}
	if (coord->X >= buttonCoord.X  && 
		coord->X <= buttonCoord.X + strlen(caption) &&
		coord->Y == buttonCoord.Y){
			if (leftDown == 0)
				CConsoleTool::ButtonStateSwitch(caption, buttonCoord,1);
			else if(leftDown == 1)
				CConsoleTool::ButtonStateSwitch(caption, buttonCoord,2);
			else{
				pMe->exit = true;
			}
	}else{
		CConsoleTool::ButtonStateSwitch(caption, buttonCoord,0);
	}
	return 0;
}

void CMessageBox::ShowMessage(const wchar_t* Msg){
	wchar_t buf[257] = L"©°Message Box ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©´\
©¦                                        ©¦\
©¦                                        ©¦\
©¦                                        ©¦\
©¦                                        ©¦\
©¸©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¼";

	CHAR_INFO buffer[257];
	int i;
	for (i = 0;i < 257; i++){
		buffer[i].Char.UnicodeChar = buf[i];
		buffer[i].Attributes = 27;
	}
	COORD bufferOrigin = {0,0}, bufferSize = {44, 6};
	SMALL_RECT rectToWrite = {15, 9, 80, 14};
	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),buffer, bufferSize, bufferOrigin, &rectToWrite);
	bool temp = false;
	for (i = 0; i < 257 ; i++){
		if (Msg[i] == '\0')
			temp = true;
		if (temp)
			buffer[i].Char.UnicodeChar = ' ';
		else
			buffer[i].Char.UnicodeChar = Msg[i];
		buffer[i].Attributes = 27;
	}	
	Proc(CWM_MOUSEMOVE, (int)&bufferOrigin, 0, this);
	COORD bufferOrigin2 = {0,0}, bufferSize2 = {31, 3};
	SMALL_RECT rectToWrite2 = {22, 10, 80, 13};
	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),buffer, bufferSize2, bufferOrigin2, &rectToWrite2);

	while (!exit){
		Sleep(50);
	}
}

DWORD WINAPI CConsoleWindowManager::ClockThread(LPVOID lpParam){
	CConsoleWindowManager* pMe = (CConsoleWindowManager*)lpParam;
	while (!pMe->clockTerm){
		Sleep(50);
		COORD setps;
		setps.X = 0;
		setps.Y = 24;
		SetConsoleCursorPosition(pMe->hCon,setps);
		if (pMe->WndNum > 0){ 
			if (pMe->oldNum != pMe->WndNum){
				pMe->oldNum = pMe->WndNum;
				pMe->WndProc[pMe->WndNum-1](CWM_PAINT, 1, 0, pMe->WndInfo[pMe->WndNum-1].userData);	
			}else
				pMe->WndProc[pMe->WndNum-1](CWM_PAINT, 0, 0, pMe->WndInfo[pMe->WndNum-1].userData);
			}
	}
	return 0;
}
DWORD WINAPI CConsoleWindowManager::InputThread(LPVOID lpParam){
	CConsoleWindowManager* pMe = (CConsoleWindowManager*)lpParam;
	INPUT_RECORD InRec;
	DWORD NumRead;
	bool LPressed = false;
	while (!pMe->clockTerm){
		if (!pMe->WndInfo[pMe->WndNum-1].captureInput){
			Sleep(100);
			continue;
		}
		ReadConsoleInput(pMe->hIn,&InRec,1,&NumRead);
		if (InRec.EventType == KEY_EVENT){
			if (InRec.Event.KeyEvent.bKeyDown == TRUE){
				pMe->WndProc[pMe->WndNum-1](CWM_KEYDOWN, InRec.Event.KeyEvent.wVirtualKeyCode, 0, pMe->WndInfo[pMe->WndNum-1].userData);
				continue;
			}else{
				pMe->WndProc[pMe->WndNum-1](CWM_KEYUP, InRec.Event.KeyEvent.wVirtualKeyCode, 0, pMe->WndInfo[pMe->WndNum-1].userData);
				continue;				
			}
		}else if (InRec.EventType == MOUSE_EVENT){
			switch(InRec.Event.MouseEvent.dwEventFlags){
			case 0:
				if(InRec.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED){
					pMe->WndProc[pMe->WndNum-1](CWM_LBUTTONDOWN,(int)&InRec.Event.MouseEvent.dwMousePosition , 0, pMe->WndInfo[pMe->WndNum-1].userData);
					LPressed = true;
				}else{
					if (LPressed == true){
						LPressed = false;
						pMe->WndProc[pMe->WndNum-1](CWM_LBUTTONUP,(int)&InRec.Event.MouseEvent.dwMousePosition , 0, pMe->WndInfo[pMe->WndNum-1].userData);
					}
				}
				break;
			case MOUSE_MOVED:
					pMe->WndProc[pMe->WndNum-1](CWM_MOUSEMOVE,(int)&InRec.Event.MouseEvent.dwMousePosition , 0, pMe->WndInfo[pMe->WndNum-1].userData);
					break;
			}
			
			continue;
		}
	}
	return 0;
}
