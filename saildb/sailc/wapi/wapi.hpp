#pragma once

#include "sailc/common/data.hpp"
#include "sailc/common/typing.hpp"
#include "sailc/common/cstring.hpp"
#include "sailc/common/strutil.hpp"

#include <ios>
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include <cwctype>
#include <cstddef>
#include <stdexcept>
#include <filesystem>
#include <unordered_map>

namespace common = saildb::common;

namespace saildb {
namespace wapi {

/************************************************************
 *                                                          *
 *                           LSA                            *
 *                                                          *
 ************************************************************/
#pragma region lsa_decl

bool tryGetUsername(std::string& username, std::string& errorMessage);
bool tryGetDomainStatus(bool& isDomain, std::string& errorMessage);
bool tryGetUsernameAndDomain(std::string& username, std::string& domain, std::string& errorMessage);
bool tryGetPasswordChangedTimepoint(std::chrono::system_clock::time_point& timepoint, std::string& errorMessage);
bool tryGetSessionInfo(common::Session& result, std::string& errorMessage);

#pragma endregion



/************************************************************
 *                                                          *
 *                         Secrets                          *
 *                                                          *
 ************************************************************/
#pragma region wapi_cred_decl

bool hasSecret(
  const std::string& label,
  const std::string& account,
  const std::string& datasource,
  bool& hasSecret,
  std::string& errorMessage
);

bool hasAnySecrets(const std::string& label, bool& hasSecrets, std::string& errorMessage);
bool hasAnySecrets(const std::string& label, const std::string& datasource, bool& hasSecrets, std::string& errorMessage);

bool tryListSecrets(const std::string& label, std::vector<common::Secret>& secrets, std::string& errorMessage);
bool tryListSecrets(const std::string& label, const std::string& datasource, std::vector<common::Secret>& secrets, std::string& errorMessage);

bool tryCompareSecret(
  const std::string& label,
  const std::string& account,
  const std::string& datasource,
  const std::string& secret,
  bool& isEqual,
  std::string& errorMessage
);

bool tryGetSecret(
  const std::string& label,
  const std::string& account,
  const std::string& datasource,
  std::string& secret,
  std::string& errorMessage
);

bool tryDeleteSecret(
  const std::string& label,
  const std::string& account,
  const std::string& datasource,
  std::string& errorMessage
);

bool tryStoreSecret(
  const std::string& label,
  const std::string& username,
  const std::string& account,
  const std::string& datasource,
  const std::string& secret,
  bool isUserAccount,
  std::string& errorMessage
);

#pragma endregion



/************************************************************
 *                                                          *
 *                           Env                            *
 *                                                          *
 ************************************************************/
#pragma region env_decl

bool tryGetEnvVar(const std::string& varName, std::string &result);
bool tryGetEnvVar(const std::wstring& varName, std::wstring &result);

class DotEnv {
  public:
    // Flags
    static constexpr const uint8_t NO_CHECK_EXT   = 0x1 << 0; // Don't enforce `.env` file ext
    static constexpr const uint8_t NO_INTERPOLATE = 0x1 << 1; // Don't interpolate vars from [ `$VAR` | `${VAR}` ]

    // Interpolation
    enum ExpansionExpr : uint8_t {
      None,
      SubstituteEmpty,
      SubstituteUnset
    };

    struct InterpToken {
      std::wstring name;
      std::wstring defaultValue;
      size_t start;
      size_t end;
      ExpansionExpr expr;

      InterpToken(
        std::wstring tName,
        std::wstring tValue,
        size_t tStart,
        size_t tEnd,
        ExpansionExpr tExpr
      );

      InterpToken() = default;
      InterpToken(InterpToken&& other) = default;
      InterpToken(const InterpToken& other) = default;
      InterpToken& operator=(InterpToken& other) = default;
      InterpToken& operator=(const InterpToken& other) = default;
    };

  public:
    DotEnv();
    DotEnv(const std::string& fp, uint8_t flags = 0);
    DotEnv(const std::wstring& fp, uint8_t flags = 0);
    DotEnv(const std::filesystem::path& fp, uint8_t flags = 0);
    ~DotEnv() = default;

    DotEnv(DotEnv const&) = default;
    DotEnv &operator=(DotEnv const&) = default;
    DotEnv &operator=(DotEnv&&) = default;

    static bool IsEnvFile(const std::filesystem::path& fp);
    static const std::wstring_view GetEnvExtension();

  public:
    bool IsEmpty() const;
    uint8_t GetFlags() const;

    template <typename T>
    auto Contains(const T& key) const -> bool;

    template <typename U, typename T>
    auto Get(const T& key) const -> U;

    template <typename U, typename T>
    auto Get(const T& key, U&& defaultValue) const -> U;

