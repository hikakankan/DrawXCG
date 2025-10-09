#pragma once
#include <string>
#include "wintox.h"

class Color {
private:
	COLORREF color;
public:
	Color(COLORREF color) {
		this->color = color;
	}
	Color(int r, int g, int b) {
		color = RGB(r, g, b);
	}
	Color(int a, int r, int g, int b) {
		color = RGB(r, g, b);
	}
	operator COLORREF() const {
		return color;
	}
	static Color FromArgb(int alpha, int red, int green, int blue){
		return Color(red, green, blue);
	}
	static Color Black;
	static Color White;
};

class Bitmap {
public:
	Bitmap() {}
	Bitmap(int width, int height) {}
	Color GetPixel(int x, int y) {
		return ::GetPixel(GetDC(NULL), x, y);
	}
	void SetPixel(int x, int y, Color color){
		::SetPixel(GetDC(NULL), x, y, color);
	}
};
class Image {};

class Pen {
private:
	HPEN pen;
public:
	Pen(Color color) {
		pen = CreatePen(PS_SOLID, 1, color);
	}
	~Pen() {
		DeleteObject(pen);
	}
	operator HPEN() const {
		return pen;
	}
};

class Pens {
public:
	static Pen Black;
};

class Brush {
private:
	HBRUSH brush;
public:
	Brush(COLORREF color) {
		brush = CreateSolidBrush(color);
	}
	~Brush() {
		DeleteObject(brush);
	}
	operator HBRUSH() const {
		return brush;
	}
};

class SolidBrush : public Brush {
public:
	SolidBrush(COLORREF color) : Brush(color) {}
};

class Brushes {
public:
	static Brush Black;
	static Brush White;
};

//#include <gdiplusenums.h>

class Font {
private:
	HFONT hfont;
public:
	Font(const wchar_t* faceName, int pointSize) {
		hfont = CreateFont(
			pointSize, 0, 0, 0,
			0, FALSE,
			FALSE, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			faceName
		);
	}
	int FontStyleItalic = 2;
	Font(int pointSize, int family, int style, int weight) {
		hfont = CreateFont(
			pointSize, 0, 0, 0,
			weight, style & FontStyleItalic ? TRUE : FALSE,
			FALSE, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			L"Times New Roman"
		);
	}
	~Font() {
		DeleteObject(hfont);
	}
	operator HFONT() const {
		return hfont;
	}
};

class Size {
private:
	int width;
	int height;
public:
	Size(int w, int h) : width(w), height(h) {}
	int Width() const { return width; }
	int Height() const { return height; }
};

