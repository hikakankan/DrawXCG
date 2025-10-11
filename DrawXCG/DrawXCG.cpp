//#define USE_X68000
#ifdef USE_X68000
//#define PACKED_STRUCT
#define CONSOLE_IO // コンソール入出力を使用
#endif
//#define CONSOLE_IO // コンソール入出力を使用

#define PIXEL_DRAW_MODE // OSの機能を使わない描画
#define USE_FUNC_TABLE // 関数テーブルを使用

#ifndef USE_X68000
#include <thread>
#include <queue>
#include <mutex>
#include <windows.h>
#define WM_USER_DRAW (WM_USER + 1)
#else
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cmath> // abs関数を使用するために必要
//typedef int COLORREF; // X68000版ではCOLORREFをintとして扱う
#endif

#include "console.h"
#ifndef CONSOLE_IO
#include "form.h"
#endif
#include "DrawXCG.h"
#include "CommandInterpreter.h"

void print_command(const DrawCommand& cmd) {
    printf("Command: type=%d, color=%d, x1=%d, y1=%d, x2=%d, y2=%d, x3=%d, y3=%d, w=%d, h=%d, w2=%d, rx=%d, ry=%d, "
        "start_angle=%d, sweep_angle=%d, r=%d, hbyw=%d, end_angle=%d, sc1=%d, sc2=%d, sc3=%d, sc4=%d, page=%d, "
        "mx=%u, my=%u, font_size=%u, rotation=%u, text='%s'\n",
        static_cast<int>(cmd.type), cmd.color,
        cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.x3, cmd.y3,
        cmd.w, cmd.h, cmd.w2, cmd.rx, cmd.ry,
        cmd.start_angle, cmd.sweep_angle,
        cmd.r, cmd.hbyw, cmd.end_angle,
        cmd.sc1, cmd.sc2, cmd.sc3, cmd.sc4,
        cmd.page,
        cmd.mx, cmd.my, cmd.font_size, cmd.rotation,
        cmd.text);
}

#ifndef USE_X68000
std::queue<DrawCommand> g_drawQueue;
std::mutex g_queueMutex;
HWND g_hWnd = nullptr;
#endif

#ifndef USE_X68000
void enqueue(const DrawCommand& cmd) {
    std::lock_guard<std::mutex> lock(g_queueMutex);
    g_drawQueue.push(cmd);
#ifdef CONSOLE_IO
    InvalidateRect(g_hWnd, nullptr, FALSE);
#else
    InvalidateRect(hDrawWindow, nullptr, false);
#endif
}
#endif

#ifdef USE_X68000
void exec_command_x(DrawCommand& cmd);
void exec_command(DrawCommand& cmd) {
    exec_command_x(cmd);
}
#else
void exec_command(DrawCommand& cmd) {
    enqueue(cmd);
}
#endif

int16_t SetPixel_cmd_(int16_t hdc, int16_t x, int16_t y, int16_t color) {
    DrawCommand cmd = {};
    cmd.type = DrawType::Pset;
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.color = color;
    //set_cmd(cmd);
    //enqueue(cmd);
    return 0;
}

int16_t SetPixel_cmd(int16_t hdc_, int16_t x, int16_t y, int16_t color) {
#ifndef USE_X68000
    HDC hdc = GetDC(hDrawWindow);
    COLORREF c = SetPixel(hdc, x, y, PALETTEINDEX(color));
    ReleaseDC(hDrawWindow, hdc);
    return c;
#else
    return 0;
#endif
}

#ifdef USE_X68000
//typedef int HDC; // HDCの定義（PC以外では使用しないが、互換性のために定義）
//struct POINT {
//    int x;
//    int y;
//};

// インラインアセンブラで使うためのグローバル変数の定義
int16_t g_color;
int32_t g_x1, g_y1, g_x2, g_y2, g_x3, g_y3, g_w, g_h, g_w2, g_rx, g_ry;
int32_t g_start_angle, g_sweep_angle;
int32_t g_r, g_hbyw, g_end_angle, g_sc1, g_sc2, g_sc3, g_sc4, g_page;
uint8_t g_mx, g_my, g_font_size, g_rotation;
char g_text[MAX_DRAW_TEXT];
int32_t g_gsize = 0;        // グラフィックス領域の幅 0: 512x512, 1: 1024x1024
int32_t g_gaddr = 0xc00000; // グラフィックス用メモリーの先頭のアドレス
uint16_t g_xargs[MAX_XARGS];
void global_copy_args(uint16_t* args, int argsize) {
    for (int i = 0; i < argsize; ++i) {
        g_xargs[i] = args[i];
    }
}
void global_copy_args(int16_t* args, int argsize) {
    for (int i = 0; i < argsize; ++i) {
        g_xargs[i] = args[i];
    }
}
uint16_t global_get_arg(int index) {
    if (index < 0 || index >= MAX_XARGS) {
        return 0; // 範囲外のインデックスは0を返す
    }
    return g_xargs[index];
}
void global_set_crtmode(int mode) {
    if (mode >= 4 && mode <= 15) {
        g_gsize = 0;
    }
    else {
        g_gsize = 1;
    }
}
void global_set_apage(int page) {
    g_gaddr = 0xc00000 + (page * 0x80000); // ページごとにアドレスを設定
}
#ifdef PACKED_STRUCT
struct __attribute__((packed)) PaintStruct {
#else
struct PaintStruct {
#endif
    uint16_t x;
    uint16_t y;
    uint16_t color;
    char* start_work;
    char* end_work;
};
PaintStruct g_paint = { 0, 0, 0, nullptr, nullptr };
PaintStruct* p_paint = &g_paint;
#ifdef PACKED_STRUCT
struct __attribute__((packed)) DrawStringStruct {
#else
struct DrawStringStruct {
#endif
    uint16_t x;
    uint16_t y;
    const char* str;
    uint8_t mx;
    uint8_t my;
    uint16_t color;
    uint8_t font_size;
    uint8_t rotation;
};
DrawStringStruct g_draw_string = { 0, 0, nullptr, 0, 0, 0, 0, 0 };
DrawStringStruct* p_draw_string = &g_draw_string;

//void point(uint16_t* data);
//void pset(uint16_t* data);
//COLORREF GetPixel(HDC hdc, int x, int y) {
//    uint16_t data[3] = { static_cast<uint16_t>(x), static_cast<uint16_t>(y), 0 };
//    point(data); // point関数を呼び出す
//    return static_cast<COLORREF>(data[2]); // 結果をCOLORREFとして返す
//}
//void SetPixel(HDC hdc, int x, int y, COLORREF color) {
//    uint16_t data[3] = { static_cast<uint16_t>(x), static_cast<uint16_t>(y), static_cast<uint16_t>(color) };
//    pset(data); // pset関数を呼び出す
//}
int pointx(int x1, int y1);
void psetx(int x1, int y1, int color);
//COLORREF GetPixel(HDC hdc, int x, int y) {
//    return static_cast<COLORREF>(pointx(x, y)); // 結果をCOLORREFとして返す
//}
//void SetPixel(HDC hdc, int x, int y, COLORREF color) {
//    psetx(x, y, color); // psetx関数を呼び出す
//}
int strcpy_s(char* dest, size_t size, const char* src) {
    // X68000版ではstrcpy_sを使用
    size_t i;
    for (i = 0; i < size - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0'; // null終端
    return 0; // 成功
}
#endif

// 色を解釈（整数 or R,G,B）
COLORREF parseColor(char* s) {
#ifndef USE_X68000
    if (strchr(s, ',') != nullptr) {
        int r, g, b;
        //sscanf_s(s.c_str(), "%d,%d,%d", &r, &g, &b);
        sscanf_s(s, "%d,%d,%d", &r, &g, &b);
        return RGB(r, g, b);
    }
    else {
        int c = atoi(s);
        return PALETTEINDEX(c);
    }
#else
    // X68000版では整数として扱う
    return atoi(s);
#endif
}

#ifdef USE_X68000
#include "drawasm.h"
#endif

#ifdef USE_X68000
// IOCSコール
// 画面モードを設定
void crtmod(int mode)
{
    global_set_crtmode(mode); // グローバル変数に画面モードを設定
    asm(
        "moveq  #0x10, %%d0\n\t" // _CRTMOD
        "move.w %0, %%d1\n\t"
        "trap   #15\n\t"
        :
    : "r"(mode)              // 入力
        : "d0", "d1"             // 使用するレジスタ
        );
}
int get_crtmod()
{
    int mode;
    asm(
        "moveq  #0x10, %%d0\n\t" // _CRTMOD
        "move.w #-1, %%d1\n\t"
        "trap   #15\n\t"
        "move.l %%d0, %0\n\t"
        : "=r"(mode)
        :                        // 入力
        : "d0", "d1"             // 使用するレジスタ
    );
    return mode;
}
// グラフィック画面のページをクリア
void gclear()
{
    asm(
        "move.l #0x90, %%d0\n\t" // _G_CLR_ON
        "trap   #15\n\t"
        :
    :                        // 入力なし
        : "d0"                   // 使用するレジスタ
        );
}
// グラフィック画面の書き込みページを設定
void apage(int page)
{
    global_set_apage(page); // グローバル変数にページを設定
    asm(
        "move.l #0xb1, %%d0\n\t" // _APAGE
        "move.l %0, %%d1\n\t"
        "trap   #15\n\t"
        :
    : "r"(page)              // 入力
        : "d0", "d1"             // 使用するレジスタ
        );
}
int get_apage()
{
    int page;
    asm(
        "move.l #0xb1, %%d0\n\t" // _APAGE
        "move.l #-1, %%d1\n\t"
        "trap   #15\n\t"
        "move.l %%d0, %0\n\t"
        : "=r"(page)
        :                        // 入力
        : "d0", "d1"             // 使用するレジスタ
    );
    return page;
}
// グラフィック画面の表示ページを設定
void vpage(int pages)
{
    asm(
        "move.l #0xb2, %%d0\n\t" // _VPAGE
        "move.l %0, %%d1\n\t"
        "trap   #15\n\t"
        :
    : "r"(pages)             // 入力
        : "d0", "d1"             // 使用するレジスタ
        );
}
// グラフィック画面のウィンドウを設定
void window(int x1, int y1, int x2, int y2)
{
    asm(
        "move.l #0xb4, %%d0\n\t" // _WINDOW
        "move.l %0, %%d1\n\t"
        "move.l %1, %%d2\n\t"
        "move.l %2, %%d3\n\t"
        "move.l %3, %%d4\n\t"
        "trap   #15\n\t"
        :
    : "r"(x1), "r"(y1), "r"(x2), "r"(y2) // 入力
        : "d0", "d1", "d2", "d3", "d4"       // 使用するレジスタ
        );
}
// グラフィック画面をクリア
void wipe()
{
    asm(
        "move.l #0xb5, %%d0\n\t" // _WIPE
        "trap   #15\n\t"
        :
    :                        // 入力
        : "d0"                   // 使用するレジスタ
        );
}
// グラフィック画面に点を描画
void pset(uint16_t * data)
{
    global_copy_args(data, 3); // 引数をグローバル変数にコピー
    asm(
        "move.l #0xb6, %%d0\n\t" // _PSET
        "lea    g_xargs, %%a1\n\t" // a1 = [x, y, color]
        "trap   #15\n\t"
        :
    :                        // 入力
        : "d0", "a1"             // 使用するレジスタ
        );
}
void psetx(int x1, int y1, int color)
{
    asm(
        "move.l   %0, %%d1\n\t"
        "move.l   %1, %%d2\n\t"
        "move.l   %2, %%d3\n\t"
        "clr.l	  -(%%sp)\n\t"
        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードへ)
        "move.l   %%d0, (%%sp)\n\t"
        "movea.l  %%d1, %%a0\n\t"
        "adda.l   %%d1, %%a0\n\t"  // a0 = x1 * 2
        "moveq.l  #10, %%d0\n\t"
        "add.l    g_gsize, %d0\n\t"
        "lsl.l    %%d0, %%d2\n\t"  // d2 = y1 * (1024 << g_gsize)
        "adda.l   %%d2, %%a0\n\t"  // a0 = x1 * 2 + y1 * (1024 << g_gsize)
        "adda.l   g_gaddr, %%a0\n\t" // a0 = g_gaddr + x1 * 2 + y1 * (1024 << g_gsize)
        "move.w   %%d3, (%%a0)\n\t"
        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードから戻る)
        "addq.l   #4,%%sp\n\t"
        :
    : "r"(x1), "r"(y1), "r"(color) // 入力
        : "d0", "d1", "d2", "d3", "a0" // 使用するレジスタ
        );
}
// グラフィック画面に点の色を返す
void point(uint16_t * data)
{
    global_copy_args(data, 2); // 引数をグローバル変数にコピー
    asm(
        "move.l #0xb7, %%d0\n\t" // _POINT
        "lea    g_xargs, %%a1\n\t" // a1 = [x, y, color]
        "trap   #15\n\t"
        :
    :                        // 入力
        : "d0", "a1"             // 使用するレジスタ
        );
    data[2] = global_get_arg(2); // 結果をdataに格納
}
int pointx(int x1, int y1)
{
    uint16_t color = 0;
    asm(
        "move.l   %1, %%d1\n\t"
        "move.l   %2, %%d2\n\t"
        "clr.l	  -(%%sp)\n\t"
        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードへ)
        "move.l   %%d0, (%%sp)\n\t"
        "movea.l  %%d1, %%a0\n\t"
        "adda.l   %%d1, %%a0\n\t"  // a0 = x1 * 2
        "moveq.l  #10, %%d0\n\t"
        "add.l    g_gsize, %d0\n\t"
        "lsl.l    %%d0, %%d2\n\t"  // d2 = y1 * (1024 << g_gsize)
        "adda.l   %%d2, %%a0\n\t"  // a0 = x1 * 2 + y1 * (1024 << g_gsize)
        "adda.l   g_gaddr, %%a0\n\t" // a0 = g_gaddr + x1 * 2 + y1 * (1024 << g_gsize)
        "move.w   (%%a0), %%d3\n\t"
        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードから戻る)
        "addq.l   #4,%%sp\n\t"
        "move.l   %%d3, %0\n\t"
        : "=r"(color)
        : "r"(x1), "r"(y1) // 入力
        : "d0", "d1", "d2", "d3", "a0" // 使用するレジスタ
    );
    return color; // 結果を返す
}
// グラフィック画面に線を描画
void drawline(uint16_t * data)
{
    global_copy_args(data, 6); // 引数をグローバル変数にコピー
    asm(
        "move.l #0xb8, %%d0\n\t"   // _LINE
        "lea    g_xargs, %%a1\n\t" // a1 = [x1, y1, x2, y2, color, style]
        "trap   #15\n\t"
        :
    :                          // 入力
        : "d0", "a1"               // 使用するレジスタ
        );
}
void drawlinex(int x1, int y1, int x2, int y2, COLORREF color)
{
    asm(
        "move.l   %0, g_x1\n\t"
        "move.l   %1, g_y1\n\t"
        "move.l   %2, g_x2\n\t"
        "move.l   %3, g_y2\n\t"
        "move.w   %4, g_color\n\t"
        "clr.l	  -(%%sp)\n\t"
        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードへ)
        "move.l   %%d0, (%%sp)\n\t"

        "move.w   g_color, -(%%sp)\n\t"
        "move.l   g_y2, -(%%sp)\n\t"
        "move.l   g_x2, -(%%sp)\n\t"
        "move.l   g_y1, -(%%sp)\n\t"
        "move.l   g_x1, -(%%sp)\n\t"
        "move.l   #0, -(%%sp)\n\t"
        "jsr     drawlinex_a\n\t"
        "add.l    #22,%%sp\n\t"

        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードから戻る)
        "addq.l   #4, %%sp\n\t"
        :
    : "r"(x1), "r"(y1), "r"(x2), "r"(y2), "r"(color) // 入力
        : "d0"                                           // 使用するレジスタ
        );
}
// グラフィック画面に長方形を描画
void drawbox(uint16_t * data)
{
    global_copy_args(data, 6); // 引数をグローバル変数にコピー
    asm(
        "move.l #0xb9, %%d0\n\t" // _BOX
        "lea    g_xargs, %%a1\n\t" // a1 = [x1, y1, x2, y2, color, style]
        "trap   #15\n\t"
        :
    :                        // 入力
        : "d0", "a1"             // 使用するレジスタ
        );
}
void drawboxx(int x1, int y1, int w, int h, COLORREF color)
{
    asm(
        "move.l   %0, %%d1\n\t"
        "move.l   %1, %%d2\n\t"
        "move.l   %2, %%d3\n\t"
        "move.l   %3, %%d4\n\t"
        "move.l   %4, %%d5\n\t"
        "clr.l	  -(%%sp)\n\t"
        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードへ)
        "move.l   %%d0, (%%sp)\n\t"

        "movea.l  %%d1, %%a0\n\t"
        "adda.l   %%d1, %%a0\n\t"  // a0 = x1 * 2
        "moveq.l  #10, %%d0\n\t"
        "add.l    g_gsize, %%d0\n\t"
        "lsl.l    %%d0, %%d2\n\t"  // d2 = y1 * (1024 << g_gsize)
        "adda.l   %%d2, %%a0\n\t"  // a0 = x1 * 2 + y1 * (1024 << g_gsize)
        "adda.l   g_gaddr, %%a0\n\t" // a0 = g_gaddr + x1 * 2 + y1 * (1024 << g_gsize)
        "move.l   %%a0, %%a1\n\t"

        "move.l   %%a0, -(%%sp)\n\t"

        "move.l   %%d4, %%d2\n\t"
        "lsl.l    %%d0, %%d2\n\t"  // d2 = h * (1024 << g_gsize)
        "adda.l   %%d2, %%a1\n\t"  // a1 = x1 * 2 + (y1 + h) * (1024 << g_gsize)

        "move.l   %%d3, %%d6\n\t"  // d6 = w (x方向のカウンター)
        "boxxxlp:\n\t"
        "move.w   %%d5, (%%a0)+\n\t"
        "move.w   %%d5, (%%a1)+\n\t"
        "dbra     %%d6, boxxxlp\n\t" // x方向のループ

        "move.l   (%%sp)+, %%a0\n\t" // a0 = g_gaddr + x1 * 2 + y1 * (1024 << g_gsize)
        "move.l   %%a0, %%a1\n\t"
        "adda.l   %%d3, %%a1\n\t"
        "adda.l   %%d3, %%a1\n\t"  // a1 = g_gaddr + (x1 + w) * 2 + y1 * (1024 << g_gsize)

        "moveq.l  #1, %%d2\n\t"
        "lsl.l    %%d0, %%d2\n\t"  // d2 = (1024 << g_gsize) (y方向の差分)

        "move.l   %%d4, %%d7\n\t"  // d7 = h (y方向のカウンター)
        "boxxylp:\n\t"
        "move.w   %%d5, (%%a0)\n\t"
        "adda.l   %%d2, %%a0\n\t"
        "move.w   %%d5, (%%a1)\n\t"
        "adda.l   %%d2, %%a1\n\t"
        "dbra     %%d7, boxxylp\n\t" // y方向のループ

        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードから戻る)
        "addq.l   #4,%%sp\n\t"
        :
    : "r"(x1), "r"(y1), "r"(w), "r"(h), "r"(color) // 入力
        : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "a0", "a1" // 使用するレジスタ
        );
}
// グラフィック画面の長方形を塗りつぶす
void fillbox(uint16_t * data)
{
    global_copy_args(data, 5); // 引数をグローバル変数にコピー
    asm(
        "move.l #0xba, %%d0\n\t" // _FILL
        "lea    g_xargs, %%a1\n\t" // a1 = [x1, y1, x2, y2, color]
        "trap   #15\n\t"
        :
    :                        // 入力
        : "d0", "a1"             // 使用するレジスタ
        );
}
void fillboxx(int x1, int y1, int w, int h, COLORREF color)
{
    asm(
        "move.l   %0, %%d1\n\t"
        "move.l   %1, %%d2\n\t"
        "move.l   %2, %%d3\n\t"
        "move.l   %3, %%d4\n\t"
        "move.l   %4, %%d5\n\t"

        "clr.l	  -(%%sp)\n\t"
        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードへ)
        "move.l   %%d0, (%%sp)\n\t"

        "movea.l  %%d1, %%a0\n\t"
        "adda.l   %%d1, %%a0\n\t"  // a0 = x1 * 2
        "moveq.l  #10, %%d0\n\t"
        "add.l    g_gsize, %%d0\n\t"
        "lsl.l    %%d0, %%d2\n\t"  // d2 = y1 * (1024 << g_gsize)
        "adda.l   %%d2, %%a0\n\t"  // a0 = x1 * 2 + y1 * (1024 << g_gsize)
        "adda.l   g_gaddr, %%a0\n\t" // a0 = g_gaddr + x1 * 2 + y1 * (1024 << g_gsize)
        "move.l   %%a0, %%a1\n\t"

        "moveq.l  #1, %%d2\n\t"
        "lsl.l    %%d0, %%d2\n\t"  // d2 = (1024 << g_gsize) (y方向の差分)

        "move.l   %%d4, %%d7\n\t"  // d7 = h (y方向のカウンター)
        "fillxylp:\n\t"
        "move.l   %%d3, %%d6\n\t"  // d6 = w (x方向のカウンター)
        "fillxxlp:\n\t"

        "move.w   %%d5, (%%a0)+\n\t"

        "dbra     %%d6, fillxxlp\n\t" // x方向のループ
        "adda.l   %%d2, %%a1\n\t"
        "move.l   %%a1, %%a0\n\t"
        "dbra     %%d7, fillxylp\n\t" // y方向のループ

        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードから戻る)
        "addq.l   #4,%%sp\n\t"
        :
    : "r"(x1), "r"(y1), "r"(w), "r"(h), "r"(color) // 入力
        : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "a0", "a1" // 使用するレジスタ
        );
}
// グラフィック画面に楕円を描画
void drawcircle(int16_t * data)
{
    global_copy_args(data, 7); // 引数をグローバル変数にコピー
    asm(
        "move.l #0xbb, %%d0\n\t" // _CIRCLE
        "lea    g_xargs, %%a1\n\t"    // a1 = [cx, cy, r, color, start_angle, end_angle, aspect_ratio]
        "trap   #15\n\t"
        :
    :                        // 入力
        : "d0", "a1"             // 使用するレジスタ
        );
}
// グラフィック画面の指定した場所を塗りつぶす
void paint(PaintStruct * data)
{
    g_paint.x = data->x;
    g_paint.y = data->y;
    g_paint.color = data->color;
    g_paint.start_work = data->start_work;
    g_paint.end_work = data->end_work;
    asm(
        "move.l #0xbc, %%d0\n\t" // _PAINT
        "lea    p_paint, %%a1\n\t"    // a1 = [x, y, color, start_work, end_work]
        "trap   #15\n\t"
        :
    :                        // 入力
        : "d0", "a1"             // 使用するレジスタ
        );
}
// グラフィック画面に文字列を表示する
void drawstring(DrawStringStruct * data)
{
    g_draw_string.x = data->x;
    g_draw_string.y = data->y;
    g_draw_string.str = data->str;
    g_draw_string.mx = data->mx;
    g_draw_string.my = data->my;
    g_draw_string.color = data->color;
    g_draw_string.font_size = data->font_size;
    g_draw_string.rotation = data->rotation;
    asm(
        "move.l #0xbd, %%d0\n\t" // _SYMBOL
        "lea    p_draw_string, %%a1\n\t"    // a1 = [x, y, str, mx, my, color, font_size, rotation]
        "trap   #15\n\t"
        :
    :                        // 入力
        : "d0", "a1"             // 使用するレジスタ
        );
}
// アセンブラのテスト
void test()
{
}

