#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <utility>

namespace saildb {
namespace common {

bool startsWithUnicodeByteMark(const wchar_t* str, int sz);

std::wstring str2wstr(const std::string& str);

std::string wstr2str(const std::wstring& str);

std::string lpwstr2str(wchar_t* str, uint32_t codepage = 65001 /*CP_UTF8*/, uint32_t flags = 0);

std::string lsastr2str(wchar_t* buf, const uint16_t& length, uint32_t codepage = 65001 /*CP_UTF8*/, uint32_t flags = 0);

} // namespace common
} // namespace saildb
