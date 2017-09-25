#pragma once

#include "../stdafx.h"
#include "../resource.h"
// CMainForm 对话框

class CMainForm : public CDialogEx
{
	DECLARE_DYNAMIC(CMainForm)

public:
	CMainForm(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMainForm();

// 对话框数据
	enum { IDD = IDD_FORMVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
