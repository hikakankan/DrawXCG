#pragma once
#include "wintox.h"

void paint_cmd(HWND hwnd);
bool set_draw_cmd(int token_count, char** tokens, ConOutput& co_cout, ConOutput& co_cerr);
int16_t SetPixel_cmd(int16_t hdc, int16_t x, int16_t y, int16_t color);
int16_t SetPixelLine_cmd(int16_t hdc, int16_t x, int16_t y, int16_t color);
int16_t HLine_cmd(int hdc, int x, int y, int w, int color);
