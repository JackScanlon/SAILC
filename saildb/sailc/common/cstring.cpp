#include "cstring.hpp"
#include "constants.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stringapiset.h>

#include <cstring>
#include <codecvt>
#include <algorithm>

namespace common = saildb::common;

bool common::startsWithUnicodeByteMark(const wchar_t* str, int sz) {
  if (!IsTextUnicode(str, sz, NULL)) {
    return false;
  }

  return (*((wchar_t*)str) == saildb::constants::UNICODE_BYTE_ORDER_MARK);
}

std::wstring common::str2wstr(const std::string& str) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  std::wstring res = converter.from_bytes(str);
  res.erase(std::find(res.begin(), res.end(), L'\0'), res.end());

  return res;
}

std::string common::wstr2str(const std::wstring& str) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.to_bytes(str);
}

std::string common::lpwstr2str(wchar_t* str, uint32_t codepage /*= CP_UTF8*/, uint32_t flags /*= 0*/) {
  size_t len = WideCharToMultiByte(codepage, flags, str, -1, NULL, 0, nullptr, nullptr);
  if (len != 0) {
    std::string buf;
    buf.resize(len);

    if (WideCharToMultiByte(codepage, flags, str, -1, &buf[0], len, nullptr, nullptr) != 0) {
      return buf;
    }
  }

  return std::string();
}

std::string common::lsastr2str(wchar_t* buf, const uint16_t& length, uint32_t codepage /*= CP_UTF8*/, uint32_t flags /*= 0*/) {
  if (!buf || length < 1) {
    return std::string();
  }

  return common::lpwstr2str(buf, codepage, flags);
}
