// gui/MainForm.cpp : ʵ���ļ�
//

#include "MainForm.h"
#include "afxdialogex.h"


// CMainForm �Ի���

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


// CMainForm ��Ϣ�������
