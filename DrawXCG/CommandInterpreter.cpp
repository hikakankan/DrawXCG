#include "CommandInterpreter.h"
#include "DrawXCG.h"
#include "draw.h"

bool CommandInterpreter::process_command(const std::string& sline, ConOutput& co_cout, ConOutput& co_cerr) {
    char line[256]; // 入力バッファ（適切なサイズを確保）
    std::string tsline = DNString(sline).Trim();
#ifdef USE_X68000
    strncpy(line, tsline.c_str(), sizeof(line));
#else
    strcpy_s(line, sizeof(line), tsline.c_str());
#endif
    line[strcspn(line, "\n")] = '\0'; // 改行文字を削除
    char* tokens[MAX_TOKENS];
    int token_count = split(line, ' ', tokens, MAX_TOKENS);
    if (token_count == 0) {
        return true; // 空行の場合は処理を続ける
    }
    return set_draw_cmd(token_count, tokens, co_cout, co_cerr);
}
