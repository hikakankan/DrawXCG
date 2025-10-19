#ifndef USE_X68000
#include <conio.h>  // _kbhit, _getch
#endif

typedef struct {
    int x;
    int y;
    bool bl;
    bool br;
} MouseState;

#ifndef USE_X68000
void get_mouse_state(MouseState* state)
{
    POINT pt; // マウス座標を格納する構造体
    if (GetCursorPos(&pt)) {
        state->x = pt.x;
        state->y = pt.y;
    }
    else {
        state->x = -1;
        state->y = -1;
    }

    // マウスボタンの状態を取得
    state->bl = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    state->br = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
}
#else
// マウスの状態を取得する関数
void get_mouse_state(MouseState* state)
{
    int x, y, x2, y2, bl, br;

    mspos(&x, &y);
    msstat(&x2, &y2, &bl, &br);

    state->x = x;
    state->y = y;
    state->bl = bl != 0;
    state->br = br != 0;
}
#endif

#ifndef USE_X68000
bool window_or_mouse_init(HWND& hwnd) {
    // ウィンドウを初期化
    return true;
}
#else
typedef int HWND; // X68000ではウィンドウは不要なのでダミー定義
bool window_or_mouse_init(HWND& hwnd) {
    mouse_init();
    return true; // X68000ではウィンドウは不要なので常に成功
}
#endif

bool is_key_pressed() {
#ifndef USE_X68000
    return _kbhit();
#else
    int key = 0;
    keysns(&key);
    return (key & 0xff) != 0; // キーが押されているかどうかをチェック
#endif
}

int key_input() {
#ifndef USE_X68000
    if (_kbhit()) {
        return _getch(); // キー入力を取得
    }
    return 0; // 入力がない場合は0を返す
#else
    int key = 0;
    keyinp(&key); // IOCSコールでキー入力を取得
    return key & 0xff; // キーコードを返す
#endif
}

void mouse_recieve() {
    MouseState state;

    HWND hwnd = hDrawWindow; // 描画ウィンドウのハンドルを使用
    if (!window_or_mouse_init(hwnd)) {
        // ウィンドウの初期化に失敗した場合は終了
        return;
    }

    while (true) {
#ifndef USE_X68000
        MSG msg;
        // メッセージがあれば処理
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#endif

        get_mouse_state(&state);

        //std::stringstream ss;
        //ss << "Position: (" << state.x << ", " << state.y << ") ";
        //ss << "Buttons: Left: " << (state.bl ? "ON" : "OFF")
        //    << " | Right: " << (state.br ? "ON" : "OFF");

        //printf("\r%s  ", ss.str().c_str());
        //fflush(stdout);

        if (state.bl) {
            POINT pt;
            pt.x = state.x;
            pt.y = state.y;
#ifndef USE_X68000
            ScreenToClient(hwnd, &pt);
#endif
            if (pt.x < 10 && pt.y < 10) {
                // クリック領域外なら終了
                return;
            }
        }

#ifndef USE_X68000
        // CPU負荷を減らすために少し待つ
        Sleep(50);
#endif

#ifdef USE_X68000
        if (is_key_pressed()) {
            int key = key_input();
            //ss << " Key Input: " << std::hex << (key & 0xff);
            //printf("\r%s  ", ss.str().c_str());
            //fflush(stdout);
            if (key == 'q') {
                // 'q'キーが押されたら終了
                break;
            }
        }
#endif
    }
}
