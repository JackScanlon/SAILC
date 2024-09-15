#include "wapi.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <lmcons.h>

#include <locale>
#include <clocale>
#include <fstream>
#include <codecvt>
#include <algorithm>

#include "sailc/wapi/internal.hpp"
#include "sailc/common/utils.hpp"
#include "sailc/common/constants.hpp"

namespace wapi = saildb::wapi;
namespace common = saildb::common;



/************************************************************
 *                                                          *
 *                           Sys                            *
 *                                                          *
 ************************************************************/

bool wapi::tryGetEnvVar(const std::string& varName, std::string &result) {
  std::wstring res;
  if (wapi::tryGetEnvVar(common::str2wstr(varName), res)) {
    result = common::wstr2str(res);
    return true;
  }

  return false;
}

bool wapi::tryGetEnvVar(const std::wstring& varName, std::wstring &result) {
  DWORD length = GetEnvironmentVariableW(varName.c_str(), nullptr, NULL);
  if (length > 0) {
    result.resize(length);

    if (GetEnvironmentVariableW(varName.c_str(), &result[0], length) != 0) {
      return true;
    }
  }

  return false;
}



/************************************************************
 *                                                          *
 *                         Parsing                          *
 *                                                          *
 ************************************************************/

using InterpToken = wapi::DotEnv::InterpToken;
using ExpansionExpr = wapi::DotEnv::ExpansionExpr;


// Impl. InterpToken
InterpToken::InterpToken(std::wstring tName, std::wstring tValue, size_t tStart, size_t tEnd, ExpansionExpr tExpr)
  : name(std::move(tName)), defaultValue(std::move(tValue)),
    start(std::exchange(tStart, 0)), end(std::exchange(tEnd, 0)), expr(std::move(tExpr)) { };


// Validation
bool isLegalEnvChar(wchar_t ch) {
  return std::iswalnum(ch) || ch == L'_';
}

bool isLegalEnvKeyword(const std::wstring& key) {
  if (key.empty() || !std::iswalpha(key.front())) {
    return false;
  }

  return std::all_of(key.begin(), key.end(), ::isLegalEnvChar);
}


// Utility
std::wstring::size_type getClosingQuote(const std::wstring& str, const wchar_t& tag, size_t i = 0) {
  size_t length = str.length();
  if (str.empty() || i >= length) {
    return std::wstring::npos;
  }

  wchar_t ch;
  bool escaped = false;
  while (i < length) {
    ch = str.at(i);

    bool isEscSeq = (!escaped && ch == L'\\');
    if (isEscSeq) {
      escaped = !escaped;
    }

    if (!escaped && ch == tag) {
      return i;
    }

    escaped = (escaped && !isEscSeq) ? !escaped : escaped;
    i++;
  }

  return std::wstring::npos;
}

size_t unescapeSequence(std::wstring& str, size_t& index) {
  const auto ch = str.at(index);
  switch (ch) {
    case L'n':
      str.replace(index - 1, 2, L"\n");
      break;
    case L'r':
      str.replace(index - 1, 2, L"\r");
      break;
    case L't':
      str.replace(index - 1, 2, L"\t");
      break;
    case L'b':
    case L'v':
    case L'f':
      str.erase(index - 1, 2);
      index--;
      break;
    default:
      str.erase(index - 1, 1);
      break;
  }

  return str.length();
}

void unescapeContent(std::wstring& str) {
  size_t length = str.length();
  if (length < 2) {
    return;
  }

  size_t i = 0;
  bool escaped = false;
  while (i < length) {
    const auto ch = str.at(i);
    if (ch == L'\\') {
      length = ::unescapeSequence(str, ++i);
      continue;
    }

    i++;
  }
}

bool tryCreateToken(std::wstring& name, size_t& start, size_t& end, InterpToken& token) {
  auto expr = ExpansionExpr::None;
  auto defaultValue = std::wstring();

  auto delim = name.find(L'-');
  if (delim != std::wstring::npos) {
    defaultValue = name.substr(delim + 1);

    if (delim > 0 && name.at(delim - 1) == L':') {
      expr = ExpansionExpr::SubstituteEmpty;
      name.erase(delim - 1, name.length());
    } else {
      expr = ExpansionExpr::SubstituteUnset;
      name.erase(delim, name.length());
    }
  }

  name.erase(
    std::remove_if(name.begin(), name.end(), [&](auto ch) { return !(::isLegalEnvChar(ch)); }),
    name.end()
  );

  if (name.length() < 1) {
    return false;
  }

  token = { name, defaultValue, start, end, expr };
  return true;
}

