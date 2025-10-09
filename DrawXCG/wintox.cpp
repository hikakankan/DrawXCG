#include "wintox.h"

#ifdef USE_X68000
bool DeleteObject(int obj) {
	return true;
}
COLORREF GetPixel(
	HDC hdc,
	int x,
	int y
) {
	return 0;
}
//COLORREF SetPixel(
//	HDC      hdc,
//	int      x,
//	int      y,
//	COLORREF color
//) {
//	return 0;
//}
HDC GetDC(
	HWND hWnd
) {
	return 0;
}
HPEN CreatePen(
	int      iStyle,
	int      cWidth,
	COLORREF color
) {
	return 0;
}
HBRUSH CreateSolidBrush(
	COLORREF color
) {
	return 0;
}
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
) {
	return 0;
}
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
) {
	return 0;
}
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
) {
	return 0;
}
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
) {
	return 0;
}
HGDIOBJ SelectObject(
	HDC     hdc,
	HGDIOBJ h
) {
	return 0;
}
BOOL MoveToEx(
	HDC     hdc,
	int     x,
	int     y,
	LPPOINT lppt
) {
	return true;
}
BOOL LineTo(
	HDC hdc,
	int x,
	int y
) {
	return true;
}
BOOL Rectangle(
	HDC hdc,
	int left,
	int top,
	int right,
	int bottom
) {
	return true;
}
BOOL TextOutA(
	HDC    hdc,
	int    x,
	int    y,
	LPCSTR lpString,
	int    c
) {
	return true;
}
BOOL TextOutW(
	HDC     hdc,
	int     x,
	int     y,
	LPCWSTR lpString,
	int     c
) {
	return true;
}
BOOL GetTextExtentPoint32A(
	HDC    hdc,
	LPCSTR lpString,
	int    c,
	LPSIZE psizl
) {
	return true;
}
BOOL GetTextExtentPoint32W(
	HDC     hdc,
	LPCWSTR lpString,
	int     c,
	LPSIZE  psizl
) {
	return true;
}
COLORREF SetTextColor(
	HDC      hdc,
	COLORREF color
) {
	return 0;
}
COLORREF SetBkColor(
	HDC      hdc,
	COLORREF color
) {
	return 0;
}
int SetBkMode(
	HDC hdc,
	int mode
) {
	return 0;
}
BOOL GetClientRect(
	HWND   hWnd,
	LPRECT lpRect
) {
	return true;
}
int FillRect(
	HDC        hDC,
	const RECT* lprc,
	HBRUSH     hbr
) {
	return 0;
}
BOOL Ellipse(
	HDC hdc,
	int left,
	int top,
	int right,
	int bottom
) {
	return true;
}
BOOL InvalidateRect(
	HWND       hWnd,
	const RECT* lpRect,
	BOOL       bErase
) {
	return true;
}
BOOL UpdateWindow(
	HWND hWnd
) {
	return true;
}
ULONG_PTR SetClassLongPtr(
	HWND     hWnd,
	int      nIndex,
	LONG_PTR dwNewLong
) {
	return 0;
}
ULONG_PTR SetClassLongPtrW(
	HWND     hWnd,
	int      nIndex,
	LONG_PTR dwNewLong
) {
	return 0;
}
HRGN CreateRectRgnIndirect(
	const RECT* lprect
) {
	return 0;
}
int SelectClipRgn(
	HDC  hdc,
	HRGN hrgn
) {
	return 0;
}
int ReleaseDC(
	HWND hWnd,
	HDC  hDC
) {
	return 0;
}
HDC BeginPaint(
	HWND          hWnd,
	LPPAINTSTRUCT lpPaint
) {
	return 0;
}
BOOL EndPaint(
	HWND              hWnd,
	const PAINTSTRUCT* lpPaint
) {
	return true;
}
HWND WindowFromDC(
	HDC hDC
) {
	return 0;
}
#endif