uint16_t default_line_style = 0xffff; // デフォルトの線のスタイル
void Clear(const DrawCommand & cmd) {
    wipe();
}
void XClear(const DrawCommand & cmd) {
    wipe();
}
int draw_x = -1;
int draw_y = -1;
void DrawLineInit() {
    draw_x = -1;
}
#ifdef PIXEL_DRAW_MODE
void drawarcx(int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color);
void drawpiex(int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color);
void drawroundedrectanglex(int x1, int y1, int w, int h, int rx, int ry, COLORREF color);
void fillellipsex(int x1, int y1, int w, int h, COLORREF color);
void fillroundedrectanglex(int x1, int y1, int w, int h, int rx, int ry, COLORREF color);
void filltrapezoidx(int x1, int y1, int x2, int y2, int w1, int w2, COLORREF color);
void filltrianglex(int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color);

void DirectDrawLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
void DirectDrawRectangle(HDC hdc, int x1, int y1, int w, int h, COLORREF color);
void DirectFillRectangle(HDC hdc, int x1, int y1, int w, int h, COLORREF color);
void DirectDrawArc(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color);
void DirectDrawEllipse(HDC hdc, int x1, int y1, int w, int h, COLORREF color);
void DirectFillEllipse(HDC hdc, int x1, int y1, int w, int h, COLORREF color);
void DirectDrawPie(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color);
void DirectDrawRoundedRectangle(HDC hdc, int x1, int y1, int w, int h, int rx, int ry, COLORREF color);
void DirectFillRoundedRectangle(HDC hdc, int x1, int y1, int w, int h, int rx, int ry, COLORREF color);
void DirectFillPie(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color);
void DirectFillTrapezoid(HDC hdc, int x1, int y1, int x2, int y2, int w1, int w2, COLORREF color);
void DirectFillTriangle(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color);

