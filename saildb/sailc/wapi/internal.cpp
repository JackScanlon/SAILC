#include "internal.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <ioapiset.h>

#include "sailc/common/cstring.hpp"
#include "sailc/common/constants.hpp"

namespace common = saildb::common;
namespace internal = saildb::wapi::internal;
namespace constants = saildb::constants;

std::chrono::system_clock::time_point internal::filetime2timepoint(const int32_t& highWord, const int32_t& lowWord) {
  std::chrono::file_clock::duration duration{(static_cast<int64_t>(highWord) << 32) | lowWord};
  duration = constants::FILETIME_EPOCH + duration;

  std::chrono::system_clock::time_point timepoint;
  return timepoint;
}

int32_t internal::convertNtStatusToWin32Error(int32_t ntstatus) {
  /*
   * NOTE:
   *  We should be loading ntdll.dll instead but to avoid that
   *  we're attempting to find the overlapped operation results
   *
   *  See here for reference: https://stackoverflow.com/a/32205631
   *
   */
  OVERLAPPED o;
  o.Internal = ntstatus;
  o.InternalHigh = 0;
  o.Offset = 0;
  o.OffsetHigh = 0;
  o.hEvent = 0;

  DWORD br;
  DWORD oldError = GetLastError();
  GetOverlappedResult(NULL, &o, &br, FALSE);

  DWORD result = GetLastError();
  SetLastError(oldError);

  return result;
}

std::string internal::getErrorMessage(const uint32_t& errorCode, int16_t languageId /*= 0*/) {
  LPWSTR msgBuf;
  DWORD msgLen = FormatMessageW(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    errorCode,
    languageId,
    (LPWSTR)&msgBuf,
    0,
    NULL
  );

  std::string result;
  if (msgBuf != NULL || msgLen == 0) {
    result = "Unknown error occurred with error code: " + std::to_string(errorCode);
  } else {
    result = common::lpwstr2str(msgBuf);
    LocalFree(msgBuf);
  }

  return result;
}

std::wstring internal::getKeywordIdentifier(const std::wstring& serviceName, const std::wstring_view& keyword, wchar_t delimiter) {
  // See: https://learn.microsoft.com/en-us/windows/win32/api/wincred/ns-wincred-credential_attributew#members
  std::wstring result(serviceName.data(), serviceName.size());
  result.append(1, delimiter);
  result.append(keyword);

  return result;
}

void internal::concatKeywordIdentifier(std::wstring& serviceName, const std::wstring_view& keyword, wchar_t delimiter) {
  // See: https://learn.microsoft.com/en-us/windows/win32/api/wincred/ns-wincred-credential_attributew#members
  serviceName.append(1, delimiter).append(keyword);
}
