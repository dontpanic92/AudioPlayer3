#pragma once

#include "../stdafx.h"
#include "../resource.h"
// CMainForm �Ի���

class CMainForm : public CDialogEx
{
	DECLARE_DYNAMIC(CMainForm)

public:
	CMainForm(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CMainForm();

// �Ի�������
	enum { IDD = IDD_FORMVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