HDC hdc = 0;
void DrawLine(const DrawCommand & cmd) {
    //DirectDrawLine(hdc, cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.color);
    drawlinex(cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.color);
}
void DrawLineTo(const DrawCommand & cmd) {
    if (draw_x >= 0) {
        DrawCommand cmd2;
        cmd2.x1 = draw_x;
        cmd2.y1 = draw_y;
        cmd2.x2 = cmd.x2;
        cmd2.y2 = cmd.y2;
        cmd2.color = cmd.color;
        DrawLine(cmd2);
    }
    draw_x = cmd.x2;
    draw_y = cmd.y2;
}
void DrawLineInit(const DrawCommand & cmd) {
    DrawLineInit();
}
void DrawRectangle(const DrawCommand & cmd) {
    //DirectDrawRectangle(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.color);
    drawboxx(cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.color);
}
void DrawEllipse(const DrawCommand & cmd) {
    // DirectDrawEllipse(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.color);
    drawarcx(cmd.x1, cmd.y1, cmd.w, cmd.h, 0, 360, cmd.color);
}
void DrawArc(const DrawCommand & cmd) {
    // DirectDrawArc(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.start_angle, cmd.sweep_angle, cmd.color);
    drawarcx(cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.start_angle, cmd.sweep_angle, cmd.color);
}
void DrawPie(const DrawCommand & cmd) {
    // DirectDrawPie(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.start_angle, cmd.sweep_angle, cmd.color);
    drawpiex(cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.start_angle, cmd.sweep_angle, cmd.color);
}
void DrawRoundedRectangle(const DrawCommand & cmd) {
    //DirectDrawRoundedRectangle(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.rx, cmd.ry, cmd.color);
    drawroundedrectanglex(cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.rx, cmd.ry, cmd.color);
}
void FillRectangle(const DrawCommand & cmd) {
    //DirectFillRectangle(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.color);
    fillboxx(cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.color);
}
void FillEllipse(const DrawCommand & cmd) {
    // DirectFillEllipse(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.color);
    fillellipsex(cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.color);
}
void FillPie(const DrawCommand & cmd) {
    DirectFillPie(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.start_angle, cmd.sweep_angle, cmd.color);
}
void FillRoundedRectangle(const DrawCommand & cmd) {
    // DirectFillRoundedRectangle(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.rx, cmd.ry, cmd.color);
    fillroundedrectanglex(cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.rx, cmd.ry, cmd.color);
}
void FillTrapezoid(const DrawCommand & cmd) {
    // DirectFillTrapezoid(hdc, cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.w, cmd.w2, cmd.color);
    filltrapezoidx(cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.w, cmd.w2, cmd.color);
}
void FillTriangle(const DrawCommand & cmd) {
    // DirectFillTriangle(hdc, cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.x3, cmd.y3, cmd.color);
    filltrianglex(cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.x3, cmd.y3, cmd.color);
}
void XLine(const DrawCommand & cmd) {
    uint16_t data[6] = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         static_cast<uint16_t>(cmd.x2), static_cast<uint16_t>(cmd.y2),
                         static_cast<uint16_t>(cmd.color), default_line_style };
    drawline(data);
}
void XBox(const DrawCommand & cmd) {
    uint16_t data[6] = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         static_cast<uint16_t>(cmd.x1 + cmd.w), static_cast<uint16_t>(cmd.y1 + cmd.h),
                         static_cast<uint16_t>(cmd.color), default_line_style };
    drawbox(data);
}
int16_t hw_ratio(int h, int w) {
    double h_by_w = (double)h / (double)w;
    return h_by_w * 256;
}
void XCircle(const DrawCommand & cmd) {
    int16_t data[7] = { static_cast<int16_t>(cmd.x1 + cmd.w / 2), static_cast<int16_t>(cmd.y1 + cmd.h / 2),
                        static_cast<int16_t>(cmd.w / 2), // static_cast<int16_t>(cmd.h / 2),
                        static_cast<int16_t>(cmd.color), 0, 360,
                        hw_ratio(cmd.h, cmd.w) };
    drawcircle(data);
}
void XPie(const DrawCommand & cmd) {
    int16_t data[7] = { static_cast<int16_t>(cmd.x1 + cmd.w / 2), static_cast<int16_t>(cmd.y1 + cmd.h / 2),
                        static_cast<int16_t>(cmd.w / 2), // static_cast<int16_t>(cmd.h / 2),
                        static_cast<int16_t>(cmd.color), static_cast<int16_t>(-cmd.start_angle), static_cast<int16_t>(-cmd.start_angle - cmd.sweep_angle),
                        hw_ratio(cmd.h, cmd.w) };
    drawcircle(data);
}
void XFill(const DrawCommand & cmd) {
    uint16_t data[5] = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         static_cast<uint16_t>(cmd.x1 + cmd.w), static_cast<uint16_t>(cmd.y1 + cmd.h),
                         static_cast<uint16_t>(cmd.color) };
    fillbox(data);
}
#else
void DrawLine(const DrawCommand & cmd) {
    uint16_t data[6] = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         static_cast<uint16_t>(cmd.x2), static_cast<uint16_t>(cmd.y2),
                         static_cast<uint16_t>(cmd.color), default_line_style };
    drawline(data);
}
void DrawLineTo(const DrawCommand & cmd) {
    if (draw_x >= 0) {
        DrawCommand cmd2;
        cmd2.x1 = draw_x;
        cmd2.y1 = draw_y;
        cmd2.x2 = cmd.x2;
        cmd2.y2 = cmd.y2;
        cmd2.color = cmd.color;
        DrawLine(cmd2);
    }
    draw_x = cmd.x2;
    draw_y = cmd.y2;
}
void DrawRectangle(const DrawCommand & cmd) {
    uint16_t data[6] = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         static_cast<uint16_t>(cmd.x1 + cmd.w), static_cast<uint16_t>(cmd.y1 + cmd.h),
                         static_cast<uint16_t>(cmd.color), default_line_style };
    drawbox(data);
}
int16_t hw_ratio(int h, int w) {
    double h_by_w = (double)h / (double)w;
    return h_by_w * 256;
}
void DrawEllipse(const DrawCommand & cmd) {
    int16_t data[7] = { static_cast<int16_t>(cmd.x1 + cmd.w / 2), static_cast<int16_t>(cmd.y1 + cmd.h / 2),
                        static_cast<int16_t>(cmd.w / 2), // static_cast<int16_t>(cmd.h / 2),
                        static_cast<int16_t>(cmd.color), 0, 360,
                        hw_ratio(cmd.h, cmd.w) };
    drawcircle(data);
}
void DrawArc(const DrawCommand & cmd) {
    int16_t data[7] = { static_cast<int16_t>(cmd.x1 + cmd.w / 2), static_cast<int16_t>(cmd.y1 + cmd.h / 2),
                        static_cast<int16_t>(cmd.w / 2), // static_cast<int16_t>(cmd.h / 2),
                        static_cast<int16_t>(cmd.color), static_cast<int16_t>(cmd.start_angle), static_cast<int16_t>(cmd.start_angle + cmd.sweep_angle),
                        hw_ratio(cmd.h, cmd.w) };
    drawcircle(data);
}
void DrawPie(const DrawCommand & cmd) {
    int16_t data[7] = { static_cast<int16_t>(cmd.x1 + cmd.w / 2), static_cast<int16_t>(cmd.y1 + cmd.h / 2),
                        static_cast<int16_t>(cmd.w / 2), // static_cast<int16_t>(cmd.h / 2),
                        static_cast<int16_t>(cmd.color), static_cast<int16_t>(-cmd.start_angle), static_cast<int16_t>(-cmd.start_angle - cmd.sweep_angle),
                        hw_ratio(cmd.h, cmd.w) };
    drawcircle(data);
}
void GDrawLine(COLORREF color, int x1, int y1, int x2, int y2) {
    uint16_t data[6] = { static_cast<uint16_t>(x1), static_cast<uint16_t>(y1),
                         static_cast<uint16_t>(x2), static_cast<uint16_t>(y2),
                         static_cast<uint16_t>(color), default_line_style };
    drawline(data);
}
void GDrawArc(COLORREF color, int x, int y, int width, int height, int startAngle, int sweepAngle) {
    int16_t data[7] = { static_cast<int16_t>(x + width / 2), static_cast<int16_t>(y + height / 2),
                        static_cast<int16_t>(width / 2), static_cast<int16_t>(height / 2),
                        static_cast<int16_t>(color), static_cast<int16_t>(startAngle), static_cast<int16_t>(startAngle + sweepAngle) };
    drawcircle(data);
}
void DrawRoundedRectangle(const DrawCommand & cmd) {
    int x = cmd.x1, y = cmd.y1, width = cmd.w, height = cmd.h;
    int rx = cmd.rx;
    int ry = cmd.ry;
    int dx = rx * 2, dy = ry * 2;
    COLORREF color = cmd.color;
    GDrawLine(color, x + rx, y, x + width - rx, y);
    GDrawLine(color, x + rx, y + height, x + width - rx, y + height);
    GDrawLine(color, x, y + ry, x, y + height - ry);
    GDrawLine(color, x + width, y + ry, x + width, y + height - ry);
    GDrawArc(color, x, y, dx, dy, 180, 90);
    GDrawArc(color, x + width - dx, y, dx, dy, 270, 90);
    GDrawArc(color, x, y + height - dy, dx, dy, 90, 90);
    GDrawArc(color, x + width - dx, y + height - dy, dx, dy, 0, 90);
}
void FillRectangle(const DrawCommand & cmd) {
    uint16_t data[5] = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         static_cast<uint16_t>(cmd.x1 + cmd.w), static_cast<uint16_t>(cmd.y1 + cmd.h),
                         static_cast<uint16_t>(cmd.color) };
    fillbox(data);
}
void FillEllipse(const DrawCommand & cmd) {
    int16_t data[5] = { static_cast<int16_t>(cmd.x1 + cmd.w / 2), static_cast<int16_t>(cmd.y1 + cmd.h / 2),
                        static_cast<int16_t>(cmd.w / 2), static_cast<int16_t>(cmd.h / 2),
                        static_cast<int16_t>(cmd.color) };
    drawcircle(data);
}
void FillPie(const DrawCommand & cmd) {
    int16_t data[7] = { static_cast<int16_t>(cmd.x1 + cmd.w / 2), static_cast<int16_t>(cmd.y1 + cmd.h / 2),
                        static_cast<int16_t>(cmd.w / 2), // static_cast<int16_t>(cmd.h / 2),
                        static_cast<int16_t>(cmd.color), static_cast<int16_t>(-cmd.start_angle), static_cast<int16_t>(-cmd.start_angle - cmd.sweep_angle),
                        hw_ratio(cmd.h, cmd.w) };
    drawcircle(data);
}
void GFillRectangle(COLORREF color, int x, int y, int width, int height) {
    uint16_t data[5] = { static_cast<uint16_t>(x), static_cast<uint16_t>(y),
                         static_cast<uint16_t>(x + width), static_cast<uint16_t>(y + height),
                         static_cast<uint16_t>(color) };
    fillbox(data);
}
void GFillPie(COLORREF color, int x, int y, int width, int height, int startAngle, int sweepAngle) {
    int16_t data[7] = { static_cast<int16_t>(x + width / 2), static_cast<int16_t>(y + height / 2),
                        static_cast<int16_t>(width / 2), static_cast<int16_t>(height / 2),
                        static_cast<int16_t>(color), static_cast<int16_t>(-startAngle), static_cast<int16_t>(-startAngle - sweepAngle) };
    drawcircle(data);
}
void FillRoundedRectangle(const DrawCommand & cmd) {
    int x = cmd.x1, y = cmd.y1, width = cmd.w, height = cmd.h;
    int rx = cmd.rx;
    int ry = cmd.ry;
    int dx = rx * 2, dy = ry * 2;
    COLORREF color = cmd.color;
    GFillRectangle(color, x + rx, y, width - dx, ry);
    GFillRectangle(color, x + rx, y + height - ry, width - dx, ry);
    GFillRectangle(color, x, y + ry, rx, height - dy);
    GFillRectangle(color, x + width - rx, y + ry, rx, height - dy);
    GFillRectangle(color, x + rx, y + ry, width - dx, height - dy);
    GFillPie(color, x, y, dx, dy, 180, 90);
    GFillPie(color, x + width - dx, y, dx, dy, 270, 90);
    GFillPie(color, x, y + height - dy, dx, dy, 90, 90);
    GFillPie(color, x + width - dx, y + height - dy, dx, dy, 0, 90);
}
#endif
void DrawString(const DrawCommand & cmd) {
    DrawStringStruct data = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
        nullptr, 1, 1, static_cast<uint16_t>(cmd.color), 0, 0 };
    data.str = cmd.text;
    drawstring(&data);
}
void Circle(const DrawCommand & cmd) {
    int16_t data[8] = { static_cast<int16_t>(cmd.x1), static_cast<int16_t>(cmd.y1),
                        static_cast<int16_t>(cmd.r),
                        static_cast<int16_t>(cmd.color),
                        static_cast<int16_t>(cmd.start_angle),
                        static_cast<int16_t>(cmd.end_angle),
                        static_cast<int16_t>(cmd.hbyw) };
    drawcircle(data);
}
void Paint(const DrawCommand & cmd) {
    char work[1000]; // 適当なサイズのワーク領域
    PaintStruct data = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         static_cast<uint16_t>(cmd.color), work, work + sizeof work };
    paint(&data);
}
void XPaint(const DrawCommand & cmd) {
    char work[1000]; // 適当なサイズのワーク領域
    PaintStruct data = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         static_cast<uint16_t>(cmd.color), work, work + sizeof work };
    paint(&data);
}
void Pset(const DrawCommand & cmd) {
    uint16_t data[3] = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         static_cast<uint16_t>(cmd.color) };
    pset(data);
}
void XPset(const DrawCommand & cmd) {
    uint16_t data[3] = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         static_cast<uint16_t>(cmd.color) };
    pset(data);
}
void Point(DrawCommand & cmd) {
    uint16_t data[3] = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         0 };
    point(data);
    cmd.color = data[2]; // 結果をcmd.colorに格納
}
void XPoint(DrawCommand & cmd) {
    uint16_t data[3] = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
                         0 };
    point(data);
    cmd.color = data[2]; // 結果をcmd.colorに格納
}
void Symbol(const DrawCommand & cmd) {
    DrawStringStruct data = { static_cast<uint16_t>(cmd.x1), static_cast<uint16_t>(cmd.y1),
        cmd.text, cmd.mx, cmd.my, static_cast<uint16_t>(cmd.color), cmd.font_size, cmd.rotation };
    drawstring(&data);
}
void Apage(const DrawCommand & cmd) {
    apage(cmd.page);
}
void Vpage(const DrawCommand & cmd) {
    vpage(cmd.page);
}
void Wipe(const DrawCommand & cmd) {
    wipe();
}
bool check_sc1(const DrawCommand & cmd, int mode) {
    if (cmd.sc1 == 0 && (mode == 2 || mode == 3 || mode == 6 || mode == 7 || mode == 10 || mode == 11 || mode == 14)) return true;
    if (cmd.sc1 == 1 && (mode == 0 || mode == 1 || mode == 4 || mode == 5 || mode == 8 || mode == 9 || mode == 12 || mode == 13 || mode == 15)) return true;
    if (cmd.sc1 == 2 && mode == 16) return true;
    return false;
}
bool check_sc2(const DrawCommand & cmd, int mode) {
    if (cmd.sc2 == 0 && (mode == 0 || mode == 1 || mode == 2 || mode == 3 || mode == 16 || mode == 17 || mode == 18)) return true;
    if (cmd.sc2 == 1 && (mode == 4 || mode == 5 || mode == 6 || mode == 7)) return true;
    if (cmd.sc2 == 2 && (mode == 8 || mode == 9 || mode == 10 || mode == 11)) return true;
    if (cmd.sc2 == 3 && (mode == 12 || mode == 13 || mode == 14 || mode == 15)) return true;
    return false;
}
bool check_sc3(const DrawCommand & cmd, int mode) {
    if (cmd.sc3 == 0 && (mode == 1 || mode == 3 || mode == 5 || mode == 7 || mode == 9 || mode == 11 || mode == 13 || mode == 15)) return true;
    if (cmd.sc3 == 1 && (mode == 0 || mode == 2 || mode == 4 || mode == 6 || mode == 8 || mode == 10 || mode == 12 || mode == 14)) return true;
    return false;
}
int get_crt_mode(const DrawCommand & cmd) {
    for (int mode = 0; mode <= 18; ++mode) {
        if (check_sc1(cmd, mode) && check_sc2(cmd, mode) && check_sc3(cmd, mode)) {
            return mode;
        }
    }
    return -1; // モードが見つからない場合
}
void Screen(const DrawCommand & cmd) {
    // sc1: 表示画面サイズ
    //   0…256×256  mode=2,3,6,7,10,11,14
    //   1…512×512  mode=0,1,4,5,8.9,12,13,15
    //   2…768×512  mode=16
    // sc2: グラフィック画面の実画面サイズ及び色モード
    //   0…1024×1024、16色(グラフィックページ0)   mode=0,1,2,3,16,17,18
    //   1…512×512、16色(グラフィックページ0～3)  mode=4,5,6,7
    //   2…512×512、256色(グラフィックページ0、1) mode=8,9,10,11
    //   3…512×512、65536色(グラフィックページ0)  mode=12,13,14,15
    // sc3: ディスプレイ解像度
    //   0…Low（標準解像度）mpde=1,3,5,7,9,11,13,15
    //   1…High（高解像度） mode=0,2,4,6,8,10,12,14
    // sc4: グラフィック画面の表示ON/OFF
    //   0…表示OFF(グラフィック関数使用不可)
    //   1…表示ON(グラフィック関数使用可)
    int mode = get_crt_mode(cmd);
    if (mode >= 0) {
        crtmod(mode);
        if (cmd.sc4 == 1) {
            gclear(); // グラフィック画面をクリア
        }
    }
}
void Window(const DrawCommand & cmd) {
    window(cmd.x1, cmd.y1, cmd.x2, cmd.y2);
}
void Test(DrawCommand & cmd) {
    test();
}
#endif

