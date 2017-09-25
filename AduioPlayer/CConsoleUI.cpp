#include "CConsoleUI.h"
#include <sstream>
int CConsoleUI::WaitForReturn(){
	MainId = GetCurrentThreadId();
	hWnd = pWndManager->CreateConsoleWindow(MainWndProc, this);   
	MSG messages;
	PeekMessage(&messages, NULL, 0, 0, PM_NOREMOVE);
	while (GetMessage(&messages, NULL, 0, 0) && messages.message != WM_QUIT){
		switch(messages.message){
			case MSG_UPSTATE:
				state = pMusicManager->GetState();
				break;
		}
	}
	return 1;
}

int CConsoleUI::MainWndProc(int Event, int Param1, int Param2, void* userData){
	CConsoleUI* pMe = (CConsoleUI*)userData;
	switch (Event){
	case CWM_PAINT:
		if (Param1)
			pMe->DrawAll();
		pMe->UpdateInfo();
		break;
	case CWM_MOUSEMOVE:
		pMe->MousePos = *(COORD*)Param1;
		pMe->MouseProc((COORD*)Param1,0);
		break;
	case CWM_LBUTTONDOWN:
		pMe->MouseProc((COORD*)Param1,1);
		break;
	case CWM_LBUTTONUP:
		pMe->MouseProc((COORD*)Param1,2);
		break;
	}
	return 0;
}
void CConsoleUI::MouseProc(COORD* coord, int leftDown){
	const int buttonNum = 3;
	static char caption[3][10] = {" Open... ", " Play! ", " Stop "};
	const COORD buttonCoord[3] = {{2,19}//open
								 ,{14,19}//play/pause
								 ,{24,19}//stop
								 };
	if (state.state == PLAY){
		strcpy_s(caption[1], " Pause ");
	}else if(state.state == PAUSE || state.state == STOP){
		strcpy_s(caption[1], " Play! ");						
	}
	for (int i = 0; i < buttonNum; i++){
		if (coord->X >= buttonCoord[i].X  && 
			coord->X <= buttonCoord[i].X + strlen(caption[i]) &&
			coord->Y == buttonCoord[i].Y){
				if (leftDown == 0)
					CConsoleTool::ButtonStateSwitch(caption[i], buttonCoord[i],1);
				else if(leftDown == 1)
					CConsoleTool::ButtonStateSwitch(caption[i], buttonCoord[i],2);
				else{
					switch(i){
					case 0:
						ConsoleWndInfo info;
						info.userData = this;
						info.captureInput = false;
						pWndManager->CreateConsoleWindow(info, OpenFileProc);
						break;
					case 1:
						if (state.state == PLAY){
							pMusicManager->Pause();
						}else if(state.state == PAUSE){
							pMusicManager->Resume();						
						}
						MouseProc(coord, 0);
						break;
					case 2:
						pMusicManager->Stop();
						break;
					}
				}
		}else{
			CConsoleTool::ButtonStateSwitch(caption[i], buttonCoord[i],0);
		}
	}
	if (leftDown == 1){
		if (MousePos.Y == 17 && MousePos.X >= 1 && MousePos.X <= 62){
			pMusicManager->Seek(int(state.total * (MousePos.X - 2) / 60.0));
		}
	}

}
void CConsoleUI::UpdateName(){
	string temp = state.Name.c_str();
	if (temp.empty())
		temp = "Audio Player";
	else
		temp.insert(0,"Audio Player  -  ");
	SetConsoleTitleA(temp.c_str());

	CHAR_INFO buffer[10];
	memset(&buffer, 0, sizeof(buffer));
	for (int i = 0; i < 10; i ++){
		if (state.aq[i] != '\0'){
			buffer[i].Char.UnicodeChar = state.aq[i]; 
			buffer[i].Attributes = 7 ;
		}else{
			buffer[i].Char.UnicodeChar = 'K'; 
			buffer[i].Attributes = 7;
			buffer[i+1].Char.UnicodeChar = 'B'; 
			buffer[i+1].Attributes = 7;
			break;
		}
	}

	COORD bufferOrigin = {0,0}, bufferSize = {10, 1};
	SMALL_RECT rectToWrite = {65, 1, 74, 1};
	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),buffer, bufferSize, bufferOrigin, &rectToWrite);
	
}
void CConsoleUI::UpdatedBInfo(){
	static int last_ldB = 0, last_rdB = 0;
	if (state.l_dB < -96)
		state.l_dB = -96;
	if (state.r_dB < -96)
		state.r_dB = -96;		
	if (state.l_dB > 0)
		state.l_dB = 0;
	if (state.r_dB > 0)
		state.r_dB = 0;	

	if (state.l_dB > last_ldB - 5)
		last_ldB = state.l_dB;
	else
		last_ldB -= 5;

	if (state.r_dB > last_rdB - 5)
		last_rdB = state.r_dB;
	else
		last_rdB -= 5;

	CHAR_INFO buffer[70 * 2];

	int i, n = static_cast<int>((last_ldB + 96) / 96.0 * 70.0);
	for (i = 0; i < n ; i ++){
		buffer[i].Char.UnicodeChar = ' ';
		buffer[i].Attributes = 135;// | COMMON_LVB_GRID_HORIZONTAL;
	}
	for (; i < 70; i ++){
		buffer[i].Char.UnicodeChar = ' '; 
		buffer[i].Attributes = 0;
	}
	buffer[0].Attributes |= COMMON_LVB_GRID_LVERTICAL;
	if (n > 0)
		buffer[n-1].Attributes |= COMMON_LVB_GRID_RVERTICAL;

	n = static_cast<int>((last_rdB + 96) / 96.0 * 70.0);	
	for (i = 70; i < n + 70 ; i ++){
		buffer[i].Char.UnicodeChar = ' '; 
		buffer[i].Attributes = 135 ;//| COMMON_LVB_UNDERSCORE;
	}
	for (; i < 140; i ++){
		buffer[i].Char.UnicodeChar = ' ';
		buffer[i].Attributes = 0;
	}
	buffer[70].Attributes |= COMMON_LVB_GRID_LVERTICAL;
	if (n > 0)
		buffer[n+69].Attributes |= COMMON_LVB_GRID_RVERTICAL;

	COORD bufferOrigin = {0,0}, bufferSize = {70, 2};
	SMALL_RECT rectToWrite = {7, 4, 77, 5};
	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),buffer, bufferSize, bufferOrigin, &rectToWrite);
}
void CConsoleUI::UpdateTime(){
	CHAR_INFO buffer[100];
	memset(buffer, 0, sizeof(buffer));
	buffer[0].Char.UnicodeChar = '|';
	buffer[0].Attributes = 120;

	int n;
	if (state.total == 0)
		n = 0;
	else
		n = int((double)state.now / (double)state.total * 60);

	int i;
	for (i = 1; i < n + 1; i++){
		buffer[i].Char.UnicodeChar = ' ';
		buffer[i].Attributes = 135 | COMMON_LVB_UNDERSCORE | COMMON_LVB_GRID_HORIZONTAL;
	}
	buffer[n+1].Char.UnicodeChar = '>';
	buffer[n+1].Attributes = 120;
	i++;
	for (; i < 61; i++){
		buffer[i].Char.UnicodeChar = ' ';
		buffer[i].Attributes = 0;// 128 | COMMON_LVB_UNDERSCORE | COMMON_LVB_GRID_HORIZONTAL | COMMON_LVB_REVERSE_VIDEO;	
	}
	buffer[61].Char.UnicodeChar = '|';
	buffer[61].Attributes = 120;

	bool MouseIn = false;
	if (MousePos.Y == 17 && MousePos.X >= 1 && MousePos.X <= 62){
		MouseIn = true;
		if (MousePos.X == 1)
			MousePos.X = 2;
		state.now = int(state.total * (MousePos.X - 2)  / 60.0);
		buffer[MousePos.X - 1].Char.UnicodeChar = '>';
		buffer[MousePos.X - 1].Attributes = 247;
	}

	char string[80];
	if (state.now % 60 < 10)
		sprintf(string,"%d:0%d/", state.now / 60, state.now % 60);
	else
		sprintf(string,"%d:%d/", state.now / 60,state.now % 60);
	
	if (state.total % 60 < 10)
		sprintf(string,"%s%d:0%d",string, state.total / 60, state.total % 60);
	else
		sprintf(string,"%s%d:%d",string, state.total / 60, state.total % 60);

	bool temp = false;
	for (unsigned i = 0; i < strlen(string); i++){
		buffer[65 + i].Char.UnicodeChar = string[i];
		buffer[65 + i].Attributes = 120 | COMMON_LVB_REVERSE_VIDEO;//143;
		if (MouseIn){
			if (string[i] == '/'){
				temp = true;
			}
			if (!temp){
				buffer[65 + i].Attributes = 137;
			}
		}
	}

	COORD bufferOrigin = {0,0}, bufferSize = {79, 1};
	SMALL_RECT rectToWrite = {1, 17, 80, 17};
	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),buffer, bufferSize, bufferOrigin, &rectToWrite);

}
void CConsoleUI::UpdateLrc(){
	static double time = 0;
	static int lastSize = 0;
	static int n = 0;
	char temp[200];
	wsprintfA(temp,"%ls", state.lrc.lrc.c_str());
	int x, limit, len = strlen(temp);
	if (len > 80)
		x = 0;
	else
		x = 40 - len / 2;

	if (state.lrc.lrc_time != time){
	//	if (n < lastSize){
	//		n = lastSize ;
	//	}else{
			n = 0;
			time = state.lrc.lrc_time;
			lastSize = state.lrc.lrc.size();
	//	}
	}

	if (state.lrc.next_time - state.lrc.lrc_time > 0.5){
		if (n < lastSize){
			n += int((lastSize / 5.0) + 0.5);
			limit = n;
		}else{
			//return;
		}
	}else
		limit = 80;

	CHAR_INFO buffer[100];
	memset(buffer, 0, sizeof(buffer));
	for (int i = 0; i < state.lrc.lrc.size() && i < 100 && i < limit; i++, x++){
		buffer[x].Char.UnicodeChar = state.lrc.lrc[i];
		buffer[x].Attributes = 7;
	}
	COORD bufferOrigin = {0,0}, bufferSize = {100, 1};
	SMALL_RECT rectToWrite = {0, 18, 80, 18};
	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),buffer, bufferSize, bufferOrigin, &rectToWrite);

}
void CConsoleUI::UpdateSpectrum(){
	static vector<double> last;
	static bool first = true;
	if (state.spectrum.size()>0){
		if (first){
			last.assign(25, 0);
			first = false;
		}
		CHAR_INFO buffer[1024];
		for (int k = 0; k < 25; k++){
			if (state.spectrum[k] > last[k] - 0.1)
				last[k] = state.spectrum[k];
			else {
				last[k] -= 0.1;
			}
			
			if (last[k] < 0)
				last[k] = 0;
		}
		int k = 0;
		for (int i = 0; i < 10; i++){	
			for(int j = 0; j < 25; j++){
				if (last[j] * 10 >=(10 - i)){
					wchar_t ch[] = L"¨€";
					buffer[k].Char.UnicodeChar = ch[0];
					buffer[k].Attributes = 7;
					buffer[k+1].Char.UnicodeChar = ' ';
					buffer[k+1].Attributes = 7;
					k+=2;
				}else{
					buffer[k].Char.UnicodeChar = ' ';
					buffer[k].Attributes = 0;
					buffer[k+1].Char.UnicodeChar = ' ';
					buffer[k+1].Attributes = 0;
					buffer[k+2].Char.UnicodeChar = ' ';
					buffer[k+2].Attributes = 0;
					k+=3;
				}
			}
		}
		COORD bufferOrigin = {0,0}, bufferSize = {75, 10};
		SMALL_RECT rectToWrite = {3, 6, 77, 15};
		WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),buffer, bufferSize, bufferOrigin, &rectToWrite);

	}

}
void CConsoleUI::UpdateInfo(){
	PostThreadMessage(MainId,MSG_UPSTATE,0,0);
	UpdateName();
	UpdatedBInfo();
	UpdateSpectrum();
	UpdateTime();
	UpdateLrc();
}
void CConsoleUI::DrawTitle(){
	wchar_t temp[180] = 
L"©°Music List©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©´\
©¦                                                                            ©¦\
©¸©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¼";
	CHAR_INFO buffer[240];
	int i;
	for (i = 0;i < 166; i++){
		buffer[i].Char.UnicodeChar = temp[i];
		buffer[i].Attributes = 7;
	}

	COORD bufferOrigin = {0,0}, bufferSize = {80, 3};
	SMALL_RECT rectToWrite = {0, 0, 80, 2};
	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),buffer, bufferSize, bufferOrigin, &rectToWrite);

}
void CConsoleUI::DrawdBInfo(){
	wchar_t buf[1200];
	wcscpy(buf,L"©°©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤Spectrum©¤©´");
	wcscat(buf,L"©¦Ch1 [                                                                      ]©¦");
	wcscat(buf,L"©¦Ch2 [                                                                      ]©¦");
	wcscat(buf,L"©¦                                                                            ©¦");
	wcscat(buf,L"©¦                                                                            ©¦");
	wcscat(buf,L"©¦                                                                            ©¦");
	wcscat(buf,L"©¦                                                                            ©¦");
	wcscat(buf,L"©¦                                                                            ©¦");
	wcscat(buf,L"©¦                                                                            ©¦");
	wcscat(buf,L"©¦                                                                            ©¦");
	wcscat(buf,L"©¦                                                                            ©¦");
	wcscat(buf,L"©¦                                                                            ©¦");
	wcscat(buf,L"©¦                                                                            ©¦");
	wcscat(buf,L"©¸©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¼");

	CHAR_INFO buffer[1200];
	int i;
	for (i = 0;i < 1200; i++){
		buffer[i].Char.UnicodeChar = buf[i];
		buffer[i].Attributes = 7;
	}
	COORD bufferOrigin = {0,0}, bufferSize = {80, 14};
	SMALL_RECT rectToWrite = {0, 3, 80, 17};
	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),buffer, bufferSize, bufferOrigin, &rectToWrite);

}
void CConsoleUI::DrawAll(){
	DrawTitle();
	DrawdBInfo();
	DrawButton();
}

