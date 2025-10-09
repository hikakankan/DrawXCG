#pragma once
#include <string>
#include <map>
#include <optional>
#include "DNString.h"
#include <concepts>
#include <iostream>
#include <cmath>
#include "wintox.h"

#ifdef USE_X68000
#define CONSOLE_IO // コンソール入出力を使用
#endif

#define ID_INPUT 1001
#define ID_OUTPUT 1002
#define ID_SOURCE 1003
#define ID_DRAWWINDOW 1004

extern HWND hInput, hOutput;
extern HWND hSource;
extern HWND hDrawWindow;

#ifndef USE_X68000
// https://qiita.com/tsuchinokoman/items/869a30e02e6ddb5786b3 からコードを引用
std::wstring StringToWstring(const std::string& str);
std::string WstringToString(const std::wstring& wstr);
#endif

class ConOutputModifier {
    // コンソール風出力クラスの修飾子
public:
    enum Type {
		HEX, // 16進数
        ENDL, // 改行
        FLUSH // フラッシュ
    };
    Type type;
    ConOutputModifier(Type type) : type(type) {}
};

class ConOutput {
    // コンソール風出力クラス
private:
    HWND hOutput; // 出力先のウィンドウハンドル
	bool is_hex = false; // 16進数モードかどうか
    void print(const std::string& str) {
        // 出力先のウィンドウに文字列を追加
#ifdef CONSOLE_IO
        printf("%s", str.c_str());
#else
        std::wstring wstr = StringToWstring(str);
        int len = GetWindowTextLength(hOutput);
        SendMessage(hOutput, EM_SETSEL, len, len);
        SendMessage(hOutput, EM_REPLACESEL, 0, (LPARAM)wstr.c_str());
#endif
    }
public:
    ConOutput(HWND hOutput) {
        // コンストラクタで出力先のウィンドウハンドルを設定
        this->hOutput = hOutput;
    }
    ConOutput& operator<<(const std::string& str) {
        // 出力演算子オーバーロード
        print(str);
        return *this;
    }
    ConOutput& operator<<(double num) {
        // 出力演算子オーバーロード
        print(std::to_string(num));
        return *this;
    }
    ConOutput& operator<<(int num) {
        // 出力演算子オーバーロード
		if (is_hex) {
            std::stringstream ss;
            ss << std::hex << num;
            print(ss.str());
			is_hex = false; // 16進数モードをリセット
		}
        else {
            print(std::to_string(num));
        }
        return *this;
    }
#ifdef USE_X68000
    ConOutput& operator<<(int32_t num) {
        // 出力演算子オーバーロード
        if (is_hex) {
            std::stringstream ss;
            ss << std::hex << num;
            print(ss.str());
            is_hex = false; // 16進数モードをリセット
        }
        else {
            print(std::to_string(num));
        }
        return *this;
    }
#endif
    ConOutput& operator<<(int64_t num) {
        // 出力演算子オーバーロード
        if (is_hex) {
            std::stringstream ss;
            ss << std::hex << num;
            print(ss.str());
            is_hex = false; // 16進数モードをリセット
        }
        else {
            print(std::to_string(num));
        }
        return *this;
    }
    ConOutput& operator<<(ConOutputModifier mod) {
        // 出力演算子オーバーロード
        if (mod.type == ConOutputModifier::ENDL) {
            print("\r\n");
        }
        else if (mod.type == ConOutputModifier::FLUSH) {
            // フラッシュは特に何もしない
        }
        else if (mod.type == ConOutputModifier::HEX) {
            // 16進数モード
			is_hex = true;
		}
        return *this;
    }
};

extern ConOutputModifier co_endl;
extern ConOutputModifier co_flush;
extern ConOutputModifier co_hex;

const int MAX_INPUT_LENGTH = 100;

// 結果を表す構造体（optional の代わり）
struct CommandMatchResult {
    bool matched;       // 見つかったかどうか
    const char* command; // 見つかったコマンド名（見つからなければ nullptr）
};

// マッチング関数
CommandMatchResult matchCommand(const char* input, const char* const* commands, int command_count);

// 最大分割数
#define MAX_TOKENS 16

// split 関数
// line: 入力文字列（変更されます）
// delim: 区切り文字
// tokens: 結果のポインタ配列
// 戻り値: 分割されたトークン数
int split(char* line, char delim, char* tokens[], int max_tokens);

bool process_line(const std::wstring& wline);