#ifdef USE_X68000
void exec_command_x(DrawCommand& cmd) {
    print_command(cmd); // デバッグ用にコマンドを表示
    switch (cmd.type) {
    case DrawType::Clear:
        Clear(cmd);
        break;
    case DrawType::Line:
        DrawLine(cmd);
        break;
    case DrawType::LineTo:
        DrawLineTo(cmd);
        break;
    case DrawType::LineInit:
        DrawLineInit(cmd);
        break;
    case DrawType::String:
        DrawString(cmd);
        break;
    case DrawType::Rect:
        DrawRectangle(cmd);
        break;
    case DrawType::Ellipse:
        DrawEllipse(cmd);
        break;
    case DrawType::Arc:
        DrawArc(cmd);
        break;
    case DrawType::Pie:
        DrawPie(cmd);
        break;
    case DrawType::RoundRect:
        DrawRoundedRectangle(cmd);
        break;
    case DrawType::FillRect:
        FillRectangle(cmd);
        break;
    case DrawType::FillEllipse:
        FillEllipse(cmd);
        break;
    case DrawType::FillPie:
        FillPie(cmd);
        break;
    case DrawType::FillRoundRect:
        FillRoundedRectangle(cmd);
        break;
    case DrawType::FillTrapezoid:
        FillTrapezoid(cmd);
        break;
    case DrawType::FillTriangle:
        FillTriangle(cmd);
        break;
    case DrawType::Circle:
        Circle(cmd);
        break;
    case DrawType::Paint:
        Paint(cmd);
        break;
    case DrawType::Pset:
        Pset(cmd);
        break;
    case DrawType::Point:
        Point(cmd);
        break;
    case DrawType::Symbol:
        Symbol(cmd);
        break;
    case DrawType::Apage:
        Apage(cmd);
        break;
    case DrawType::Vpage:
        Vpage(cmd);
        break;
    case DrawType::Wipe:
        Wipe(cmd);
        break;
    case DrawType::Screen:
        Screen(cmd);
        break;
    case DrawType::Window:
        Window(cmd);
        break;
    case DrawType::XClear:
        XClear(cmd);
        break;
    case DrawType::XLine:
        XLine(cmd);
        break;
    case DrawType::XBox:
        XBox(cmd);
        break;
    case DrawType::XFill:
        XFill(cmd);
        break;
    case DrawType::XCircle:
        XCircle(cmd);
        break;
    case DrawType::XPie:
        XPie(cmd);
        break;
    case DrawType::XPaint:
        XPaint(cmd);
        break;
    case DrawType::XPset:
        XPset(cmd);
        break;
    case DrawType::XPoint:
        XPoint(cmd);
        break;
    case DrawType::Test:
        Test(cmd);
        break;
    }
}
#endif

#ifndef USE_X68000
#include <stack>
using namespace std;
void Paint_(HDC hdc, int x, int y, COLORREF bc)
{
    stack<POINT> points;
    points.push(POINT(x, y));
    while (points.size() > 0)
    {
        if (points.size() > 100) {
            break;
        }
        POINT p = points.top();
        points.pop();
        int a = p.x, b = p.y;
        COLORREF c = GetPixel(hdc, a, b);
        if (c != bc)
        {
            SetPixel(hdc, a, b, bc);
            points.push(POINT(a - 1, b));
            points.push(POINT(a + 1, b));
            points.push(POINT(a, b - 1));
            points.push(POINT(a, b + 1));
        }
    }
}
void change_color(HDC hdc, int x, int y, COLORREF bc, stack<POINT>&points)
{
    COLORREF c = GetPixel(hdc, x, y);
    if (c == bc) {
        return; // 既に同じ色なら何もしない
    }
    if (c != bc) {
        SetPixel(hdc, x, y, bc);
        points.push(POINT(x, y));
    }
}
void Paint(HDC hdc, int x, int y, COLORREF bc)
{
    stack<POINT> points;
    change_color(hdc, x, y, bc, points);
    while (points.size() > 0)
    {
        if (points.size() > 1000) {
            // 失敗したときのために、スタックのサイズを制限
            break;
        }
        POINT p = points.top();
        points.pop();
        change_color(hdc, p.x - 1, p.y, bc, points);
        change_color(hdc, p.x + 1, p.y, bc, points);
        change_color(hdc, p.x, p.y - 1, bc, points);
        change_color(hdc, p.x, p.y + 1, bc, points);
    }
}
void DrawTextOutW(HDC hdc, int x, int y, const char* text) {
    // char* を wchar_t* に変換
    wchar_t wtext[256]; // 必要に応じてサイズ調整
    int wlen = MultiByteToWideChar(
        CP_ACP,        // コードページ（例: ANSI。必要なら CP_UTF8 に）
        0,
        text,
        -1,            // 入力文字列の長さ。-1 は NULL 終端まで
        wtext,
        sizeof(wtext) / sizeof(wchar_t)
    );

    if (wlen == 0) {
        // 変換失敗
        printf("MultiByteToWideChar failed (error %lu)\n", GetLastError());
        return;
    }

    // TextOutW で出力
    TextOutW(hdc, x, y, wtext, wlen - 1); // wlen には終端の NULL 含むため -1
}
#endif

#ifdef USE_X68000
void drawarcx(int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color) {
    g_xargs[0] = x1 + w / 2;
    g_xargs[1] = y1 + h / 2;
    g_xargs[2] = h / 2;
    g_xargs[3] = color;
    g_xargs[4] = start_angle;
    g_xargs[5] = start_angle + sweep_angle;
    g_xargs[6] = h * 256 / w; // aspect_ratio = h / w * 256
    asm(
        "move.l #0xbb, %%d0\n\t" // _CIRCLE
        "lea    g_xargs, %%a1\n\t"    // a1 = [cx, cy, r, color, start_angle, end_angle, aspect_ratio]
        "trap   #15\n\t"
        :
    :                        // 入力
        : "d0", "a1"             // 使用するレジスタ
        );
}
void drawpiex(int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color) {
    g_xargs[0] = x1 + w / 2;
    g_xargs[1] = y1 + h / 2;
    g_xargs[2] = h / 2;
    g_xargs[3] = color;
    g_xargs[4] = -start_angle;
    g_xargs[5] = -start_angle - sweep_angle;
    g_xargs[6] = h * 256 / w; // aspect_ratio = h / w * 256
    asm(
        "move.l #0xbb, %%d0\n\t" // _CIRCLE
        "lea    g_xargs, %%a1\n\t"    // a1 = [cx, cy, r, color, start_angle, end_angle, aspect_ratio]
        "trap   #15\n\t"
        :
    :                        // 入力
        : "d0", "a1"             // 使用するレジスタ
        );
}
void drawroundedrectanglex(int x1, int y1, int w, int h, int rx, int ry, COLORREF color) {
    int dx = rx * 2;
    int dy = ry * 2;
    drawlinex(x1 + rx, y1, x1 + w - rx, y1, color);
    drawlinex(x1 + rx, y1 + h, x1 + w - rx, y1 + h, color);
    drawlinex(x1, y1 + ry, x1, y1 + h - ry, color);
    drawlinex(x1 + w, y1 + ry, x1 + w, y1 + h - ry, color);
    drawarcx(x1, y1, dx, dy, 180, 90, color);
    drawarcx(x1 + w - dx, y1, dx, dy, 270, 90, color);
    drawarcx(x1, y1 + h - dy, dx, dy, 90, 90, color);
    drawarcx(x1 + w - dx, y1 + h - dy, dx, dy, 0, 90, color);
}
void fillellipsex(int x1, int y1, int w, int h, COLORREF color) {
    asm(
        "move.l   %0, g_x1\n\t"
        "move.l   %1, g_y1\n\t"
        "move.l   %2, g_w\n\t"
        "move.l   %3, g_h\n\t"
        "move.l   %4, g_color\n\t"

        "clr.l	  -(%%sp)\n\t"
        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードへ)
        "move.l   %%d0, (%%sp)\n\t"

        "move.l   g_color, -(%%sp)\n\t"
        "move.l   g_h, -(%%sp)\n\t"
        "move.l   g_w, -(%%sp)\n\t"
        "move.l   g_y1, -(%%sp)\n\t"
        "move.l   g_x1, -(%%sp)\n\t"
        "move.l   #0, -(%%sp)\n\t"
        //"jsr      fillellipsex_a\n\t"
        "jsr      fillellipse_asm\n\t"
        "add.l    #24,%%sp\n\t"

        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードから戻る)
        "addq.l   #4, %%sp\n\t"
        :                        // 出力
        : "r"(x1), "r"(y1), "r"(w), "r"(h), "r"(color) // 入力
        : "d0"                   // 使用するレジスタ
        );
}
void fillroundedrectanglex(int x1, int y1, int w, int h, int rx, int ry, COLORREF color) {
    asm(
        "move.l   %0, g_x1\n\t"
        "move.l   %1, g_y1\n\t"
        "move.l   %2, g_w\n\t"
        "move.l   %3, g_h\n\t"
        "move.l   %4, g_rx\n\t"
        "move.l   %5, g_ry\n\t"
        "move.l   %6, g_color\n\t"

        "clr.l	  -(%%sp)\n\t"
        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードへ)
        "move.l   %%d0, (%%sp)\n\t"

        "move.l   g_color, -(%%sp)\n\t"
        "move.l   g_ry, -(%%sp)\n\t"
        "move.l   g_rx, -(%%sp)\n\t"
        "move.l   g_h, -(%%sp)\n\t"
        "move.l   g_w, -(%%sp)\n\t"
        "move.l   g_y1, -(%%sp)\n\t"
        "move.l   g_x1, -(%%sp)\n\t"
        "move.l   #0, -(%%sp)\n\t"
        //"jsr      fillroundedrectanglex_a\n\t"
        "jsr      fillroundedrectangle_asm\n\t"
        "add.l    #32,%%sp\n\t"

        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードから戻る)
        "addq.l   #4, %%sp\n\t"
        :                        // 出力
    : "r"(x1), "r"(y1), "r"(w), "r"(h), "r"(rx), "r"(ry), "r"(color) // 入力
        : "d0"                   // 使用するレジスタ
        );
}
void filltrapezoidx(int x1, int y1, int x2, int y2, int w1, int w2, COLORREF color) {
    asm(
        "move.l   %0, g_x1\n\t"
        "move.l   %1, g_y1\n\t"
        "move.l   %2, g_x2\n\t"
        "move.l   %3, g_y2\n\t"
        "move.l   %4, g_w\n\t"
        "move.l   %5, g_w2\n\t"
        "move.l   %6, g_color\n\t"

        "clr.l	  -(%%sp)\n\t"
        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードへ)
        "move.l   %%d0, (%%sp)\n\t"

        "move.l   g_color, -(%%sp)\n\t"
        "move.l   g_w2, -(%%sp)\n\t"
        "move.l   g_w, -(%%sp)\n\t"
        "move.l   g_y2, -(%%sp)\n\t"
        "move.l   g_x2, -(%%sp)\n\t"
        "move.l   g_y1, -(%%sp)\n\t"
        "move.l   g_x1, -(%%sp)\n\t"
        "move.l   #0, -(%%sp)\n\t"
        "jsr      filltrapezoid_asm\n\t"
        "add.l    #32,%%sp\n\t"

        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードから戻る)
        "addq.l   #4, %%sp\n\t"
        :                        // 出力
    : "r"(x1), "r"(y1), "r"(x2), "r"(y2), "r"(w1), "r"(w2), "r"(color) // 入力
        : "d0"                   // 使用するレジスタ
        );
}
void filltrianglex(int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color) {
    asm(
        "move.l   %0, g_x1\n\t"
        "move.l   %1, g_y1\n\t"
        "move.l   %2, g_x2\n\t"
        "move.l   %3, g_y2\n\t"
        "move.l   %4, g_x3\n\t"
        "move.l   %5, g_y3\n\t"
        "move.l   %6, g_color\n\t"

        "clr.l	  -(%%sp)\n\t"
        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードへ)
        "move.l   %%d0, (%%sp)\n\t"

        "move.l   g_color, -(%%sp)\n\t"
        "move.l   g_y3, -(%%sp)\n\t"
        "move.l   g_x3, -(%%sp)\n\t"
        "move.l   g_y2, -(%%sp)\n\t"
        "move.l   g_x2, -(%%sp)\n\t"
        "move.l   g_y1, -(%%sp)\n\t"
        "move.l   g_x1, -(%%sp)\n\t"
        "move.l   #0, -(%%sp)\n\t"
        "jsr      filltriangle_asm\n\t"
        "add.l    #32,%%sp\n\t"

        "dc.w	  0xff20\n\t"      // _SUPER (スーパーバイザーモードから戻る)
        "addq.l   #4, %%sp\n\t"
        :                        // 出力
    : "r"(x1), "r"(y1), "r"(x2), "r"(y2), "r"(x3), "r"(y3), "r"(color) // 入力
        : "d0"                   // 使用するレジスタ
        );
}
#endif

#ifdef PIXEL_DRAW_MODE
void DirectDrawLine__(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    int w = x2 - x1;
    int h = y2 - y1;
    if (abs(h) < abs(w)) {
        int d = w >= 0 ? 1 : -1;
        for (int x = x1; x * d < x2 * d; x += d) {
            int y = y1 + (h * (x - x1)) / w;
            SetPixel(hdc, x, y, color);
        }
    }
    else {
        int d = h >= 0 ? 1 : -1;
        for (int y = y1; y * d < y2 * d; y += d) {
            int x = x1 + (w * (y - y1)) / h;
            SetPixel(hdc, x, y, color);
        }
    }
}
void DirectDrawLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
#ifndef USE_X68000
    ConOutput co_cout(hOutput);
    co_cout << "DirectDrawLine" << co_endl;
    co_cout << "  x1 = " << x1 << co_endl;
    co_cout << "  y1 = " << y1 << co_endl;
    co_cout << "  x2 = " << x2 << co_endl;
    co_cout << "  y2 = " << y2 << co_endl;
    co_cout << "  color = " << (int)color << co_endl;
