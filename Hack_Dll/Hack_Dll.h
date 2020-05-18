// Hack_Dll.h : main header file for the HACK_DLL DLL
//

#if !defined(AFX_HACK_DLL_H__583C7954_A0B9_4DD6_95EE_32F2DC6C4C6F__INCLUDED_)
#define AFX_HACK_DLL_H__583C7954_A0B9_4DD6_95EE_32F2DC6C4C6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CHack_DllApp
// See Hack_Dll.cpp for the implementation of this class
//

class CHack_DllApp : public CWinApp
{
public:
	CHack_DllApp();
	~CHack_DllApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHack_DllApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CHack_DllApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HACK_DLL_H__583C7954_A0B9_4DD6_95EE_32F2DC6C4C6F__INCLUDED_)