bool processContent(std::wstring& str, size_t& index, size_t& length, const bool& isQuoted, wapi::DotEnv::InterpToken& token) {
  if (length < 2 || index >= length - 1) {
    return false;
  }

  size_t startIndex = 0,
         endIndex = 0;

  bool inToken = false,
       escaped = false,
       requireClosure = false,
       hasInterpToken = false;

  std::wstring tag, content;
  while (!hasInterpToken && index < length) {
    auto chead = str.at(index);
    bool isEscSeq = (!escaped && chead == L'\\');
    if (isEscSeq) {
      escaped = !escaped;
    }

    if (!inToken) {
      if (length - index >= 2 && !escaped && chead == L'$') {
        auto pos = index + 1;
        auto cnext = str.at(pos);
        if (cnext != L'{') {
          tag += chead;
          startIndex = index;
          index = pos;
          inToken = true;
        } else if (length - pos > 2) {
          requireClosure = true;
          startIndex = index;
          index = ++pos;
          tag = std::wstring(1, chead) + cnext;
          inToken = true;
        }
      } else if (isQuoted && escaped && !isEscSeq) {
        length = ::unescapeSequence(str, index);
      } else {
        index++;
      }
    } else if (requireClosure && !escaped && chead == L'}') {
        hasInterpToken = true;
        endIndex = ++index;
        break;
    } else {
      const bool isLastChar = index >= length - 1;
      if (isLastChar) {
        content += chead;
        index++;
      }

      hasInterpToken = (isLastChar || std::iswspace(chead));
      if (hasInterpToken) {
        endIndex = index;
        break;
      }

      content += chead;
      index++;
    }

    if (escaped && !isEscSeq) {
      escaped = !escaped;
    }
  }

  hasInterpToken = (hasInterpToken && content.length() > 0);
  if (hasInterpToken) {
    return (::tryCreateToken(content, startIndex, endIndex, token));
  }

  return false;
}




/************************************************************
 *                                                          *
 *                         DotEnv                           *
 *                                                          *
 ************************************************************/

// Impl. DotEnv reader
wapi::DotEnv::DotEnv() = default;

wapi::DotEnv::DotEnv(const std::string& fp, uint8_t flags /*= 0*/) {
  parseFile(std::filesystem::path(common::str2wstr(fp)), flags);
}

wapi::DotEnv::DotEnv(const std::wstring& fp, uint8_t flags /*= 0*/) {
  parseFile(std::filesystem::path(fp), flags);
}

wapi::DotEnv::DotEnv(const std::filesystem::path& fp, uint8_t flags /*= 0*/) {
  parseFile(fp, flags);
}


/* Static impl. */
const std::wstring_view wapi::DotEnv::GetEnvExtension() {
  static const std::wstring_view FILE_EXT(L".env");
  return FILE_EXT;
}

bool wapi::DotEnv::IsEnvFile(const std::filesystem::path& fp) {
  auto filename = fp.filename().wstring();

  const auto ext = wapi::DotEnv::GetEnvExtension();
  if (filename.length() < ext.length()) {
    return false;
  }

  std::transform(filename.begin(), filename.end(), filename.begin(), std::towlower);
  return (filename.find(ext) != std::wstring::npos);
}


/* Public impl. */
bool wapi::DotEnv::IsEmpty() const {
  return m_entries.empty();
}

uint8_t wapi::DotEnv::GetFlags() const {
  return m_flags;
}


