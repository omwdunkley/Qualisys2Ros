#ifndef QUALISYS_NBC_NBC_MARKUP_H_INCLUDED
#define QUALISYS_NBC_NBC_MARKUP_H_INCLUDED
/***************************************************************************** 
 *         Copyright (c) Qualisys AB 2006. All rights reserved. 
 ***************************************************************************** 
 * 
 * The copyright to the computer program(s) herein is the property of  
 * Qualisys AB, Sweden. The program(s) may be used and/or copied  
 * only with the written permission from Qualisys AB or in  
 * accordance with the terms and conditions stipulated in the  
 * agreement/contract under which the program(s) have been supplied. 
 * 
 ********************************* HEADER FILE *******************************
 * 
 *  $Project: QTM$
 * 
 *   $Folder: RT Client 2$
 * 
 * $Workfile: NBC_Markup.h$
 * 
 * $Revision: 1$
 * 
 *     $Date: 2006-03-14 16:17:14$
 * 
 *****************************************************************************/ 


// Markup.h: interface for the NBC_CMarkup class.
//
// NBC_CMarkup Release 6.5 Lite
// Copyright (C) 1999-2003 First Objective Software, Inc. All rights reserved
// This entire notice must be retained in this source code
// Redistributing this source code requires written permission
// This software is provided "as is", with no warranty.
// Latest fixes enhancements and documentation at www.firstobject.com

// #include <afxtempl.h>
#pragma warning( push )
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include "StdString.h"
#pragma warning( pop )

#include <vector>
#include <string.h>
#include <float.h>
#include <arpa/inet.h>


#ifdef _DEBUG
#define _DS(i) (i?&((LPCTSTR)m_csDoc)[m_aPos[i].nStartL]:0)
#define MARKUP_SETDEBUGSTATE m_pMainDS=_DS(m_iPos); m_pChildDS=_DS(m_iPosChild)
#else
#define MARKUP_SETDEBUGSTATE
#endif
//define string compare etc for unicode this is a possible error source for the Markup

#ifdef _UNICODE 
#define _tccpy		wcscpy
#define _tclen 		wcslen
#define _tcslen     wcslen
#define _tcsnccmp	wcsncmp
#define _tcsncmp	wcsncmp
#define _tcslen     wcslen
#define _tcscpy     wcscpy
#define _tcscpy_s   wcscpy_s
#define _tcsncpy    wcsncpy
#define _tcsncpy_s  wcsncpy_s
#define _tcscat     wcscat
#define _tcscat_s   wcscat_s
#define _tcsupr     wcsupr
#define _tcsupr_s   wcsupr_s
#define _tcslwr     wcslwr
#define _tcslwr_s   wcslwr_s

#define _stprintf_s swprintf_s
#define _stprintf   swprintf
#define _tprintf    wprintf

#define _vstprintf_s    vswprintf_s
#define _vstprintf      vswprintf

#define _tscanf     wscanf


#define _TCHAR wchar_t

#else
#define _tccpy		strcpy
#define _tclen 		strlen
#define _tcslen     strlen
#define _tcsnccmp	strncmp
#define _tcsncmp	strncmp
#define _tcscpy     strcpy
#define _tcscpy_s   strcpy_s
#define _tcsncpy    strncpy
#define _tcsncpy_s  strncpy_s
#define _tcscat     strcat
#define _tcscat_s   strcat_s
#define _tcsupr     strupr
#define _tcsupr_s   strupr_s
#define _tcslwr     strlwr
#define _tcslwr_s   strlwr_s

#define _stprintf_s sprintf_s
#define _stprintf   sprintf
#define _tprintf    printf

#define _vstprintf_s    vsprintf_s
#define _vstprintf      vsprintf

#define _tscanf     scanf

#define _TCHAR char
#endif

#define stricmp  strcasecmp
  #define _stricmp strcasecmp
  #define strnicmp strncasecmp
  #define _tcsstr strstr
  #define _tcsnicmp strncasecmp
  #define _tcsncpy strncpy
  #define _tcschr strchr

// Simplified definition of LPCTSTR and _T this could be the error source
#ifdef _UNICODE
typedef const WCHAR* LPCTSTR; 
#else
typedef const char* LPCTSTR; 
#endif
// Sebastian tauscher
#if defined(_UNICODE)
#define _T(x) L ##x
#else
#define _T(x) x
#endif

class NBC_CMarkup  
{
public:
	NBC_CMarkup() { SetDoc( NULL ); mnIndent = 4; };
	NBC_CMarkup( LPCTSTR szDoc ) { SetDoc( szDoc ); };
	NBC_CMarkup( const NBC_CMarkup& markup ) { *this = markup; };
	void operator=( const NBC_CMarkup& markup );
	virtual ~NBC_CMarkup() {};

    // Settings
    void SetIndent( int nIndent = 4 );