class Graphics {
private:
	HDC hdc;
public:
	Graphics(HDC hdc) : hdc(hdc) {}
	void SetPen(const Pen& pen) {
		SelectObject(hdc, pen);
	}
	void SetBrush(const Brush& brush) {
		SelectObject(hdc, brush);
	}
	void SetFont(const Font& font) {
		SelectObject(hdc, HFONT(font));
	}
	void DrawLine(float x1, float y1, float x2, float y2) {
		MoveToEx(hdc, (int)x1, (int)y1, NULL);
		LineTo(hdc, (int)x2, (int)y2);
	}
	void DrawRectangle(int x, int y, int width, int height) {
		Rectangle(hdc, x, y, x + width, y + height);
	}
	void DrawText(const char* text, float x, float y) {
		TextOutA(hdc, (int)x, (int)y, text, (int)strlen(text));
	}
	void DrawText(const wchar_t* text, float x, float y) {
		TextOutW(hdc, (int)x, (int)y, text, (int)wcslen(text));
	}
	void GetTextExtent(const char* text, int* width, int* height) {
		SIZE size;
		GetTextExtentPoint32A(hdc, text, (int)strlen(text), &size);
		if (width) *width = size.cx;
		if (height) *height = size.cy;
	}
	void GetTextExtent(const wchar_t* text, int* width, int* height) {
		SIZE size;
		GetTextExtentPoint32W(hdc, text, (int)wcslen(text), &size);
		if (width) *width = size.cx;
		if (height) *height = size.cy;
	}
	Size MeasureString(std::string str, Font font) {
		int width, height;
		GetTextExtent(str.c_str(), &width, &height);
		return Size(width, height);
	}
	void SetTextForeground(const Color& color) {
		SetTextColor(hdc, color);
	}
	void SetTextBackground(const Color& color) {
		SetBkColor(hdc, color);
	}
	void SetTextBackgroundMode(int mode) {
		SetBkMode(hdc, mode);
	}
	HDC GetHDC() const {
		return hdc;
	}
	void Clear(Color color){
		HBRUSH brush = CreateSolidBrush(color);
		RECT rect;
		GetClientRect(WindowFromDC(hdc), &rect);
		FillRect(hdc, &rect, brush);
		DeleteObject(brush);
	}
	void DrawLine(Pen pen, int x1, int y1, int x2, int y2) {
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);
		MoveToEx(hdc, x1, y1, NULL);
		LineTo(hdc, x2, y2);
		SelectObject(hdc, oldPen);
	}
	void DrawEllipse(Pen pen, int x, int y, int width, int height){
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);
		Ellipse(hdc, x, y, x + width, y + height);
		SelectObject(hdc, oldPen);
	}
	void FillRectangle(Brush brush, int x, int y, int width, int height){
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
		Rectangle(hdc, x, y, x + width, y + height);
		SelectObject(hdc, oldBrush);
	}
	void DrawImage(Bitmap image, int x, int y){
		//HDC hdcImage = image.GetHDC();
		//BitBlt(hdc, x, y, image.Width(), image.Height(), hdcImage, 0, 0, SRCCOPY);
		//ReleaseDC(NULL, hdcImage);
	}
	void DrawString(std::string str, Font font, Brush brush, int x, int y){
		HFONT oldFont = (HFONT)SelectObject(hdc, font);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
		TextOutA(hdc, (int)x, (int)y, str.c_str(), (int)str.length());
		SelectObject(hdc, oldFont);
		SelectObject(hdc, oldBrush);
	}
	static Graphics FromImage(Bitmap image){
		//HDC hdc = image.GetHDC();
		HDC hdc = nullptr;
		return Graphics(hdc);
	}
};

class DotNetControl
{
private:
	HWND hwnd;
public:
	DotNetControl(HWND hwnd) : hwnd(hwnd) {}
	Size GetClientSize() {
		RECT rect;
		GetClientRect(hwnd, &rect);
		return Size(rect.right - rect.left, rect.bottom - rect.top);
	}
	void Refresh() {
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
	}
	void SetBackgroundColour(const Color& color) {
		SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(color));
	}
	Graphics CreateGraphics() {
		HDC hdc = GetDC(hwnd);
		RECT rect;
		GetClientRect(hwnd, &rect);
		HRGN hrgn = CreateRectRgnIndirect(&rect);
		SelectClipRgn(hdc, hrgn);
		return Graphics(hdc);
	}
	void ReleaseGraphics(Graphics g) {
		// ReleaseDC 関数:
		// アプリケーションで ReleaseDC 関数を使用して、 
		// CreateDC 関数を呼び出して作成された DC を解放することはできません。
		// 代わりに、 DeleteDC 関数を使用する必要があります。
		ReleaseDC(hwnd, g.GetHDC());
	}
	PAINTSTRUCT ps = {};
	Graphics BeginPaintGraphics() {
		HDC hdc = BeginPaint(hwnd, &ps);
		return Graphics(hdc);
	}
	void EndPaintGraphics() {
		EndPaint(hwnd, &ps);
	}
	int Width() const {
		RECT rect;
		GetClientRect(hwnd, &rect);
		return rect.right - rect.left;
	}
	int Height() const {
		RECT rect;
		GetClientRect(hwnd, &rect);
		return rect.bottom - rect.top;
	}
};
