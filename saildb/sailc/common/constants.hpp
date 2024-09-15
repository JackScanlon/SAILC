#pragma once

#include <string>
#include <chrono>

namespace saildb {
namespace constants {

/* Unicode byte order mark*/
inline constexpr const wchar_t UNICODE_BYTE_ORDER_MARK = 0xFEFF;

/* Filetime conversion constants */
inline constexpr const std::chrono::seconds FILETIME_EPOCH { -11'644'473'600 }; 					// Windows FILETIME<->SYSTIME conversion

/* Credential constants */
inline constexpr const std::wstring_view CREDENTIAL_COMMENT(L"DBI Database Credentials"); // Credential comment tag
inline constexpr const std::wstring_view CREDENTIAL_KEY_DSN(L"DatasourceName"); 					// Datasource credential attribute keyword
inline constexpr const std::wstring_view CREDENTIAL_KEY_IUA(L"UserAccount"); 							// UserAccount credential attribute keyword

} // namespace constants
} // namespace saildb
