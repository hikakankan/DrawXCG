#pragma once

#ifdef USE_X68000
#include <cstdint>
#include <cstring>
extern bool TRUE;
extern bool FALSE;
extern int PS_SOLID;
extern int DEFAULT_CHARSET;
extern int OUT_DEFAULT_PRECIS;
extern int CLIP_DEFAULT_PRECIS;
extern int DEFAULT_QUALITY;
extern int DEFAULT_PITCH;
extern int FF_DONTCARE;
extern int GCLP_HBRBACKGROUND;
typedef int HWND, *HDC, HRGN, HPEN, HBRUSH, HFONT, HGDIOBJ;
typedef uint32_t COLORREF;
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef bool BOOL;
typedef long LONG_PTR;
typedef long ULONG_PTR;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef const CHAR* LPCSTR;
typedef const WCHAR* LPCWSTR;
#define RGB(r, g, b) ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
typedef long LONG;
typedef struct tagPOINT {
	LONG x;
	LONG y;
} POINT, * PPOINT, * NPPOINT, * LPPOINT;
typedef struct tagSIZE {
	LONG cx;
	LONG cy;
} SIZE, * PSIZE, * LPSIZE;
typedef struct tagRECT {
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT, * PRECT, * NPRECT, * LPRECT;
typedef struct tagPAINTSTRUCT {
	HDC  hdc;
	BOOL fErase;
	RECT rcPaint;
	BOOL fRestore;
	BOOL fIncUpdate;
	BYTE rgbReserved[32];
} PAINTSTRUCT, * PPAINTSTRUCT, * NPPAINTSTRUCT, * LPPAINTSTRUCT;
bool DeleteObject(int obj);
COLORREF GetPixel(
	HDC hdc,
	int x,
	int y
);
COLORREF SetPixel(
	HDC      hdc,
	int      x,
	int      y,
	COLORREF color
);
HDC GetDC(
	HWND hWnd
);
HPEN CreatePen(
	int      iStyle,
	int      cWidth,
	COLORREF color
);
HBRUSH CreateSolidBrush(
	COLORREF color
);
HFONT CreateFont(
	int    cHeight,
	int    cWidth,
	int    cEscapement,
	int    cOrientation,
	int    cWeight,
	DWORD  bItalic,
	DWORD  bUnderline,
	DWORD  bStrikeOut,
	DWORD  iCharSet,
	DWORD  iOutPrecision,
	DWORD  iClipPrecision,
	DWORD  iQuality,
	DWORD  iPitchAndFamily,
	LPCSTR pszFaceName
);
HFONT CreateFontA(
	int    cHeight,
	int    cWidth,
	int    cEscapement,
	int    cOrientation,
	int    cWeight,
	DWORD  bItalic,
	DWORD  bUnderline,
	DWORD  bStrikeOut,
	DWORD  iCharSet,
	DWORD  iOutPrecision,
	DWORD  iClipPrecision,
	DWORD  iQuality,
	DWORD  iPitchAndFamily,
	LPCSTR pszFaceName
);
HFONT CreateFont(
	int     cHeight,
	int     cWidth,
	int     cEscapement,
	int     cOrientation,
	int     cWeight,
	DWORD   bItalic,
	DWORD   bUnderline,
	DWORD   bStrikeOut,
	DWORD   iCharSet,
	DWORD   iOutPrecision,
	DWORD   iClipPrecision,
	DWORD   iQuality,
	DWORD   iPitchAndFamily,
	LPCWSTR pszFaceName
);
HFONT CreateFontW(
	int     cHeight,
	int     cWidth,
	int     cEscapement,
	int     cOrientation,
	int     cWeight,
	DWORD   bItalic,
	DWORD   bUnderline,
	DWORD   bStrikeOut,
	DWORD   iCharSet,
	DWORD   iOutPrecision,
	DWORD   iClipPrecision,
	DWORD   iQuality,
	DWORD   iPitchAndFamily,
	LPCWSTR pszFaceName
);
HGDIOBJ SelectObject(
	HDC     hdc,
	HGDIOBJ h
);
BOOL MoveToEx(
	HDC     hdc,
	int     x,
	int     y,
	LPPOINT lppt
);
BOOL LineTo(
	HDC hdc,
	int x,
	int y
);
BOOL Rectangle(
	HDC hdc,
	int left,
	int top,
	int right,
	int bottom
);
BOOL TextOutA(
	HDC    hdc,
	int    x,
	int    y,
	LPCSTR lpString,
	int    c
);
BOOL TextOutW(
	HDC     hdc,
	int     x,
	int     y,
	LPCWSTR lpString,
	int     c
);
BOOL GetTextExtentPoint32A(
	HDC    hdc,
	LPCSTR lpString,
	int    c,
	LPSIZE psizl
);
BOOL GetTextExtentPoint32W(
	HDC     hdc,
	LPCWSTR lpString,
	int     c,
	LPSIZE  psizl
);
COLORREF SetTextColor(
	HDC      hdc,
	COLORREF color
);
COLORREF SetBkColor(
	HDC      hdc,
	COLORREF color
);
int SetBkMode(
	HDC hdc,
	int mode
);
BOOL GetClientRect(
	HWND   hWnd,
	LPRECT lpRect
);
int FillRect(
	HDC        hDC,
	const RECT* lprc,
	HBRUSH     hbr
);
BOOL Ellipse(
	HDC hdc,
	int left,
	int top,
	int right,
	int bottom
);
BOOL InvalidateRect(
	HWND       hWnd,
	const RECT* lpRect,
	BOOL       bErase
);
BOOL UpdateWindow(
	HWND hWnd
);
ULONG_PTR SetClassLongPtr(
	HWND     hWnd,
	int      nIndex,
	LONG_PTR dwNewLong
);
ULONG_PTR SetClassLongPtrW(
	HWND     hWnd,
	int      nIndex,
	LONG_PTR dwNewLong
);
HRGN CreateRectRgnIndirect(
	const RECT* lprect
);
int SelectClipRgn(
	HDC  hdc,
	HRGN hrgn
);
int ReleaseDC(
	HWND hWnd,
	HDC  hDC
);
HDC BeginPaint(
	HWND          hWnd,
	LPPAINTSTRUCT lpPaint
);
BOOL EndPaint(
	HWND              hWnd,
	const PAINTSTRUCT* lpPaint
);
HWND WindowFromDC(
	HDC hDC
);
#else
#include <windows.h>
#include <tchar.h>
#endif