#endif
    int w = x2 - x1;
    int h = y2 - y1;
    if (abs(h) < abs(w)) {
        if (x1 > x2) {
            int t = x1; x1 = x2; x2 = t;
            t = y1; y1 = y2; y2 = t;
            w = -w;
            h = -h;
        }
        for (int x = x1; x < x2; x++) {
            int y = y1 + (h * (x - x1)) / w;
            SetPixel(hdc, x, y, color);
        }
    }
    else {
        if (y1 > y2) {
            int t = x1; x1 = x2; x2 = t;
            t = y1; y1 = y2; y2 = t;
            w = -w;
            h = -h;
        }
        for (int y = y1; y < y2; y++) {
            int x = x1 + (w * (y - y1)) / h;
            SetPixel(hdc, x, y, color);
        }
    }
}
int direct_draw_x = -1;
int direct_draw_y = -1;
void DirectDrawLineInit() {
    direct_draw_x = -1;
}
void DirectDrawLineTo(HDC hdc, int x2, int y2, COLORREF color) {
#ifndef USE_X68000
    ConOutput co_cout(hOutput);
    co_cout << "DirectDrawLineTo" << co_endl;
    co_cout << "  x2 = " << x2 << co_endl;
    co_cout << "  y2 = " << y2 << co_endl;
    co_cout << "  color = " << (int)color << co_endl;
#endif
    if (direct_draw_x >= 0) {
        DirectDrawLine(hdc, direct_draw_x, direct_draw_y, x2, y2, color);
    }
    direct_draw_x = x2;
    direct_draw_y = y2;
}
void DirectDrawRectangle(HDC hdc, int x1, int y1, int w, int h, COLORREF color) {
    for (int x = x1; x < x1 + w; ++x) {
        SetPixel(hdc, x, y1, color);
        SetPixel(hdc, x, y1 + h, color);
    }
    for (int y = y1; y < y1 + h; ++y) {
        SetPixel(hdc, x1, y, color);
        SetPixel(hdc, x1 + w, y, color);
    }
}
void DirectFillRectangle(HDC hdc, int x1, int y1, int w, int h, COLORREF color) {
    for (int y = y1; y < y1 + h; ++y) {
        for (int x = x1; x < x1 + w; ++x) {
            SetPixel(hdc, x, y, color);
        }
    }
}
void DirectFillRectangle_hline(HDC hdc, int x1, int y1, int w, int h, COLORREF color) {
    for (int y = y1; y < y1 + h; ++y) {
        DrawHLine(hdc, x1, y, w, color);
    }
}
float sine_table[] = {
    0.0000000,
    0.0871557,
    0.1736482,
    0.2588190,
    0.3420201,
    0.4226183,
    0.5000000,
    0.5735764,
    0.6427876,
    0.7071068,
    0.7660444,
    0.8191520,
    0.8660254,
    0.9063078,
    0.9396926,
    0.9659258,
    0.9848078,
    0.9961947,
    1.0000000,
    0.9961947,
    0.9848078,
    0.9659258,
    0.9396926,
    0.9063078,
    0.8660254,
    0.8191520,
    0.7660444,
    0.7071068,
    0.6427876,
    0.5735764,
    0.5000000,
    0.4226183,
    0.3420201,
    0.2588190,
    0.1736482,
    0.0871557,
    0.0000000,
    -0.0871557,
    -0.1736482,
    -0.2588190,
    -0.3420201,
    -0.4226183,
    -0.5000000,
    -0.5735764,
    -0.6427876,
    -0.7071068,
    -0.7660444,
    -0.8191520,
    -0.8660254,
    -0.9063078,
    -0.9396926,
    -0.9659258,
    -0.9848078,
    -0.9961947,
    -1.0000000,
    -0.9961947,
    -0.9848078,
    -0.9659258,
    -0.9396926,
    -0.9063078,
    -0.8660254,
    -0.8191520,
    -0.7660444,
    -0.7071068,
    -0.6427876,
    -0.5735764,
    -0.5000000,
    -0.4226183,
    -0.3420201,
    -0.2588191,
    -0.1736482,
    -0.0871557,
    -0.0000000,
    0.0871557,
    0.1736482,
    0.2588190,
    0.3420201,
    0.4226183,
    0.5000000,
    0.5735764,
    0.6427876,
    0.7071068,
    0.7660444,
    0.8191520,
    0.8660254,
    0.9063078,
    0.9396926,
    0.9659258,
    0.9848078,
    0.9961947,
    1.0000000,
};
inline float tab_sin(int deg) {
    if (deg < 0 || deg > 360) {
        return 0.0f; // 範囲外の角度は0を返す
    }
    int index = deg / 5; // 5度刻みのインデックス
    return sine_table[index];
}
inline float tab_cos(int deg) {
    if (deg < 0 || deg > 360) {
        return 0.0f; // 範囲外の角度は0を返す
    }
    int index = (deg + 90) / 5; // cosは90度ずらす
    return sine_table[index];
}
inline void DirectDrawArcLineTo(HDC hdc, int cx, int cy, int w, int h, int a, COLORREF color) {
#ifndef USE_X68000
    ConOutput co_cout(hOutput);
    co_cout << "DirectDrawArcLineTo" << co_endl;
    co_cout << "  cx = " << cx << co_endl;
    co_cout << "  cy = " << cy << co_endl;
    co_cout << "  w  = " << w << co_endl;
    co_cout << "  h  = " << h << co_endl;
    co_cout << "  a  = " << a << co_endl;
    co_cout << "  color = " << (int)color << co_endl;
#endif
#ifdef USE_FUNC_TABLE
    int x = cx + (int)(tab_cos(a) * w / 2);
    int y = cy - (int)(tab_sin(a) * h / 2);
#else
    int x = cx + (int)(cos(deg_to_rad(a)) * w / 2);
    int y = cy - (int)(sin(deg_to_rad(a)) * h / 2);
#endif
    DirectDrawLineTo(hdc, x, y, color);
}
//inline void DirectDrawArcLineTo_tab(HDC hdc, int cx, int cy, int w, int h, int a, COLORREF color) {
//    int x = cx + (int)(tab_cos(a) * w / 2);
//    int y = cy - (int)(tab_sin(a) * h / 2);
//    DirectDrawLineTo(hdc, x, y, color);
//}
void DirectDrawArcTo(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color) {
#ifndef USE_X68000
    ConOutput co_cout(hOutput);
    co_cout << "DirectDrawArcTo" << co_endl;
    co_cout << "  x1 = " << x1 << co_endl;
    co_cout << "  y1 = " << y1 << co_endl;
    co_cout << "  w  = " << w << co_endl;
    co_cout << "  h  = " << h << co_endl;
    co_cout << "  start_angle = " << start_angle << co_endl;
    co_cout << "  sweep_angle = " << sweep_angle << co_endl;
    co_cout << "  color = " << (int)color << co_endl;
#endif
    int cx = x1 + w / 2;
    int cy = y1 + h / 2;
    for (int a = start_angle; a < start_angle + sweep_angle; a += 5) {
        DirectDrawArcLineTo(hdc, cx, cy, w, h, a, color);
    }
    DirectDrawArcLineTo(hdc, cx, cy, w, h, start_angle + sweep_angle, color);
}
//void DirectDrawArcTo_calc(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color) {
//    int cx = x1 + w / 2;
//    int cy = y1 + h / 2;
//    for (int a = start_angle; a < start_angle + sweep_angle; a += 5) {
//        int x = cx + (int)(cos(deg_to_rad(a)) * w / 2);
//        int y = cy - (int)(sin(deg_to_rad(a)) * h / 2);
//        DirectDrawLineTo(hdc, x, y, color);
//    }
//    {
//        int a = start_angle + sweep_angle;
//        int x = cx + (int)(cos(deg_to_rad(a)) * w / 2);
//        int y = cy - (int)(sin(deg_to_rad(a)) * h / 2);
//        DirectDrawLineTo(hdc, x, y, color);
//    }
//}
//inline void DirectDrawArcLineTo(HDC hdc, int x1, int y1, int w, int h, int a, COLORREF color) {
//    int cx = x1 + w / 2;
//    int cy = y1 + h / 2;
//    int x = cx + (int)(tab_cos(a) * w / 2);
//    int y = cy - (int)(tab_sin(a) * h / 2);
//    DirectDrawLineTo(hdc, x, y, color);
//}
//void DirectDrawArcTo(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color) {
//    for (int a = start_angle; a < start_angle + sweep_angle; a += 5) {
//        DirectDrawArcLineTo(hdc, x1, y1, w, h, a, color);
//    }
//    DirectDrawArcLineTo(hdc, x1, y1, w, h, start_angle + sweep_angle, color);
//}
//void DirectDrawArcTo(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color) {
//    int cx = x1 + w / 2;
//    int cy = y1 + h / 2;
//    for (int a = start_angle; a < start_angle + sweep_angle; a += 5) {
//        int x = cx + (int)(tab_cos(a) * w / 2);
//        int y = cy - (int)(tab_sin(a) * h / 2);
//        DirectDrawLineTo(hdc, x, y, color);
//    }
//    {
//        int a = start_angle + sweep_angle;
//        int x = cx + (int)(tab_cos(a) * w / 2);
//        int y = cy - (int)(tab_sin(a) * h / 2);
//        DirectDrawLineTo(hdc, x, y, color);
//    }
//}
void DirectDrawArc(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color) {
#ifndef USE_X68000
    ConOutput co_cout(hOutput);
    co_cout << "DirectDrawArc" << co_endl;
    co_cout << "  x1 = " << x1 << co_endl;
    co_cout << "  y1 = " << y1 << co_endl;
    co_cout << "  w  = " << w << co_endl;
    co_cout << "  h  = " << h << co_endl;
    co_cout << "  start_angle = " << start_angle << co_endl;
    co_cout << "  sweep_angle = " << sweep_angle << co_endl;
    co_cout << "  color = " << (int)color << co_endl;
#endif
    DirectDrawLineInit();
    DirectDrawArcTo(hdc, x1, y1, w, h, start_angle, sweep_angle, color);
}
void DirectDrawEllipse(HDC hdc, int x1, int y1, int w, int h, COLORREF color) {
    DirectDrawLineInit();
    DirectDrawArcTo(hdc, x1, y1, w, h, 0, 360, color);
}
void DirectDrawPie(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color) {
    DirectDrawLineInit();
    DirectDrawLineTo(hdc, x1 + w / 2, y1 + h / 2, color);
    DirectDrawArcTo(hdc, x1, y1, w, h, start_angle, sweep_angle, color);
    DirectDrawLineTo(hdc, x1 + w / 2, y1 + h / 2, color);
}
void DirectDrawRoundedRectangle(HDC hdc, int x1, int y1, int w, int h, int rx, int ry, COLORREF color) {
    // 角丸矩形を描く
    DirectDrawLineInit();
    // 右上の弧を描く
    DirectDrawArcTo(hdc, x1 + w - rx * 2, y1, rx * 2, ry * 2, 0, 90, color);
    // 上の直線を描く
    DirectDrawLineTo(hdc, x1 + rx, y1, color);
    // 左上の弧を描く
    DirectDrawArcTo(hdc, x1, y1, rx * 2, ry * 2, 90, 90, color);
    // 左の直線を描く
    DirectDrawLineTo(hdc, x1, y1 + h - ry, color);
    // 左下の弧を描く
    DirectDrawArcTo(hdc, x1, y1 + h - ry * 2, rx * 2, ry * 2, 180, 90, color);
    // 下の直線を描く
    DirectDrawLineTo(hdc, x1 + w - rx, y1 + h, color);
    // 右下の弧を描く
    DirectDrawArcTo(hdc, x1 + w - rx * 2, y1 + h - ry * 2, rx * 2, ry * 2, 270, 90, color);
    // 右の直線を描く
    DirectDrawLineTo(hdc, x1 + w, y1 + ry, color);
}
#ifdef USE_X68000
const int g_gwidth = 512;
const int g_gheight = 512;
int16_t g_gmemory[g_gheight][g_gwidth];
inline COLORREF SetPixel(HDC hdc, int x, int y, COLORREF color) {
    g_gmemory[y][x] = color;
    return color;
}
inline void DrawHLine(HDC hdc, int x1, int y1, int w, COLORREF color) {
    for (int x = x1; x <= x1 + w; ++x) {
        SetPixel(hdc, x, y1, color);
    }
}
#else
inline void DrawHLine(HDC hdc, int x1, int y1, int w, COLORREF color) {
    for (int x = x1; x <= x1 + w; ++x) {
        SetPixel(hdc, x, y1, color);
    }
}
//void DrawHLine(HDC hdc, int x1, int y1, int w, COLORREF color) {
//    HPEN hPen = CreatePen(PS_SOLID, 1, color);
//    SelectObject(hdc, hPen);
//    MoveToEx(hdc, x1, y1, NULL);
//    LineTo(hdc, x1 + w, y1);
//    DeleteObject(hPen);
//}
#endif
void DirectFillEllipse_pixel(HDC hdc, int x1, int y1, int w, int h, COLORREF color) {
    int cx = x1 + w / 2;
    int cy = y1 + h / 2;
    for (int y = y1; y < y1 + h; ++y) {
        double uy = ((double)y - cy) * 2 / h; // 単位円のy座標
        int ex = (int)(sqrt(1 - uy * uy) * w / 2); // 楕円のx座標
        for (int x = cx - ex; x <= cx + ex; ++x) {
            SetPixel(hdc, x, y, color);
        }
    }
}
#ifdef USE_FUNC_TABLE
int tab_orthogonal_dim(int x, int rx, int ry);
void DirectFillEllipse(HDC hdc, int x1, int y1, int w, int h, COLORREF color) {
    int cx = x1 + w / 2;
    int cy = y1 + h / 2;
    for (int y = y1; y < y1 + h; ++y) {
        int ex = tab_orthogonal_dim(y - cy, h / 2, w / 2); // 楕円のx座標
        DrawHLine(hdc, cx - ex, y, ex * 2, color);
    }
}
#else
void DirectFillEllipse(HDC hdc, int x1, int y1, int w, int h, COLORREF color) {
    int cx = x1 + w / 2;
    int cy = y1 + h / 2;
    for (int y = y1; y < y1 + h; ++y) {
        double uy = ((double)y - cy) * 2 / h; // 単位円のy座標
        int ex = (int)(sqrt(1 - uy * uy) * w / 2); // 楕円のx座標
        DrawHLine(hdc, cx - ex, y, ex * 2, color);
    }
}
#endif
void DirectFillRoundedRectangle_pixel(HDC hdc, int x1, int y1, int w, int h, int rx, int ry, COLORREF color) {
    int cx = x1 + rx;
    int cy = y1 + ry;
    for (int y = y1; y < y1 + ry; ++y) {
        double uy = ((double)y - cy) / ry; // 単位円のy座標
        int ex = (int)(sqrt(1 - uy * uy) * rx); // 楕円のx座標
        int y2 = y1 + h - (y - y1) - 1;
        for (int x = cx - ex; x <= x1 + w - rx + ex; ++x) {
            SetPixel(hdc, x, y, color);
            SetPixel(hdc, x, y2, color);
        }
    }
    for (int y = y1 + ry; y < y1 + h - ry; ++y) {
        for (int x = x1; x < x1 + w; ++x) {
            SetPixel(hdc, x, y, color);
        }
    }
}
#ifdef USE_FUNC_TABLE
void DirectFillRoundedRectangle(HDC hdc, int x1, int y1, int w, int h, int rx, int ry, COLORREF color) {
    int cx = x1 + rx;
    int cy = y1 + ry;
    for (int y = y1; y < y1 + ry; ++y) {
        int ex = tab_orthogonal_dim(y - cy, ry, rx); // 楕円のx座標
        int y2 = y1 + h - (y - y1) - 1;
        DrawHLine(hdc, cx - ex, y, x1 + ex * 2 + w - rx - cx, color);
        DrawHLine(hdc, cx - ex, y2, x1 + ex * 2 + w - rx - cx, color);
    }
    for (int y = y1 + ry; y < y1 + h - ry; ++y) {
        DrawHLine(hdc, x1, y, w, color);
    }
}
#else
void DirectFillRoundedRectangle(HDC hdc, int x1, int y1, int w, int h, int rx, int ry, COLORREF color) {
    int cx = x1 + rx;
    int cy = y1 + ry;
    for (int y = y1; y < y1 + ry; ++y) {
        double uy = ((double)y - cy) / ry; // 単位円のy座標
        int ex = (int)(sqrt(1 - uy * uy) * rx); // 楕円のx座標
        int y2 = y1 + h - (y - y1) - 1;
        DrawHLine(hdc, cx - ex, y, x1 + ex * 2 + w - rx - cx, color);
        DrawHLine(hdc, cx - ex, y2, x1 + ex * 2 + w - rx - cx, color);
    }
    for (int y = y1 + ry; y < y1 + h - ry; ++y) {
        DrawHLine(hdc, x1, y, w, color);
    }
}
#endif
void DirectFillQuarterPie1(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, int mir_x, int mir_y, COLORREF color) {
    // 扇型の左上1/4の領域の中の一つの塗りつぶした扇型を描画する
    int cx = x1 + w / 2;
    int cy = y1 + h / 2;
    int rx = w / 2;
    int ry = h / 2;
    if (sweep_angle < 0) {
        sweep_angle += 360; // 負の角度を正に変換
    }
    if (start_angle < 0) {
        start_angle += 360; // 負の角度を正に変換
    }
    int sa = start_angle;
    int ea = start_angle + sweep_angle;
    double sy = -ry * sin(deg_to_rad(start_angle));
    double ey = -ry * sin(deg_to_rad(start_angle + sweep_angle));
    if (sa >= 90 && sa <= 180 && ea >= 90 && ea <= 180) {
        // 上の部分の左側はは楕円と同じ・右側は三角形を描画
        double sx = rx * cos(deg_to_rad(start_angle));
        double ex = rx * cos(deg_to_rad(start_angle + sweep_angle));
        int elps_ey = (int)(cy + ey); // 弧の終了位置の楕円のy座標
        int elps_ex = (int)(cx + ex); // 弧の終了位置の楕円のx座標
        for (int y = y1; y < elps_ey; ++y) {
            double uy = ((double)y - cy) / ry; // 単位円のy座標
            int elps_x = (int)(sqrt(1 - uy * uy) * rx); // 楕円のx座標
            for (int x = cx - elps_x; x <= cx - (sx / sy) * (cy - y); ++x) {
                //SetPixel(hdc, x, y, color);
                SetPixel(hdc, cx + (x - cx) * mir_x, cy + (y - cy) * mir_y, color);
            }
        }
        // 下の部分は三角形を描画
        for (int y = elps_ey; y <= cy; ++y) {
            for (int x = cx - (ex / ey) * (cy - y); x <= cx - (sx / sy) * (cy - y); ++x) {
                //SetPixel(hdc, x, y, color);
                SetPixel(hdc, cx + (x - cx) * mir_x, cy + (y - cy) * mir_y, color);
            }
        }
    }
}
void DirectFillQuarterPie(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, int mir_x, int mir_y, COLORREF color) {
    // 扇型の左上1/4の領域を塗りつぶす
    // sa = start_angle, ea = start_angle + sweep_angle
    // (0 <= sa < 360, 0 < ea - sa <= 360)
    // 弧の開始点の単位円の周上の y 座標 sy
    // 弧の終了点の単位円の周上の y 座標 ey
    // (1) 楕円の左上が片方の端だけ含む場合
    //   (1-1) 終了点だけを含む場合: 90 <= ea - 360 <= 180 <= sa
    //     -1 <= ey <= 0
    //     - 1 <= uy <= ey のとき : 楕円と同じ領域に線を描く
    //     ey <= uy <= 0 のとき : ey を通る半径と y 軸の間に線を描く
    //   (1-2) 開始点だけを含む場合: 90 <= sa <= 180 <= ea
    //     -1 <= sy <= 0
    //     (1-1) の y = x に関して対称の図形を描く
    // (2) 楕円の左上が両端を含む場合
    //   (2-1) 終了点→開始点の順の場合: 90 <= ea - 360 < sa <= 180
    //     -1 <= ey < sy <= 0
    //     (1-1) と (1-2) の領域を描画する
    //   (2-2) 開始点→終了点の順の場合: 90 <= sa < ea <= 180
    //     -1 <= sy < ey <= 0
    //     -1 <= uy <= sy のとき: 何もしない
    //     sy <= uy <= ey のとき: 楕円の端から sy を通る半径の間に線を描く
    //     ey <= uy <= 0 のとき: ey を通る半径と sy を通る半径の間に線を描く
    // (3) 楕円の左上が両端とも含まないの場合
    //   左上を除いたものが
    //   (3-1) 終了点→開始点の順の場合: 180 <= ea - 360 < sa <= 450
    //     楕円と同じ領域を塗りつぶす
    //   (3-2) 開始点→終了点の順の場合: 180 <= sa < ea <= 450
    //     何もしない
    int cx = x1 + w / 2;
    int cy = y1 + h / 2;
    int rx = w / 2;
    int ry = h / 2;
    double sy = -ry * sin(start_angle * 3.14159 / 180);
    double ey = -ry * sin((start_angle + sweep_angle) * 3.14159 / 180);
    if (sweep_angle < 0) {
        sweep_angle += 360; // 負の角度を正に変換
    }
    if (start_angle < 0) {
        start_angle += 360; // 負の角度を正に変換
    }
    int sa = start_angle;
    int ea = start_angle + sweep_angle;
    if (!(sa > 90 && sa < 180) && ea >= 90 && ea <= 180) {
        // (1-1) 終了点だけを含む場合
        DirectFillQuarterPie1(hdc, x1, y1, w, h, 90, ea - 90, mir_x, mir_y, color);
    }
    else if (sa >= 90 && sa <= 180 && !(ea > 90 && ea < 180)) {
        // (1-2) 開始点だけを含む場合
        DirectFillQuarterPie1(hdc, x1, y1, w, h, sa, 180 - sa, mir_x, mir_y, color);
    }
    else if (ea - 360 >= 90 && ea - 360 < sa && sa <= 180) {
        // (2-1) 終了点→開始点の順の場合
        DirectFillQuarterPie1(hdc, x1, y1, w, h, 90, ea - 450, mir_x, mir_y, color);
        DirectFillQuarterPie1(hdc, x1, y1, w, h, sa, 180 - sa, mir_x, mir_y, color);
    }
    else if (sa >= 90 && ea <= 180) {
        // (2-2) 開始点→終了点の順の場合
        DirectFillQuarterPie1(hdc, x1, y1, w, h, sa, ea - sa, mir_x, mir_y, color);
    }
    else if (sa >= 180 && sa < 450 && ea - 360 >= 180 && ea - 360 < 450) {
        // (3-1) 終了点→開始点の順の場合
        DirectFillQuarterPie1(hdc, x1, y1, w, h, 90, 90, mir_x, mir_y, color);
    }
    else if (sa >= 180 && sa < 450 && ea >= 180 && ea < 450) {
        // (3-2) 開始点→終了点の順の場合
        // 何もしない
    }
}
void DirectFillPie(HDC hdc, int x1, int y1, int w, int h, int start_angle, int sweep_angle, COLORREF color) {
    // 扇型を塗りつぶす
    // 扇型の左上1/4の領域を塗りつぶす
    DirectFillQuarterPie(hdc, x1, y1, w, h, start_angle, sweep_angle, 1, 1, color);
}
void DirectFillWTriangle(HDC hdc, int x1, int y1, int x2, int y2, int w, COLORREF color) {
    // 塗りつぶし三角形を描く
    //printf("DirectFillWTriangle: (%d, %d), (%d, %d), w=%d\n", x1, y1, x2, y2, w);
    if (y1 != y2) {
        bool down = y1 < y2;
        int dy = down ? 1 : -1;
        for (int y = y1; down ? y < y2 : y > y2; y += dy) {
            bool right = w > 0;
            int dx = right ? 1 : -1;
            int sx = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            int ex = x1 + (x2 - x1 + w) * (y - y1) / (y2 - y1);
            for (int x = sx; right ? x < ex : x > ex; x += dx) {
                SetPixel(hdc, x, y, color);
            }
        }
    }
}
int get_coordinate(int x1, int y1, int x2, int y2, int y) {
    // 直線の方程式からx座標を計算
    if (y1 == y2) return x1; // 水平線の場合
    return x1 + (x2 - x1) * (y - y1) / (y2 - y1);
}
void DirectFillTrapezoid_pixel(HDC hdc, int x1, int y1, int x2, int y2, int w1, int w2, COLORREF color) {
    // 塗りつぶし台形を描く
    // 台形の上底 (x1, y1) から下底 (x2, y2) までの線分と、上底の幅 w1, 下底の幅 w2 を持つ
    int x1_left = w1 > 0 ? x1 : x1 + w1;
    int x1_right = w1 > 0 ? x1 + w1 : x1;
    int x2_left = w2 > 0 ? x2 : x2 + w2;
    int x2_right = w2 > 0 ? x2 + w2 : x2;
    for (int y = y1; y <= y2; ++y) {
        int left_x = get_coordinate(x1_left, y1, x2_left, y2, y);
        int right_x = get_coordinate(x1_right, y1, x2_right, y2, y);
        for (int x = left_x; x <= right_x; ++x) {
            SetPixel(hdc, x, y, color);
        }
    }
}
void DirectFillTrapezoid(HDC hdc, int x1, int y1, int x2, int y2, int w1, int w2, COLORREF color) {
    // 塗りつぶし台形を描く
    // 台形の上底 (x1, y1) から下底 (x2, y2) までの線分と、上底の幅 w1, 下底の幅 w2 を持つ
    int x1_left = w1 > 0 ? x1 : x1 + w1;
    int x1_right = w1 > 0 ? x1 + w1 : x1;
    int x2_left = w2 > 0 ? x2 : x2 + w2;
    int x2_right = w2 > 0 ? x2 + w2 : x2;
    for (int y = y1; y <= y2; ++y) {
        int left_x = get_coordinate(x1_left, y1, x2_left, y2, y);
        int right_x = get_coordinate(x1_right, y1, x2_right, y2, y);
        DrawHLine(hdc, left_x, y, right_x - left_x, color);
    }
}
void DirectFillTriangle(HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color) {
    // 塗りつぶし三角形を描く
    // 三角形の頂点 (x1, y1), (x2, y2), (x3, y3) を持つ
    // まず、y座標でソートして、y1 <= y2 <= y3 とする
    if (y1 > y2) std::swap(x1, x2), std::swap(y1, y2);
    if (y1 > y3) std::swap(x1, x3), std::swap(y1, y3);
    if (y2 > y3) std::swap(x2, x3), std::swap(y2, y3);

    // y2に対応する辺上のx座標を計算
    int x2_opposite = get_coordinate(x1, y1, x3, y3, y2);

    // y1からy2までの範囲で塗りつぶし
    DirectFillTrapezoid(hdc, x1, y1, x2, y2, 0, x2_opposite - x2, color);
    // y2からy3までの範囲で塗りつぶし
    DirectFillTrapezoid(hdc, x2, y2, x3, y3, x2_opposite - x2, 0, color);
}
#endif

