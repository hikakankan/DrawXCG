#pragma once
#include <string>

#include "draw.h"

#ifndef USE_X68000
WNDPROC oldEditProc = nullptr;
WNDPROC oldOutputProc = nullptr;
WNDPROC oldSourceProc = nullptr;
WNDPROC oldDrawProc = nullptr;

std::wstring prompt = L"> ";

std::wstring GetCurrentLine(HWND hwnd) {
    // テキスト全体を取得
    int len = GetWindowTextLength(hwnd);
    std::wstring buffer(len + 1, L'\0');
    GetWindowText(hwnd, &buffer[0], len + 1);

    // キャレット位置を取得
    DWORD start, end;
    SendMessage(hwnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);

    // start のある行の先頭と末尾を探す
    size_t lineStart = buffer.rfind(L'\n', start);
    if (lineStart == std::wstring::npos) lineStart = 0; else lineStart++;

    size_t lineEnd = buffer.find(L'\n', start);
    if (lineEnd == std::wstring::npos) lineEnd = buffer.size();

    // 一行全体を抽出
    std::wstring line = buffer.substr(lineStart, lineEnd - lineStart);
	return line;
}

// サブクラス用プロシージャ
LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        std::wstring line = GetCurrentLine(hwnd);

		int currentLine = (int)SendMessage(hwnd, EM_LINEFROMCHAR, (WPARAM)-1, 0);
        int lineCount = (int)SendMessage(hwnd, EM_GETLINECOUNT, 0, 0);
        if (currentLine < lineCount - 1) {
            // 次の行がある場合は、最後に移動する
            int len = GetWindowTextLength(hwnd);
            SendMessage(hwnd, EM_SETSEL, len, len);
        }
        else {
            // 最後の行の場合は、プロンプトを表示する
            int len = GetWindowTextLength(hwnd);
            SendMessage(hwnd, EM_SETSEL, len, len);
            SendMessage(hwnd, EM_REPLACESEL, 0, (LPARAM)(L"\r\n" + prompt).c_str());
        }

        // 行の処理
        line = line.substr(prompt.size()); // プロンプト部分を除去
        if (!process_line(line)) {
            // "exit" コマンドが入力された場合、アプリケーションを終了
            PostQuitMessage(0);
        }

        return 0; // エンターキーの既定動作（ビープ音など）を抑制
    }
    if (msg == WM_CHAR && wParam == VK_RETURN) {
        return 0; // エンターキーの既定動作（ビープ音など）を抑制
    }
    if (msg == WM_KEYDOWN && wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000))
    {
        // Ctrl+Aが押されたときの処理
        // 全体を選択
        SendMessage(hwnd, EM_SETSEL, 0, -1);
        return 0;
    }
    return CallWindowProc(oldEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK OutputProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000))
    {
        // Ctrl+Aが押されたときの処理
        // 全体を選択
        SendMessage(hwnd, EM_SETSEL, 0, -1);
        return 0;
    }
    return CallWindowProc(oldOutputProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK SourceProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000))
    {
        // Ctrl+Aが押されたときの処理
        // 全体を選択
        SendMessage(hwnd, EM_SETSEL, 0, -1);
        return 0;
    }
    return CallWindowProc(oldSourceProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK DrawProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE:
        return 0; // Size message handled
    case WM_PAINT:
		paint_cmd(hwnd); // 描画コマンドを実行
        return 0; // Paint message handled
    }
    return CallWindowProc(oldDrawProc, hwnd, msg, wParam, lParam);
}

RECT input_rect = { 10, 10, 310, 210 };
RECT output_rect = { 10, 210, 310, 410 };
RECT source_rect = { 10, 410, 310, 530 };
RECT draw_rect = { 320, 10, 840, 530 };
std::optional<RECT> last_sizing_rect = std::nullopt;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
    {
        // 入力用
        hInput = CreateWindow(L"EDIT", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL,
            input_rect.left, input_rect.top, input_rect.right - input_rect.left, input_rect.bottom - input_rect.top,
            hwnd, (HMENU)ID_INPUT, NULL, NULL);

        // 出力用（読み取り専用、複数行）
        hOutput = CreateWindow(L"EDIT", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            output_rect.left, output_rect.top, output_rect.right - output_rect.left, output_rect.bottom - output_rect.top,
            hwnd, (HMENU)ID_OUTPUT, NULL, NULL);

        // ソースコード用
        hSource = CreateWindow(L"EDIT", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL,
            source_rect.left, source_rect.top, source_rect.right - source_rect.left, source_rect.bottom - source_rect.top,
            hwnd, (HMENU)ID_SOURCE, NULL, NULL);

        // 描画用ウィンドウを作成
        hDrawWindow = CreateWindow(L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            draw_rect.left, draw_rect.top, draw_rect.right - draw_rect.left, draw_rect.bottom - draw_rect.top,
            hwnd, (HMENU)ID_DRAWWINDOW, NULL, NULL);

        SetWindowText(hInput, prompt.c_str());
        {
            int len = GetWindowTextLength(hInput);
            SendMessage(hInput, EM_SETSEL, len, len);
        }
        SetFocus(hInput);

        // サブクラス化
        oldEditProc = (WNDPROC)SetWindowLongPtr(hInput, GWLP_WNDPROC, (LONG_PTR)EditProc);
        oldOutputProc = (WNDPROC)SetWindowLongPtr(hOutput, GWLP_WNDPROC, (LONG_PTR)OutputProc);
        oldSourceProc = (WNDPROC)SetWindowLongPtr(hSource, GWLP_WNDPROC, (LONG_PTR)SourceProc);
        oldDrawProc = (WNDPROC)SetWindowLongPtr(hDrawWindow, GWLP_WNDPROC, (LONG_PTR)DrawProc);
        break;

    }
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_INPUT && HIWORD(wParam) == EN_CHANGE) {
            // Enterキーを検出するためにEN_CHANGEではなくWM_KEYDOWNを使う
        }
        break;

    case WM_KEYDOWN:
        break;

    case WM_SIZE:
        if (last_sizing_rect.has_value()) {
            // ウィンドウのサイズ変更時に、子ウィンドウの位置とサイズをメインウィンドウに合わせて調整
            RECT rect;
            GetClientRect(hwnd, &rect);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
			int last_width = last_sizing_rect->right - last_sizing_rect->left;
			int last_height = last_sizing_rect->bottom - last_sizing_rect->top;
            last_sizing_rect = rect; // 現在のサイズを保存
			// ソースコードウィンドウは下端に合わせてサイズを調整
            source_rect = { source_rect.left, source_rect.top, source_rect.right, source_rect.bottom + height - last_height };
            SetWindowPos(hSource, NULL, source_rect.left, source_rect.top, source_rect.right - source_rect.left, source_rect.bottom - source_rect.top, SWP_NOZORDER);
            // 描画ウィンドウは右端と下端に合わせてサイズを調整
            draw_rect = { draw_rect.left, draw_rect.top, draw_rect.right + width - last_width, draw_rect.bottom + height - last_height };
            SetWindowPos(hDrawWindow, NULL, draw_rect.left, draw_rect.top, draw_rect.right - draw_rect.left, draw_rect.bottom - draw_rect.top, SWP_NOZORDER);
        }
		return 0; // Size message handled

    case WM_SIZING:
        if (!last_sizing_rect.has_value()) {
			// 最初のサイズ変更時に、現在のクライアント領域のサイズを保存
            RECT rect;
            GetClientRect(hwnd, &rect); // 変更前のサイズ
			last_sizing_rect = rect; // 現在のサイズを保存
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"MyConsoleWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"GUI Console",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 868, 578,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
#endif
