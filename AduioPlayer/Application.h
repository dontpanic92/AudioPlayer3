#ifndef _APPLICATION_H
#define _APPLICATION_H
#include "CConsoleUI.h"
#include "CMusicManger.h"
#include "gui/MainForm.h"

class Application{
private:
	CConsoleUI* pConsoleUI;
	CMusicManager* pMusicManager;
public:
	Application() : pConsoleUI(NULL){
		pMusicManager = new CMusicManager;
	}
	void go(){
		pConsoleUI = new CConsoleUI(pMusicManager);
		pConsoleUI->WaitForReturn();
	}
	~Application(){
		if (pConsoleUI){
			delete pConsoleUI;
		}
		if (pMusicManager){
			delete pConsoleUI;
		}
	}
};
#endif