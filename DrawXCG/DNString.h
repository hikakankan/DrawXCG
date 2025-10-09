#pragma once
#include <string>
#include <sstream>
#include <ranges>
#include <algorithm> // transform
#include <cctype>    // tolower

class DNString
{
	std::string str;
public:
	DNString()
	{
	}

	DNString(const char* str)
	{
		this->str = str;
	}

	DNString(const DNString& str)
	{
		this->str = str.str;
	}

	DNString(const std::string& str)
	{
		this->str = str;
	}

	DNString(double n)
	{
		std::ostringstream oss;
		oss << n;
		str = oss.str();
	}

	DNString(char c, int length)
	{
		str = std::string(c, length);
	}

	DNString Substring(int start, int length)
	{
		return DNString(str.substr(start, length));
	}

	DNString substr(int start, int length)
	{
		return DNString(str.substr(start, length));
	}

	DNString Substring(int start)
	{
		return DNString(str.substr(start));
	}

	int Length() const
	{
		return static_cast<int>(str.length());
	}

	char operator[](int index) const
	{
		return str[index];
	}

	int IndexOf(DNString s) const
	{
		return static_cast<int>(str.find_first_of(s.str));
	}

	bool Equals(DNString s) const
	{
		return str == s.str;
	}

	double ToDouble() const
	{
		return std::stod(str);
	}

	static DNString Concat(DNString x, DNString y)
	{
		return x.str + y.str;
	}

	DNString operator=(DNString s)
	{
		str = s.str;
		return *this;
	}

	DNString operator=(const char* s)
	{
		str = s;
		return *this;
	}

	DNString operator+=(DNString y)
	{
		str += y.str;
		return *this;
	}

	bool operator==(DNString y) const
	{
		return str == y.str;
	}

	bool operator!=(DNString y) const
	{
		return str != y.str;
	}

	const char* to_str()
	{
		return str.c_str();
	}

	const std::string WHITESPACE = " \n\r\t\f\v";

	size_t lfind_space()
	{
		return str.find_first_of(WHITESPACE);
	}

	std::string ltrim(const std::string& s)
	{
		size_t start = s.find_first_not_of(WHITESPACE);
		return (start == std::string::npos) ? "" : s.substr(start);
	}

	std::string LeftTrim()
	{
		return ltrim(str);
	}

	std::string rtrim(const std::string& s)
	{
		size_t end = s.find_last_not_of(WHITESPACE);
		return (end == std::string::npos) ? "" : s.substr(0, end + 1);
	}

	std::string RightTrim()
	{
		return rtrim(str);
	}

	std::string trim(const std::string& s) {
		return rtrim(ltrim(s));
	}

	std::string Trim()
	{
		return trim(str);
	}

	std::string ToLower(std::string src)
	{
		std::string text = src;
		std::transform(text.begin(), text.end(), text.begin(),
			[](unsigned char c) { return std::tolower(c); });
		return text;
	}

	std::string GetString()
	{
		return str;
	}

	int length()
	{
		return static_cast<int>(str.length());
	}

	int Length()
	{
		return static_cast<int>(str.length());
	}

	double ToDouble()
	{
		double val;
		val = std::stod(str);
		return val;
	}

	DNString remove_spaces() const {
		std::string result = str;
		result.erase(remove(result.begin(), result.end(), ' '), result.end());
		return result;
	}
	bool is_number_string() const {
		if (str.empty()) return false;
		for (char c : str) {
			if (!isdigit(c) && c != '.' && c != '-' && c != '+') {
				return false;
			}
		}
		return true;
	}
	bool is_varname_string() const {
		if (str.empty()) return false;
		if (!isalpha(str[0]) && str[0] != '_') return false; // 変数名はアルファベットまたはアンダースコアで始まる必要がある
		for (char c : str) {
			if (!isalnum(c) && c != '_') { // アルファベット、数字、アンダースコア以外は許可しない
				return false;
			}
		}
		return true;
	}
};
