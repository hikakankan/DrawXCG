#pragma once

//#include "resource.h"

void DrawHLine(HDC hdc, int x1, int y1, int w, COLORREF color);

enum class DrawType {
    Clear, Line, String, Rect, Ellipse, Arc, Pie, RoundRect, FillRect, FillEllipse, FillPie, FillRoundRect,
    FillTriangle, FillWTriangle, FillTrapezoid,
    Circle, Paint, Pset, Point, Symbol, Apage, Vpage, Wipe, Screen, Window, LineTo, LineInit,
    XClear, XLine, XCircle, XPie, XBox, XFill, XPaint, XPset, XPoint, Test
};

#define MAX_DRAW_TEXT 1024 // Å‘å•¶š—ñ’·
#define MAX_XARGS 8 // Å‘åˆø””

struct DrawCommand {
    DrawType type;
    COLORREF color;
    int x1, y1, x2, y2, w, h, rx, ry, x3, y3, w2;
    int start_angle, sweep_angle;
    int r, hbyw, end_angle, sc1, sc2, sc3, sc4, page;
    uint8_t mx, my, font_size, rotation;
    char text[MAX_DRAW_TEXT];
};

void print_command(const DrawCommand& cmd);

void exec_command(DrawCommand& cmd);

#ifndef CONSOLE_IO
// F‚ğ‰ğßi®” or R,G,Bj
COLORREF parseColor(char* s);
#endif

inline double deg_to_rad(int angle) {
    return angle * 3.14159265 / 180.0;
}
