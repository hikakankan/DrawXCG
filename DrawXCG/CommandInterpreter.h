#pragma once
#include <string>
#include <map>
#include <cstring>
#include "console.h"

class CommandInterpreter
{
public:
    bool process_command(const std::string& sline, ConOutput& co_cout, ConOutput& co_cerr);
};