	// Create
	CStdStringA GetDoc() const { return m_csDoc; };
	bool AddElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,false,false); };
	bool AddChildElem( LPCTSTR szName, LPCTSTR szData=NULL ) { return x_AddElem(szName,szData,false,true); };
	bool AddAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_iPos,szAttrib,szValue); };
	bool AddChildAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_iPosChild,szAttrib,szValue); };
	bool SetAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_iPos,szAttrib,szValue); };
	bool SetChildAttrib( LPCTSTR szAttrib, LPCTSTR szValue ) { return x_SetAttrib(m_iPosChild,szAttrib,szValue); };

	// Navigate
	bool SetDoc( LPCTSTR szDoc );
	bool IsWellFormed();
	bool FindElem( LPCTSTR szName=NULL );
	bool FindChildElem( LPCTSTR szName=NULL );
	bool IntoElem();
	bool OutOfElem();
	void ResetChildPos() { x_SetPos(m_iPosParent,m_iPos,0); };
	void ResetMainPos() { x_SetPos(m_iPosParent,0,0); };
	void ResetPos() { x_SetPos(0,0,0); };
	CStdStringA GetTagName() const;
	CStdStringA GetChildTagName() const { return x_GetTagName(m_iPosChild); };
	CStdStringA GetData() const { return x_GetData(m_iPos); };
	CStdStringA GetChildData() const { return x_GetData(m_iPosChild); };
	CStdStringA GetAttrib( LPCTSTR szAttrib ) const { return x_GetAttrib(m_iPos,szAttrib); };
	CStdStringA GetChildAttrib( LPCTSTR szAttrib ) const { return x_GetAttrib(m_iPosChild,szAttrib); };
	CStdStringA GetError() const { return m_csError; };

	enum MarkupNodeType
	{
		MNT_ELEMENT					= 1,  // 0x01
		MNT_TEXT					= 2,  // 0x02
		MNT_WHITESPACE				= 4,  // 0x04
		MNT_CDATA_SECTION			= 8,  // 0x08
		MNT_PROCESSING_INSTRUCTION	= 16, // 0x10
		MNT_COMMENT					= 32, // 0x20
		MNT_DOCUMENT_TYPE			= 64, // 0x40
		MNT_EXCLUDE_WHITESPACE		= 123,// 0x7b
	};

protected:

#ifdef _DEBUG
	LPCTSTR m_pMainDS;
	LPCTSTR m_pChildDS;
#endif

	CStdStringA m_csDoc;
	CStdStringA m_csError;

	struct ElemPos
	{
		ElemPos() { Clear(); };
		ElemPos( const ElemPos& pos ) { *this = pos; };
		bool IsEmptyElement() const { return (nStartR == nEndL + 1); };
		void Clear()
		{
			nStartL=0; nStartR=0; nEndL=0; nEndR=0; nReserved=0;
			iElemParent=0; iElemChild=0; iElemNext=0;
		};
		void AdjustStart( int n ) { nStartL+=n; nStartR+=n; };
		void AdjustEnd( int n ) { nEndL+=n; nEndR+=n; };
		int nStartL;
		int nStartR;
		int nEndL;
		int nEndR;
		int nReserved;
		int iElemParent;
		int iElemChild;
		int iElemNext;
	};

    std::vector<ElemPos> m_aPos;
	int m_iPosParent;
	int m_iPos;
	int m_iPosChild;
	int m_iPosFree;
	int m_nNodeType;

	struct TokenPos
	{
		TokenPos( LPCTSTR sz ) { Clear(); szDoc = sz; };
		bool IsValid() const { return (nL <= nR); };
		void Clear() { nL=0; nR=-1; nNext=0; bIsString=false; };
		bool Match( LPCTSTR szName )
		{
			int nLen = nR - nL + 1;
			return ( (_tcsnccmp( &szDoc[nL], szName, nLen ) == 0)
				&& ( szName[nLen] == _T('\0') || _tcschr(_T(" =/["),szName[nLen]) ) );
		};
		int nL;
		int nR;
		int nNext;
		LPCTSTR szDoc;
		bool bIsString;
	};

	void x_SetPos( int iPosParent, int iPos, int iPosChild )
	{
		m_iPosParent = iPosParent;
		m_iPos = iPos;
		m_iPosChild = iPosChild;
		m_nNodeType = iPos?MNT_ELEMENT:0;
		MARKUP_SETDEBUGSTATE;
	};

	int x_GetFreePos();
	int x_ReleasePos();
	int x_ParseElem( int iPos );
	int x_ParseError( LPCTSTR szError, LPCTSTR szName = NULL );
	static bool x_FindChar( LPCTSTR szDoc, int& nChar, _TCHAR c );
	static bool x_FindAny( LPCTSTR szDoc, int& nChar );
	static bool x_FindToken( TokenPos& token );
	CStdStringA x_GetToken( const TokenPos& token ) const;
	int x_FindElem( int iPosParent, int iPos, LPCTSTR szPath );
	CStdStringA x_GetTagName( int iPos ) const;
	CStdStringA x_GetData( int iPos ) const;
	CStdStringA x_GetAttrib( int iPos, LPCTSTR szAttrib ) const;
	bool x_AddElem( LPCTSTR szName, LPCTSTR szValue, bool bInsert, bool bAddChild );
	bool x_FindAttrib( TokenPos& token, LPCTSTR szAttrib=NULL ) const;
	bool x_SetAttrib( int iPos, LPCTSTR szAttrib, LPCTSTR szValue );
	void x_LocateNew( int iPosParent, int& iPosRel, int& nOffset, int nLength, int nFlags );
	int x_ParseNode( TokenPos& token );
	void x_DocChange( int nLeft, int nReplace, const CStdStringA& csInsert );
	void x_Adjust( int iPos, int nShift, bool bAfterPos = false );
	CStdStringA x_TextToDoc( LPCTSTR szText, bool bAttrib = false ) const;
	CStdStringA x_TextFromDoc( int nLeft, int nRight ) const;

protected:
    _TCHAR mtIndent[ 1000 ];
    int    mnIndent;

};

#endif /* QUALISYS_NBC_NBC_MARKUP_H_INCLUDED */
