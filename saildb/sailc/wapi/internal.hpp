#pragma once

#include <string>
#include <chrono>
#include <cstdint>

namespace saildb {
namespace wapi {
namespace internal {

std::chrono::system_clock::time_point filetime2timepoint(const int32_t& highWord, const int32_t& lowWord);

int32_t convertNtStatusToWin32Error(int32_t ntstatus);

std::string getErrorMessage(const uint32_t& errorCode, int16_t languageId = 0);

std::wstring getKeywordIdentifier(const std::wstring& serviceName, const std::wstring_view& keyword, wchar_t separator = L'_');

void concatKeywordIdentifier(std::wstring& serviceName, const std::wstring_view& keyword, wchar_t separator = L'_');

} // namespace internal
} // namespace wapi
} // namespace saildb