void draw_cmd(HDC hdc, DrawCommand & cmd);

void paint_cmd(HWND hwnd) {
#ifndef USE_X68000
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    std::lock_guard<std::mutex> lock(g_queueMutex);
    while (!g_drawQueue.empty()) {
        DrawCommand& cmd = g_drawQueue.front();
        draw_cmd(hdc, cmd);
        g_drawQueue.pop();
    }
    EndPaint(hwnd, &ps);
#endif
}

void draw_cmd(HDC hdc, DrawCommand & cmd) {
#ifdef PIXEL_DRAW_MODE // Windowsの機能を使わず直接描画する場合(SetPixelを使う)
    switch (cmd.type) {
    case DrawType::Clear:
        //hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        //FillRect(hdc, &ps.rcPaint, hBrush);
        //SelectObject(hdc, hOldBrush);
        break;
    case DrawType::Line:
        DirectDrawLine(hdc, cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.color);
        break;
    case DrawType::LineTo:
        DirectDrawLineTo(hdc, cmd.x2, cmd.y2, cmd.color);
        break;
    case DrawType::LineInit:
        DirectDrawLineInit();
        break;
    case DrawType::String:
        //DrawTextOutW(hdc, cmd.x1, cmd.y1, cmd.text);
        break;
    case DrawType::Rect:
        DirectDrawRectangle(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.color);
        break;
    case DrawType::Ellipse:
        DirectDrawEllipse(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.color);
        break;
    case DrawType::Arc:
        DirectDrawArc(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.start_angle, cmd.sweep_angle, cmd.color);
        break;
    case DrawType::Pie:
        DirectDrawPie(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.start_angle, cmd.sweep_angle, cmd.color);
        break;
    case DrawType::RoundRect:
        DirectDrawRoundedRectangle(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.rx, cmd.ry, cmd.color);
        break;
    case DrawType::FillRect:
        DirectFillRectangle(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.color);
        break;
    case DrawType::FillEllipse:
        DirectFillEllipse(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.color);
        break;
    case DrawType::FillPie:
    {
        DirectFillPie(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.start_angle, cmd.sweep_angle, cmd.color);
        //int cx = cmd.x1 + cmd.w / 2;
        //int cy = cmd.y1 + cmd.h / 2;
        //for (int y = cmd.y1; y < cmd.y1 + cmd.h; ++y) {
        //    double uy = ((double)y - cy) * 2 / cmd.h; // 単位円のy座標
        //    int ex = (int)(sqrt(1 - uy * uy) * cmd.w / 2); // 楕円のx座標
        //    for (int x = cx - ex; x <= cx + ex; ++x) {
        //        double ux = ((double)x - cx) * 2 / cmd.w; // 単位円のx座標
        //        // (ux, uy) が cmd.start_angle から cmd.sweep_angle の範囲内にあるかチェック
        //        double angle = atan2(-uy, ux) * 180 / 3.14159; // 度に変換
        //        if (angle < 0) angle += 360; // 正の角度に変換
        //        if (angle >= cmd.start_angle && angle <= cmd.start_angle + cmd.sweep_angle) {
        //            SetPixel(hdc, x, y, cmd.color);
        //        }
        //    }
        //}
        break;
    }
    case DrawType::FillRoundRect:
        DirectFillRoundedRectangle(hdc, cmd.x1, cmd.y1, cmd.w, cmd.h, cmd.rx, cmd.ry, cmd.color);
        break;
    case DrawType::FillWTriangle:
        DirectFillWTriangle(hdc, cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.w, cmd.color);
        break;
    case DrawType::FillTriangle:
        DirectFillTriangle(hdc, cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.x3, cmd.y3, cmd.color);
        break;
    case DrawType::FillTrapezoid:
        DirectFillTrapezoid(hdc, cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.w, cmd.w2, cmd.color);
        break;
    case DrawType::Circle:
    {
        //HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        //int x1 = cmd.x1 - cmd.r;
        //int x2 = cmd.x1 + cmd.r * cmd.hbyw / 256;
        //int y1 = cmd.y1 - cmd.r;
        //int y2 = cmd.y1 + cmd.r * cmd.hbyw / 256;
        //if (cmd.start_angle == 0 && cmd.end_angle == 360) {
        //    Ellipse(hdc, x1, y1, x2, y2);
        //}
        //else if (cmd.start_angle <= 0 && cmd.end_angle < 0) {
        //    Pie(hdc, x1, y1, x2, y2,
        //        cmd.x1 + (int)(cmd.r * cos((cmd.start_angle + 360) * 3.14159 / 180)),
        //        cmd.y1 - (int)(cmd.r * sin((cmd.start_angle + 360) * 3.14159 / 180)),
        //        cmd.x1 + (int)(cmd.r * cos((cmd.end_angle + 360) * 3.14159 / 180)),
        //        cmd.y1 - (int)(cmd.r * sin((cmd.end_angle + 360) * 3.14159 / 180)));
        //}
        //else {
        //    Arc(hdc, x1, y1, x2, y2,
        //        cmd.x1 + (int)(cmd.r * cos((cmd.start_angle + 360) * 3.14159 / 180)),
        //        cmd.y1 - (int)(cmd.r * sin((cmd.start_angle + 360) * 3.14159 / 180)),
        //        cmd.x1 + (int)(cmd.r * cos((cmd.end_angle + 360) * 3.14159 / 180)),
        //        cmd.y1 - (int)(cmd.r * sin((cmd.end_angle + 360) * 3.14159 / 180)));
        //}
        //SelectObject(hdc, hOldBrush);
        break;
    }
    case DrawType::Test:
    {
        // テスト用
        for (int a = 0; a < 360; a += 10) {
            int r = 100;
            int color = 5;
            int x = cmd.x1 + (int)(r * cos(a * 3.14159 / 180));
            int y = cmd.y1 - (int)(r * sin(a * 3.14159 / 180));
            DirectDrawLine(hdc, cmd.x1, cmd.y1, x, y, color);
        }
        break;
    }
    case DrawType::Paint:
        // 指定した色の領域を塗りつぶす
        //Paint(hdc, cmd.x1, cmd.y1, cmd.color);
        break;
    case DrawType::Pset:
        SetPixel(hdc, cmd.x1, cmd.y1, cmd.color);
        break;
    case DrawType::Point:
    {
        COLORREF color = GetPixel(hdc, cmd.x1, cmd.y1);
        cmd.color = color; // 取得した色をcmdに設定
        break;
    }
    case DrawType::Symbol:
        // Windowsでは特に何もしない
        break;
    case DrawType::Apage:
        // Windowsでは特に何もしない
        break;
    case DrawType::Vpage:
        // Windowsでは特に何もしない
        break;
    case DrawType::Wipe:
        // 画面全体を背景色で塗りつぶす(未実装)
        break;
    case DrawType::Screen:
        // Windowsでは特に何もしない
        break;
    case DrawType::Window:
        // リージョンを使用してクリッピング
        //HRGN hRgn = CreateRectRgn(cmd.x1, cmd.y1, cmd.x2, cmd.y2);
        //SelectClipRgn(hdc, hRgn);
        // 座標を平行移動(未実装)
        break;
    }
#else
    switch (cmd.type) {
    case DrawType::Clear:
        hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        FillRect(hdc, &ps.rcPaint, hBrush);
        SelectObject(hdc, hOldBrush);
        break;
    case DrawType::Line:
        MoveToEx(hdc, cmd.x1, cmd.y1, nullptr);
        LineTo(hdc, cmd.x2, cmd.y2);
        break;
    case DrawType::LineTo:
        if (draw_x < 0) {
            MoveToEx(hdc, cmd.x2, cmd.y2, nullptr);
        }
        else {
            LineTo(hdc, cmd.x2, cmd.y2);
        }
        draw_x = cmd.x2; // 次のLineToのために座標を保存
        draw_y = cmd.y2;
        break;
    case DrawType::LineInit:
        draw_x = -1; // 初期化
        break;
    case DrawType::String:
        DrawTextOutW(hdc, cmd.x1, cmd.y1, cmd.text);
        break;
    case DrawType::Rect:
        hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, cmd.x1, cmd.y1, cmd.x1 + cmd.w, cmd.y1 + cmd.h);
        SelectObject(hdc, hOldBrush);
        break;
    case DrawType::Ellipse:
        hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, cmd.x1, cmd.y1, cmd.x1 + cmd.w, cmd.y1 + cmd.h);
        SelectObject(hdc, hOldBrush);
        break;
    case DrawType::Arc:
    {
        hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        int cx = cmd.x1 + cmd.w / 2;
        int cy = cmd.y1 + cmd.h / 2;
        Arc(hdc, cmd.x1, cmd.y1, cmd.x1 + cmd.w, cmd.y1 + cmd.h,
            cx + (int)(cmd.w / 2 * cos(cmd.start_angle * 3.14159 / 180)),
            cy - (int)(cmd.h / 2 * sin(cmd.start_angle * 3.14159 / 180)),
            cx + (int)(cmd.w / 2 * cos((cmd.start_angle + cmd.sweep_angle) * 3.14159 / 180)),
            cy - (int)(cmd.h / 2 * sin((cmd.start_angle + cmd.sweep_angle) * 3.14159 / 180)));
        SelectObject(hdc, hOldBrush);
        break;
    }
    case DrawType::Pie:
    {
        hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        int cx = cmd.x1 + cmd.w / 2;
        int cy = cmd.y1 + cmd.h / 2;
        Pie(hdc, cmd.x1, cmd.y1, cmd.x1 + cmd.w, cmd.y1 + cmd.h,
            cx + (int)(cmd.w / 2 * cos(cmd.start_angle * 3.14159 / 180)),
            cy - (int)(cmd.h / 2 * sin(cmd.start_angle * 3.14159 / 180)),
            cx + (int)(cmd.w / 2 * cos((cmd.start_angle + cmd.sweep_angle) * 3.14159 / 180)),
            cy - (int)(cmd.h / 2 * sin((cmd.start_angle + cmd.sweep_angle) * 3.14159 / 180)));
        SelectObject(hdc, hOldBrush);
        break;
    }
    case DrawType::RoundRect:
        hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, cmd.x1, cmd.y1, cmd.x1 + cmd.w, cmd.y1 + cmd.h, cmd.rx, cmd.ry);
        SelectObject(hdc, hOldBrush);
        break;
    case DrawType::FillRect:
        hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        Rectangle(hdc, cmd.x1, cmd.y1, cmd.x1 + cmd.w, cmd.y1 + cmd.h);
        SelectObject(hdc, hOldBrush);
        break;
    case DrawType::FillEllipse:
        hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        Ellipse(hdc, cmd.x1, cmd.y1, cmd.x1 + cmd.w, cmd.y1 + cmd.h);
        SelectObject(hdc, hOldBrush);
        break;
    case DrawType::FillPie:
    {
        hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        int cx = cmd.x1 + cmd.w / 2;
        int cy = cmd.y1 + cmd.h / 2;
        Pie(hdc, cmd.x1, cmd.y1, cmd.x1 + cmd.w, cmd.y1 + cmd.h,
            cx + (int)(cmd.w / 2 * cos(cmd.start_angle * 3.14159 / 180)),
            cy - (int)(cmd.h / 2 * sin(cmd.start_angle * 3.14159 / 180)),
            cx + (int)(cmd.w / 2 * cos((cmd.start_angle + cmd.sweep_angle) * 3.14159 / 180)),
            cy - (int)(cmd.h / 2 * sin((cmd.start_angle + cmd.sweep_angle) * 3.14159 / 180)));
        SelectObject(hdc, hOldBrush);
        break;
    }
    case DrawType::FillRoundRect:
        hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        RoundRect(hdc, cmd.x1, cmd.y1, cmd.x1 + cmd.w, cmd.y1 + cmd.h, cmd.rx, cmd.ry);
        SelectObject(hdc, hOldBrush);
        break;
    case DrawType::Circle:
    {
        hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        int x1 = cmd.x1 - cmd.r;
        int x2 = cmd.x1 + cmd.r * cmd.hbyw / 256;
        int y1 = cmd.y1 - cmd.r;
        int y2 = cmd.y1 + cmd.r * cmd.hbyw / 256;
        if (cmd.start_angle == 0 && cmd.end_angle == 360) {
            Ellipse(hdc, x1, y1, x2, y2);
        }
        else if (cmd.start_angle <= 0 && cmd.end_angle < 0) {
            Pie(hdc, x1, y1, x2, y2,
                cmd.x1 + (int)(cmd.r * cos((cmd.start_angle + 360) * 3.14159 / 180)),
                cmd.y1 - (int)(cmd.r * sin((cmd.start_angle + 360) * 3.14159 / 180)),
                cmd.x1 + (int)(cmd.r * cos((cmd.end_angle + 360) * 3.14159 / 180)),
                cmd.y1 - (int)(cmd.r * sin((cmd.end_angle + 360) * 3.14159 / 180)));
        }
        else {
            Arc(hdc, x1, y1, x2, y2,
                cmd.x1 + (int)(cmd.r * cos((cmd.start_angle + 360) * 3.14159 / 180)),
                cmd.y1 - (int)(cmd.r * sin((cmd.start_angle + 360) * 3.14159 / 180)),
                cmd.x1 + (int)(cmd.r * cos((cmd.end_angle + 360) * 3.14159 / 180)),
                cmd.y1 - (int)(cmd.r * sin((cmd.end_angle + 360) * 3.14159 / 180)));
        }
        SelectObject(hdc, hOldBrush);
        break;
    }
    case DrawType::Paint:
        // 指定した色の領域を塗りつぶす
        Paint(hdc, cmd.x1, cmd.y1, cmd.color);
        break;
    case DrawType::Pset:
        SetPixel(hdc, cmd.x1, cmd.y1, cmd.color);
        break;
    case DrawType::Point:
    {
        COLORREF color = GetPixel(hdc, cmd.x1, cmd.y1);
        cmd.color = color; // 取得した色をcmdに設定
        break;
    }
    case DrawType::Symbol:
        // Windowsでは特に何もしない
        break;
    case DrawType::Apage:
        // Windowsでは特に何もしない
        break;
    case DrawType::Vpage:
        // Windowsでは特に何もしない
        break;
    case DrawType::Wipe:
        // 画面全体を背景色で塗りつぶす(未実装)
        break;
    case DrawType::Screen:
        // Windowsでは特に何もしない
        break;
    case DrawType::Window:
        // リージョンを使用してクリッピング
        HRGN hRgn = CreateRectRgn(cmd.x1, cmd.y1, cmd.x2, cmd.y2);
        SelectClipRgn(hdc, hRgn);
        // 座標を平行移動(未実装)
        break;
    }