int CConsoleUI::OpenFileProc(int,int,int,void*userData){
	CConsoleUI* pMe = (CConsoleUI*)userData;

	wchar_t buf[140] = L"¨XOpen File ¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨[\
¨U                                                            ¨U\
¨^¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨a";

	CHAR_INFO buffer[140];
	int i;
	for (i = 0;i < 140; i++){
		buffer[i].Char.UnicodeChar = buf[i];
		buffer[i].Attributes = 159;
	}
	COORD bufferOrigin = {0,0}, bufferSize = {64, 3};
	SMALL_RECT rectToWrite = {6, 10, 80, 12};
	WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),buffer, bufferSize, bufferOrigin, &rectToWrite);


	COORD   setps;
	setps.X = 8;
	setps.Y = 11;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),setps);
	string str;

	getline(cin, str);
	if (!str.empty())
		pMe->PlayIt(str.c_str());
	pMe->pWndManager->CloseConsoleTopWindow();
	return 0;
}
void CConsoleUI::PlayIt(const char* filename){
	switch (pMusicManager->Play(filename)){
	case ERR_OPEN_FAILED:
		pWndManager->ShowMessage(L" Open file error. Please check  your file name.");
		break;
	case ERR_STREAM_NOT_FOUND:
		pWndManager->ShowMessage(L"Audio stream doesn't Found.    Please make sure the file is anaudio/video file.");
		break;
	case ERR_UNSUPPORT_CODEC:
		pWndManager->ShowMessage(L"Codec is unsupported.");
		break;
	case ERR_INVALID_RATE_CH:
		pWndManager->ShowMessage(L"Rate or Channel counts is invalid.");
		break;
	case ERR_OPEN_CODEC:
		pWndManager->ShowMessage(L"Cannot open decoder.");
		break;
	case ERR_SDL:
		pWndManager->ShowMessage(L"Some errors with SDL occured.");
		break;
	}
	DrawButton();
}