    template <typename T, typename U>
    auto TryGet(const T& key, U& value) const -> bool;

  private:
    void parseFile(const std::filesystem::path& fp, uint8_t flags = 0);
    void expandContent(std::wstring& content, const bool& isValueQuoted);

    bool coerceIntoBoolean(std::wstring value) const;

    template<typename T>
    auto coerceIntoType(const std::wstring& value) const -> T;

  private:
    std::unordered_map<std::wstring, std::wstring> m_entries;
    uint8_t m_flags{0};
};


/* Public */
template <typename T>
inline auto DotEnv::Contains(const T& key) const -> bool {
  static_assert((std::is_convertible_v<T, std::string> || std::is_convertible_v<T, std::wstring>));
  if constexpr(std::is_same_v<T, std::string>) {
    return m_entries.contains(common::str2wstr(key));
  } else if constexpr(std::is_convertible_v<T, std::string>) {
    return m_entries.contains(common::str2wstr(std::string(key)));
  } else if constexpr(std::is_same_v<T, std::wstring>) {
    return m_entries.contains(key);
  } else if constexpr(std::is_convertible_v<T, std::wstring>) {
    return m_entries.contains(std::wstring(key));
  }
}

template <typename U, typename T>
inline auto DotEnv::Get(const T& key) const -> U {
	static_assert((std::is_convertible_v<T, std::string> || std::is_convertible_v<T, std::wstring>));

  std::wstring keyname;
  if constexpr(std::is_same_v<T, std::string>) {
    keyname = common::str2wstr(key);
  } else if constexpr(std::is_convertible_v<T, std::string>) {
    keyname = common::str2wstr(std::string(key));
  } else if constexpr(std::is_same_v<T, std::wstring>) {
    keyname = key;
  } else if constexpr(std::is_convertible_v<T, std::wstring>) {
    keyname = { key };
  }

  const auto entry = m_entries.find(keyname);
  if (entry == m_entries.end()) {
    throw std::runtime_error(
      std::string("Key of name '")
        .append(common::wstr2str(keyname))
        .append("' does not exist")
    );
  }

  auto value = entry->second;
  if constexpr(std::is_same_v<U, std::string> || std::is_convertible_v<U, std::string_view>) {
    return common::wstr2str(value);
  } else if constexpr(std::is_same_v<U, std::wstring> || std::is_convertible_v<U, std::wstring_view>) {
    return value;
  } else if constexpr(std::is_same_v<U, bool>) {
    return this->coerceIntoBoolean(value);
  } else {
    return this->coerceIntoType<U>(value);
  }
}

template <typename U, typename T>
inline auto DotEnv::Get(const T& key, U&& defaultValue) const -> U {
	static_assert((std::is_convertible_v<T, std::string> || std::is_convertible_v<T, std::wstring>));

  try {
    return this->Get<U>(key);
  } catch (std::runtime_error& err) {
    return defaultValue;
  }
}

template <typename T, typename U>
inline auto DotEnv::TryGet(const T& key, U& value) const -> bool {
	static_assert((std::is_convertible_v<T, std::string> || std::is_convertible_v<T, std::wstring>));

  try {
    value = this->Get<U>(key);
  } catch (std::runtime_error& err) {
    return false;
  }

  return true;
}


/* Private */
inline bool DotEnv::coerceIntoBoolean(std::wstring value) const {
  static constexpr const char* expectedValues = "1/0, true/false, on/off"; 
  static const std::unordered_map<std::wstring, bool> booleanMap{
    { L"true", true }, { L"false", false },
    {   L"on", true }, {   L"off", false }
  };

  common::trim(value);
  std::transform(value.begin(), value.end(), value.begin(), std::towlower);

  if (value.length() == 1) {
    switch (value.front()) {
      case L'1':
        return true;
      case L'0':
        return false;
      default:
        break;
    }
  } else {
    const auto mapped = booleanMap.find(value);
    if (mapped != booleanMap.end()) {
      return mapped->second;
    }
  }

  throw std::runtime_error(
    std::string("Failed to coerce '")
      .append(common::wstr2str(value))
      .append("' into boolean, expected one of: ")
      .append(expectedValues)
  );
}

template<typename T>
inline auto DotEnv::coerceIntoType(const std::wstring& value) const -> T {
  try {
    T result{};
    auto stream = std::wistringstream{value};
    stream.exceptions(std::ios::failbit);
    stream >> result;

    return result;
  } catch (std::exception& err) {
    throw std::runtime_error(
      std::string("Failed to coerce '")
        .append(common::wstr2str(value))
        .append("' into ")
        .append(common::getTypeName<T>())
    );
  }
}

#pragma endregion

} // namespace wapi
} // namespace saildb