#endif
}

int16_t sp_x = 0;
int16_t sp_y = 0;
int16_t sp_dx = 0;
int16_t sp_dy = 0;
int16_t sp_length = 0;
int16_t sp_color = 0;

int16_t SetPixelLine_cmd(int16_t hdc_, int16_t x, int16_t y, int16_t color) {
    if (sp_length < 0) {
        sp_x = x;
        sp_y = y;
        sp_length = 0;
    }
    else if (sp_length == 0) {
        sp_dx = x - sp_x;
        sp_dy = y - sp_y;
        if (sp_dx == 0 && sp_dy == 1 || sp_dx == 1 && sp_dy == 0) {
            sp_length++;
            sp_color = color;
        }
        else {
            SetPixel_cmd(hdc_, sp_x, sp_y, sp_color);
            sp_length = -1;
        }
    }
    else if (x == sp_x + sp_dx * (sp_length + 1) && y == sp_y + sp_dy * (sp_length + 1)) {
        sp_length++;
    }
    else {
        //sp_length++;
        //if (sp_dx == 0 && sp_dy == 0) {
        //    // 始点と終点が同じ場合は点を描くだけ
        //    SetPixel_cmd(hdc_, sp_x, sp_y, sp_color);
        //}
        //else {
        //    // Bresenhamのアルゴリズムで線を描く
        //    int dx = abs(sp_dx);
        //    int dy = abs(sp_dy);
        //    int sx = sp_dx > 0 ? 1 : -1;
        //    int sy = sp_dy > 0 ? 1 : -1;
        //    int err = dx - dy;
        //    int x0 = sp_x;
        //    int y0 = sp_y;
        //    int x1 = x;
        //    int y1 = y;
        //    while (true) {
        //        SetPixel_cmd(hdc_, x0, y0, sp_color);
        //        if (x0 == x1 && y0 == y1) break;
        //        int err2 = err * 2;
        //        if (err2 > -dy) {
        //            err -= dy;
        //            x0 += sx;
        //        }
        //        if (err2 < dx) {
        //            err += dx;
        //            y0 += sy;
        //        }
        //    }
        //}
#ifndef USE_X68000
        HDC hdc = GetDC(hDrawWindow);
        HPEN hPen = CreatePen(PS_SOLID, 1, color);
        SelectObject(hdc, hPen);
        MoveToEx(hdc, sp_x, sp_y, NULL);
        LineTo(hdc, sp_x + sp_dx * sp_length, sp_y + sp_dy * sp_length);
        DeleteObject(hPen);
        ReleaseDC(hDrawWindow, hdc);
#endif
        // 次の線分のために初期化
        sp_length = -1;
    }
    return 0;
}

int16_t HLine_cmd(int hdc_, int x, int y, int w, int color) {
#ifndef USE_X68000
    HDC hdc = GetDC(hDrawWindow);
    DrawHLine(hdc, x, y, w, PALETTEINDEX(color));
    ReleaseDC(hDrawWindow, hdc);
#endif
    return 0;
}

