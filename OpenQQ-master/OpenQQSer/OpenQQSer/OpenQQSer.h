
// OpenQQSer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// COpenQQSerApp:
// �йش����ʵ�֣������ OpenQQSer.cpp
//

class COpenQQSerApp : public CWinApp
{
public:
	COpenQQSerApp();
	~COpenQQSerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern COpenQQSerApp theApp;