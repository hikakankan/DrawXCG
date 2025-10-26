#pragma once

//#include "resource.h"

void DrawHLine(HDC hdc, int x1, int y1, int w, COLORREF color);

enum class DrawType {
    Line, Rect, Ellipse, Arc, Pie, RoundRect, FillRect, FillEllipse, FillPie, FillRoundRect,
	FillTriangle, FillTrapezoid, NumberString,
    Circle, Paint, Pset, Point, Symbol, Apage, Vpage, Wipe, Screen, Window,
    Test
};

#define MAX_DRAW_TEXT 1024 // 最大文字列長
#define MAX_XARGS 8 // 最大引数数

struct DrawCommand {
    DrawType type;
    COLORREF color;
    int x1, y1, x2, y2, w, h, rx, ry, x3, y3, w2;
    int start_angle, sweep_angle;
    int r, hbyw, end_angle, sc1, sc2, sc3, sc4, page;
    uint8_t mx, my, font_size, rotation;
	int number_string_type; // 数値文字列のタイプ
    char text[MAX_DRAW_TEXT];
};

void print_command(const DrawCommand& cmd);

void exec_command(DrawCommand& cmd);

#ifndef CONSOLE_IO
// 色を解釈（整数 or R,G,B）
COLORREF parseColor(char* s);
#endif

inline double deg_to_rad(int angle) {
    return angle * 3.14159265 / 180.0;
}
