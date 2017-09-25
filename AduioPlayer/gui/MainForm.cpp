// gui/MainForm.cpp : 实现文件
//

#include "MainForm.h"
#include "afxdialogex.h"


// CMainForm 对话框

IMPLEMENT_DYNAMIC(CMainForm, CDialogEx)

CMainForm::CMainForm(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMainForm::IDD, pParent)
{

}

CMainForm::~CMainForm()
{
}

void CMainForm::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMainForm, CDialogEx)
END_MESSAGE_MAP()


// CMainForm 消息处理程序
