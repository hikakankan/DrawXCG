#include "console.h"
//#include "form.h"
//#include "AsmInterpreter.h"
//#include "Program.h"
//#include "CommandInterpreter.h"
#ifdef USE_X68000
#include <cstring>
#endif

ConOutputModifier co_endl(ConOutputModifier::ENDL);
ConOutputModifier co_flush(ConOutputModifier::FLUSH);
ConOutputModifier co_hex(ConOutputModifier::HEX);

//#define USE_X68000

#ifndef USE_X68000
HWND hInput, hOutput;
HWND hSource;
HWND hDrawWindow;
#else
HWND hOutput;
HWND hDrawWindow;
#endif

// マッチング関数
CommandMatchResult matchCommand(const char* input, const char* const* commands, int command_count) {
    // 完全一致の確認
    for (int i = 0; i < command_count; ++i) {
        if (strcmp(commands[i], input) == 0) {
            CommandMatchResult result = { true, commands[i] };
            return result;
        }
    }

    // プレフィックス一致の確認
    const char* match = nullptr;
    int matchCount = 0;
    size_t input_len = strlen(input);

    for (int i = 0; i < command_count; ++i) {
        if (strncmp(commands[i], input, input_len) == 0) {
            match = commands[i];
            matchCount++;
        }
    }

    if (matchCount == 1) {
        CommandMatchResult result = { true, match };
        return result;
    }

    // 一意でない、または一致しない場合
    CommandMatchResult result = { false, nullptr };
    return result;
}

// split 関数
// line: 入力文字列（変更されます）
// delim: 区切り文字
// tokens: 結果のポインタ配列
// 戻り値: 分割されたトークン数
int split(char* line, char delim, char* tokens[], int max_tokens) {
    int count = 0;
    char* p = line;

    while (*p != '\0' && count < max_tokens) {
        // トークンの開始位置を記録
        tokens[count++] = p;

        // 区切り文字または終端まで進む
        while (*p != '\0' && *p != delim) {
            p++;
        }

        if (*p == delim) {
            // 区切り文字を終端に置き換え
            *p = '\0';
            p++; // 次のトークンの先頭へ
        }
    }

    return count;
}

#ifndef USE_X68000
// https://qiita.com/tsuchinokoman/items/869a30e02e6ddb5786b3 からコードを引用
std::wstring StringToWstring(const std::string& str)
{
    std::wstring ret;
    //一度目の呼び出しは文字列数を知るため
    auto result = MultiByteToWideChar(CP_UTF8,
        0,
        str.c_str(),//入力文字列
        (int)str.length(),
        nullptr,
        0);
    ret.resize(result);//確保する
    //二度目の呼び出しは変換
    result = MultiByteToWideChar(CP_UTF8,
        0,
        str.c_str(),//入力文字列
        (int)str.length(),
        ret.data(),
        (int)ret.size());
    return ret;
}

// https://qiita.com/tsuchinokoman/items/869a30e02e6ddb5786b3 からコードを引用
std::string WstringToString(const std::wstring& wstr)
{
    std::string ret;
    //一度目の呼び出しは文字列数を知るため
    auto result = WideCharToMultiByte(
        CP_ACP,
        0,
        wstr.c_str(),//入力文字列
        (int)wstr.length(),
        nullptr,
        0,
        nullptr,
        nullptr);
    ret.resize(result);//確保する
    //二度目の呼び出しは変換
    result = WideCharToMultiByte(
        CP_ACP,
        0,
        wstr.c_str(),//入力文字列
        (int)wstr.length(),
        ret.data(),
        (int)ret.size(),
        nullptr,
        nullptr);
    return ret;
}
#endif