/* Private impl. */
void wapi::DotEnv::parseFile(const std::filesystem::path& fp, uint8_t flags /*= 0*/) {
  // TODO:
  //  - Do we want to support parsing warning(s)/error(s)?
  //  - Similarly, do we want to bubble these to the client or should they
  //    just be sunk? Could also be hidden behind a flag?
  //

  static constexpr const std::wstring_view whitespace(L" \t\n\v\r\f;");
  m_flags = flags;

  if (!(flags & wapi::DotEnv::NO_CHECK_EXT) && !wapi::DotEnv::IsEnvFile(fp)) {
    throw std::invalid_argument(common::concatTo<std::string>("Expected .env file type but got ", fp.extension()));
  }

  std::wifstream ws(fp.wstring());
  if (ws.is_open()) {
    auto closure = common::OnScopeExit([&]() { ws.close(); });
    // ws.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

    // Skip UTF-8 BOM if present
    if (ws.peek() == saildb::constants::UNICODE_BYTE_ORDER_MARK) {
      ws.get();
    }

    // Parse
    std::wstring line;
    uint32_t lcount = 0;
    while (std::getline(ws, line)) {
      const auto nwsPos = line.find_first_not_of(whitespace);

      // Skip empty and/or comment-only line(s)
      if (nwsPos == std::wstring::npos || line.at(nwsPos) == L'#') {
        lcount++;
        continue;
      }
      line.erase(0, nwsPos);

      const auto eqTkPos = line.find(L'=');
      if (eqTkPos == std::wstring::npos) {
        // [?] Record issue: Malformed assignment on Line<n>
        lcount++;
        continue;
      }

      auto key = line.substr(0, eqTkPos);
      common::trimRight(key);

      if (!::isLegalEnvKeyword(key)) {
        // [?] Record issue: Malformed key on Line<n>
        lcount++;
        continue;
      }

      auto val = line.substr(eqTkPos + 1);
      common::trimLeft(val);

      // Early exit empty key-value pair(s)
      if (val.length() < 1) {
        m_entries.insert_or_assign(key, val);
        lcount++;
        continue;
      }

      const auto qchar = val.front();
      bool isEnclosed = false;
      bool isSingleQuote = qchar == L'\'';
      bool isValueQuoted = (isSingleQuote || qchar == L'\"' || qchar == L'`');
      if (isValueQuoted) {
        auto vTailPos = ::getClosingQuote(val, qchar, 1);
        if (vTailPos == std::wstring::npos) {
          // Attempt to parse multi-line value
          //  e.g. KEY="...Value... \ ... \ ..."
          std::wstring head;
          val = val.substr(1);

          uint32_t steps = 0;
          while (std::getline(ws, head)) {
            vTailPos = ::getClosingQuote(head, qchar);
            if (vTailPos != std::wstring::npos) {
              val += head.length() > 1 ? (L'\n' + head.substr(0, vTailPos)) : L"\n";
              isEnclosed = true;
              lcount = steps;
              break;
            }

            val += L'\n' + head;
            steps++;
          }
        } else {
          // Parse as quoted single-line key-value
          //  e.g. KEY='...Value...`, KEY="...Value..." etc
          val.erase(0, 1);
          val.erase(vTailPos - 1, val.length());

          isEnclosed = true;
        }
      } else {
        // Parse as unquoted single-line key-value
        //  e.g. KEY=...Value...
        const auto vTailPos = val.find(L'#');
        if (vTailPos != std::wstring::npos) {
          val.erase(vTailPos - 1, val.length());
        }
        common::trimRight(val);
        isEnclosed = true;
      }

      // Process value
      if ((!isValueQuoted || !isSingleQuote) && !(flags & wapi::DotEnv::NO_INTERPOLATE)) {
        this->expandContent(val, isValueQuoted);
      } else if (isValueQuoted && !isSingleQuote) {
        ::unescapeContent(val);
      }
      m_entries.insert_or_assign(key, val);

      if (!isEnclosed) {
        // [?] Record issue: malformed closure for Line<n>
      }

      lcount++;
    }
  }
}

void wapi::DotEnv::expandContent(std::wstring& content, const bool& isValueQuoted) {
  size_t length = content.length();
  if (length < 2) {
    return;
  }

  size_t index = 0;
  while (index < length) {
    InterpToken token;
    if (!(::processContent(content, index, length, isValueQuoted, token))) {
      continue;
    }

    auto name = token.name;
    auto expr = token.expr;
    auto size = token.end - token.start;
    auto value = std::wstring();

    bool isSetValue = wapi::tryGetEnvVar(name, value);
    bool isEmptyValue = (!isSetValue || value.empty());
    if (!isSetValue && m_entries.contains(name)) {
      value = m_entries.at(name);
      isSetValue = !isSetValue;
      isEmptyValue = value.empty();
    }

    if (isEmptyValue && !token.defaultValue.empty()) {
      switch (expr) {
        case ExpansionExpr::SubstituteUnset: {
          if (!isSetValue) {
            value = token.defaultValue;
          }
        } break;

        case ExpansionExpr::SubstituteEmpty: {
          value = token.defaultValue;
        } break;

        default:
          break;
      }
    }

    content.replace(token.start, size, value);
    length = content.length();
  }
}