bool set_draw_cmd(int token_count, char** tokens, ConOutput & co_cout, ConOutput & co_cerr) {
    const char* commands[] = {
        "exit", "clear", "drawline", "drawstring", "drawrectangle",
        "drawellipse", "drawarc", "drawpie", "drawroundedrectangle", "fillrectangle",
        "fillellipse", "fillpie", "fillroundedrectangle",
        "filltriangle","fillwtriangle", "filltrapezoid",
        "line", "box", "circle", "fill", "paint", "pset", "point", "symbol",
        "apage", "vpage", "wipe", "screen", "window",
        "lineto", "lineinit",
        "xline", "xbox", "xcircle", "xfill", "xpie", "xpaint", "xpset", "xpoint", "xclear",
        "test", "sine", "opposite"
    };
    const int command_count = sizeof(commands) / sizeof(commands[0]);
    CommandMatchResult result = matchCommand(tokens[0], commands, command_count);

    if (!result.matched) {
        co_cerr << "Unknown command: " << tokens[0] << co_endl;
        return true; // コマンドが不明な場合は処理を続ける
    }
    if (strcmp(result.command, "exit") == 0) {
        //co_cout << "Exiting command interpreter." << co_endl;
        return false;
    }

    DrawCommand cmd = {};
    if (strcmp(result.command, "clear") == 0 && token_count == 2) {
        cmd.type = DrawType::Clear;
        cmd.color = parseColor(tokens[1]);
    }
    else if (strcmp(result.command, "xclear") == 0 && token_count == 2) {
        cmd.type = DrawType::XClear;
        cmd.color = parseColor(tokens[1]);
    }
    else if (strcmp(result.command, "drawline") == 0 && token_count == 6) {
        cmd.type = DrawType::Line;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.x2 = atoi(tokens[3]);
        cmd.y2 = atoi(tokens[4]);
        cmd.color = parseColor(tokens[5]);
    }
    else if (strcmp(result.command, "drawstring") == 0 && token_count == 5) {
        cmd.type = DrawType::String;
        strcpy_s(cmd.text, sizeof(cmd.text), tokens[1]);
        cmd.x1 = atoi(tokens[2]);
        cmd.y1 = atoi(tokens[3]);
        cmd.color = parseColor(tokens[4]);
    }
    else if (strcmp(result.command, "drawrectangle") == 0 && token_count == 6) {
        cmd.type = DrawType::Rect;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.color = parseColor(tokens[5]);
    }
    else if (strcmp(result.command, "drawellipse") == 0 && token_count == 6) {
        cmd.type = DrawType::Ellipse;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.color = parseColor(tokens[5]);
    }
    else if (strcmp(result.command, "drawarc") == 0 && token_count == 8) {
        cmd.type = DrawType::Arc;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.start_angle = atoi(tokens[5]);
        cmd.sweep_angle = atoi(tokens[6]);
        cmd.color = parseColor(tokens[7]);
    }
    else if (strcmp(result.command, "drawpie") == 0 && token_count == 8) {
        cmd.type = DrawType::Pie;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.start_angle = atoi(tokens[5]);
        cmd.sweep_angle = atoi(tokens[6]);
        cmd.color = parseColor(tokens[7]);
    }
    else if (strcmp(result.command, "xpie") == 0 && token_count == 8) {
        cmd.type = DrawType::XPie;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.start_angle = atoi(tokens[5]);
        cmd.sweep_angle = atoi(tokens[6]);
        cmd.color = parseColor(tokens[7]);
    }
    else if (strcmp(result.command, "drawroundedrectangle") == 0 && token_count == 8) {
        cmd.type = DrawType::RoundRect;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.rx = atoi(tokens[5]);
        cmd.ry = atoi(tokens[6]);
        cmd.color = parseColor(tokens[7]);
    }
    else if (strcmp(result.command, "fillrectangle") == 0 && token_count == 6) {
        cmd.type = DrawType::FillRect;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.color = parseColor(tokens[5]);
    }
    else if (strcmp(result.command, "fillellipse") == 0 && token_count == 6) {
        cmd.type = DrawType::FillEllipse;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.color = parseColor(tokens[5]);
    }
    else if (strcmp(result.command, "fillpie") == 0 && token_count == 8) {
        cmd.type = DrawType::FillPie;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.start_angle = atoi(tokens[5]);
        cmd.sweep_angle = atoi(tokens[6]);
        cmd.color = parseColor(tokens[7]);
    }
    else if (strcmp(result.command, "fillroundedrectangle") == 0 && token_count == 8) {
        cmd.type = DrawType::FillRoundRect;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.rx = atoi(tokens[5]);
        cmd.ry = atoi(tokens[6]);
        cmd.color = parseColor(tokens[7]);
    }
    else if (strcmp(result.command, "fillwtriangle") == 0 && token_count == 7) {
        cmd.type = DrawType::FillWTriangle;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.x2 = atoi(tokens[3]);
        cmd.y2 = atoi(tokens[4]);
        cmd.w = atoi(tokens[5]);
        cmd.color = parseColor(tokens[6]);
    }
    else if (strcmp(result.command, "filltriangle") == 0 && token_count == 8) {
        cmd.type = DrawType::FillTriangle;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.x2 = atoi(tokens[3]);
        cmd.y2 = atoi(tokens[4]);
        cmd.x3 = atoi(tokens[5]);
        cmd.y3 = atoi(tokens[6]);
        cmd.color = parseColor(tokens[7]);
    }
    else if (strcmp(result.command, "filltrapezoid") == 0 && token_count == 8) {
        cmd.type = DrawType::FillTrapezoid;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.x2 = atoi(tokens[3]);
        cmd.y2 = atoi(tokens[4]);
        cmd.w = atoi(tokens[5]);
        cmd.w2 = atoi(tokens[6]);
        cmd.color = parseColor(tokens[7]);
    }
    else if (strcmp(result.command, "line") == 0 && token_count == 6) {
        cmd.type = DrawType::Line;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.x2 = atoi(tokens[3]);
        cmd.y2 = atoi(tokens[4]);
        cmd.color = parseColor(tokens[5]);
    }
    else if (strcmp(result.command, "xline") == 0 && token_count == 6) {
        cmd.type = DrawType::XLine;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.x2 = atoi(tokens[3]);
        cmd.y2 = atoi(tokens[4]);
        cmd.color = parseColor(tokens[5]);
    }
    else if (strcmp(result.command, "lineto") == 0 && token_count == 4) {
        cmd.type = DrawType::LineTo;
        cmd.x2 = atoi(tokens[1]);
        cmd.y2 = atoi(tokens[2]);
        cmd.color = parseColor(tokens[3]);
    }
    else if (strcmp(result.command, "lineinit") == 0 && token_count == 1) {
        cmd.type = DrawType::LineInit;
    }
    else if (strcmp(result.command, "box") == 0 && token_count == 6) {
        cmd.type = DrawType::Rect;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.color = parseColor(tokens[5]);
    }
    else if (strcmp(result.command, "xbox") == 0 && token_count == 6) {
        cmd.type = DrawType::XBox;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.color = parseColor(tokens[5]);
    }
    else if (strcmp(result.command, "fill") == 0 && token_count == 6) {
        cmd.type = DrawType::FillRect;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.color = parseColor(tokens[5]);
    }
    else if (strcmp(result.command, "xfill") == 0 && token_count == 6) {
        cmd.type = DrawType::XFill;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.w = atoi(tokens[3]);
        cmd.h = atoi(tokens[4]);
        cmd.color = parseColor(tokens[5]);
    }
    else if (strcmp(result.command, "circle") == 0 && token_count == 8) {
        cmd.type = DrawType::Circle;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.r = atoi(tokens[3]);
        cmd.color = parseColor(tokens[4]);
        cmd.start_angle = atoi(tokens[5]);
        cmd.end_angle = atoi(tokens[6]);
        cmd.hbyw = atoi(tokens[7]);
    }
    else if (strcmp(result.command, "xcircle") == 0 && token_count == 8) {
        cmd.type = DrawType::XCircle;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.r = atoi(tokens[3]);
        cmd.color = parseColor(tokens[4]);
        cmd.start_angle = atoi(tokens[5]);
        cmd.end_angle = atoi(tokens[6]);
        cmd.hbyw = atoi(tokens[7]);
    }
    else if (strcmp(result.command, "paint") == 0 && token_count == 4) {
        cmd.type = DrawType::Paint;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.color = parseColor(tokens[3]);
    }
    else if (strcmp(result.command, "xpaint") == 0 && token_count == 4) {
        cmd.type = DrawType::XPaint;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.color = parseColor(tokens[3]);
    }
    else if (strcmp(result.command, "pset") == 0 && token_count == 4) {
        cmd.type = DrawType::Pset;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.color = parseColor(tokens[3]);
    }
    else if (strcmp(result.command, "xpset") == 0 && token_count == 4) {
        cmd.type = DrawType::XPset;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.color = parseColor(tokens[3]);
    }
    else if (strcmp(result.command, "point") == 0 && token_count == 3) {
        cmd.type = DrawType::Point;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
    }
    else if (strcmp(result.command, "xpoint") == 0 && token_count == 3) {
        cmd.type = DrawType::XPoint;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
    }
    else if (strcmp(result.command, "symbol") == 0 && token_count == 9) {
        cmd.type = DrawType::Symbol;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        strcpy_s(cmd.text, sizeof(cmd.text), tokens[3]);
        cmd.mx = atoi(tokens[4]); // X方向の拡大率
        cmd.my = atoi(tokens[5]); // Y方向の拡大率
        cmd.font_size = atoi(tokens[6]); // フォントのタイプ 0-2
        cmd.color = parseColor(tokens[7]);
        cmd.rotation = atoi(tokens[8]); // 回転データ 0-3
    }
    else if (strcmp(result.command, "apage") == 0 && token_count == 2) {
        cmd.type = DrawType::Apage;
        cmd.page = atoi(tokens[1]);
    }
    else if (strcmp(result.command, "vpage") == 0 && token_count == 2) {
        cmd.type = DrawType::Vpage;
        cmd.page = atoi(tokens[1]);
    }
    else if (strcmp(result.command, "wipe") == 0 && token_count == 1) {
        cmd.type = DrawType::Wipe;
    }
    else if (strcmp(result.command, "screen") == 0 && token_count >= 2 && token_count <= 4) {
        cmd.type = DrawType::Screen;
        cmd.sc1 = atoi(tokens[1]);
        cmd.sc2 = atoi(tokens[2]);
        if (token_count > 3) {
            cmd.sc3 = atoi(tokens[3]);
        }
        else {
            cmd.sc3 = 0; // デフォルト値
        }
    }
    else if (strcmp(result.command, "window") == 0 && token_count == 5) {
        cmd.type = DrawType::Window;
        cmd.x1 = atoi(tokens[1]);
        cmd.y1 = atoi(tokens[2]);
        cmd.x2 = atoi(tokens[3]);
        cmd.y2 = atoi(tokens[4]);
    }
    else if (strcmp(result.command, "test") == 0 && token_count == 1) {
        cmd.type = DrawType::Test;
    }
    else if (strcmp(result.command, "sine") == 0 && token_count == 1) {
        // 正弦関数の表を作る
        // 単精度浮動小数点数の有効桁数約7桁なので小数点以下7桁まで表示
        for (int a = 0; a <= 450; a += 5) {
            double rad = deg_to_rad(a);
            double sine_value = sin(rad);
            //printf("Sine(%d) = %.7f\n", a, sine_value);
            printf("%.7f,\n", sine_value);
        }
    }
    else if (strcmp(result.command, "opposite") == 0 && token_count == 1) {
        // 円の反対側の座標の表を作る
        // 画面上の座標(最大1024)なので、半径は10000とする
        double r = 10000; // 円の半径
        for (int y = 0; y <= r; y += 100) {
            int x = (int)sqrt(r * r - y * y); // 円のx座標
            printf("%d,\n", x);
        }
    }
    //set_cmd(cmd);
    exec_command(cmd);
    return true;
}

#ifndef USE_X68000
#ifdef CONSOLE_IO
// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        std::lock_guard<std::mutex> lock(g_queueMutex);
        while (!g_drawQueue.empty()) {
            //const DrawCommand& cmd = g_drawQueue.front();
            DrawCommand& cmd = g_drawQueue.front();
            HPEN hPen = CreatePen(PS_SOLID, 1, cmd.color);
            HBRUSH hBrush = CreateSolidBrush(cmd.color);
            SelectObject(hdc, hPen);
            HBRUSH hOldBrush;

            draw_cmd(hdc, cmd);

            DeleteObject(hPen);
            DeleteObject(hBrush);
            g_drawQueue.pop();
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ウィンドウスレッド
void WindowThread() {
    const wchar_t CLASS_NAME[] = L"MyDrawWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    g_hWnd = CreateWindowEx(0, CLASS_NAME, L"Drawing Window",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, nullptr, nullptr, wc.hInstance, nullptr);

    ShowWindow(g_hWnd, SW_SHOW);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
#endif
#endif

CommandInterpreter command_interpreter;

#ifdef CONSOLE_IO
bool process_line(char* cline) {
    ConOutput co_cout(hOutput); // 出力オブジェクト
    ConOutput co_cerr(hOutput); // エラー出力オブジェクト

    std::string line(cline);
    return command_interpreter.process_command(line, co_cout, co_cerr);
}
#else
bool process_line(const std::wstring& wline) {
    ConOutput co_cout(hOutput); // 出力オブジェクト
    ConOutput co_cerr(hOutput); // エラー出力オブジェクト

    std::string line = WstringToString(wline);
    return command_interpreter.process_command(line, co_cout, co_cerr);
}
#endif

#ifdef CONSOLE_IO
void prompt() {
    // プロンプトを表示
    printf("> ");
}

// メイン関数
int main() {
#ifdef USE_X68000
    crtmod(4);  // X68000版では画面モードを設定
    gclear();    // 画面をクリア
    apage(0); // グラフィックページを設定
#else
    std::thread winThread(WindowThread);
#endif
    prompt();
    char line[256]; // 入力バッファ（適切なサイズを確保）
    while (fgets(line, sizeof(line), stdin)) {
        if (!process_line(line)) {
            break;
        }
        prompt();
    }

#ifndef USE_X68000
    winThread.join();
#endif
    return 0;
}
#endif

int orthogonal_dim_table[] = {
    10000,
    9999,
    9997,
    9995,
    9991,
    9987,
    9981,
    9975,
    9967,
    9959,
    9949,
    9939,
    9927,
    9915,
    9901,
    9886,
    9871,
    9854,
    9836,
    9817,
    9797,
    9777,
    9754,
    9731,
    9707,
    9682,
    9656,
    9628,
    9600,
    9570,
    9539,
    9507,
    9474,
    9439,
    9404,
    9367,
    9329,
    9290,
    9249,
    9208,
    9165,
    9120,
    9075,
    9028,
    8979,
    8930,
    8879,
    8826,
    8772,
    8717,
    8660,
    8601,
    8541,
    8479,
    8416,
    8351,
    8284,
    8216,
    8146,
    8074,
    8000,
    7924,
    7846,
    7765,
    7683,
    7599,
    7512,
    7423,
    7332,
    7238,
    7141,
    7042,
    6939,
    6834,
    6726,
    6614,
    6499,
    6380,
    6257,
    6131,
    6000,
    5864,
    5723,
    5577,
    5425,
    5267,
    5102,
    4930,
    4749,
    4559,
    4358,
    4146,
    3919,
    3675,
    3411,
    3122,
    2800,
    2431,
    1989,
    1410,
    0,
};

//int tab_opposite_edge(int x, int rx, int ry) {
//    // 半径 r の円の y 座標の表から、
//    // x 座標、x 方向の半径、y 方向の半径を
//    // 指定して y 座標を求める
//    int r = 10000;
//    int ux = abs(x) * r / rx; // 半径 r の円の x 座標
//    if (ux > r) {
//		// 入力が半径 r の円の範囲外なら 0 を返す
//		return 0;
//    }
//    int index = ux / 100;
//	return opposite_edge_table[index] * ry / r; // y 座標を求める
//}
int tab_orthogonal_dim(int x, int rx, int ry) {
    // 半径 r の円の y 座標の表から、
    // x 座標、x 方向の半径、y 方向の半径を
    // 指定して y 座標を求める
    long r = 10000;
    long ux = abs(x) * r / rx; // 半径 r の円の x 座標
    if (ux > r) {
        // 入力が半径 r の円の範囲外なら 0 を返す
        return 0;
    }
    int index = ux / 100;
    return orthogonal_dim_table[index] * ry / r; // y 座標を求める
}
