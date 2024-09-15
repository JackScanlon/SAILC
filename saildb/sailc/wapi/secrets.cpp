#include "wapi.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <wincred.h>
#include <lmcons.h>

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#include <Security.h>
#pragma comment(lib, "Secur32.lib")

#include <utility>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <codecvt>
#include <sstream>
#include <algorithm>
#include <exception>

#include "sailc/wapi/internal.hpp"
#include "sailc/common/utils.hpp"
#include "sailc/common/constants.hpp"

namespace wapi = saildb::wapi;
namespace common = saildb::common;
namespace constants = saildb::constants;



/************************************************************
 *                                                          *
 *                       Credentials                        *
 *                                                          *
 ************************************************************/

#pragma region wapi_cred_impl

bool wapi::hasSecret(
  const std::string& label,
  const std::string& account,
  const std::string& datasource,
  bool& hasSecret,
  std::string& errorMessage
) {
  std::wstring filterLabel = common::str2wstr(label + '/' + datasource + '/' + account + '*');
  hasSecret = false;

  DWORD credLength;
  CREDENTIALW** credentials{nullptr};

  auto cDel = common::OnScopeExit([&]() { CredFree(credentials); });
  DWORD flag = 0;
  LPWSTR filter = filterLabel.data();

  bool result = CredEnumerateW(filter, flag, &credLength, &credentials);
  if (!result) {
    DWORD errorCode = GetLastError();
    if (errorCode != ERROR_NOT_FOUND) {
      errorMessage = wapi::internal::getErrorMessage(errorCode);
      return false;
    }
  }

  if (credentials != nullptr) {
    for (uint32_t i = 0; i < credLength; ++i) {
      CREDENTIALW* cred = credentials[i];
      if (cred->UserName == NULL || cred->CredentialBlob == NULL || cred->CredentialBlobSize == NULL) {
        continue;
      }

      hasSecret = true;
      break;
    }
  }

  return true;
}

bool wapi::hasAnySecrets(const std::string& label, bool& hasSecrets, std::string& errorMessage) {
  std::wstring filterLabel = common::str2wstr(label + '*');
  hasSecrets = false;

  DWORD credLength;
  CREDENTIALW** credentials{nullptr};

  auto cDel = common::OnScopeExit([&]() { CredFree(credentials); });
  DWORD flag = 0;
  LPWSTR filter = filterLabel.data();

  bool result = CredEnumerateW(filter, flag, &credLength, &credentials);
  if (!result) {
    DWORD errorCode = GetLastError();
    if (errorCode != ERROR_NOT_FOUND) {
      errorMessage = wapi::internal::getErrorMessage(errorCode);
      return false;
    }
  }

  if (credentials != nullptr) {
    for (uint32_t i = 0; i < credLength; ++i) {
      CREDENTIALW* cred = credentials[i];
      if (cred->UserName == NULL || cred->CredentialBlob == NULL || cred->CredentialBlobSize == NULL) {
        continue;
      }

      hasSecrets = true;
      break;
    }
  }

  return true;
}

bool wapi::hasAnySecrets(const std::string& label, const std::string& datasource, bool& hasSecrets, std::string& errorMessage) {
  std::wstring filterLabel = common::str2wstr(label + '/' + datasource + '*');
  hasSecrets = false;

  DWORD credLength;
  CREDENTIALW** credentials{nullptr};

  auto cDel = common::OnScopeExit([&]() { CredFree(credentials); });
  DWORD flag = 0;
  LPWSTR filter = filterLabel.data();

  bool result = CredEnumerateW(filter, flag, &credLength, &credentials);
  if (!result) {
    DWORD errorCode = GetLastError();
    if (errorCode != ERROR_NOT_FOUND) {
      errorMessage = wapi::internal::getErrorMessage(errorCode);
      return false;
    }
  }

  if (credentials != nullptr) {
    for (uint32_t i = 0; i < credLength; ++i) {
      CREDENTIALW* cred = credentials[i];
      if (cred->UserName == NULL || cred->CredentialBlob == NULL || cred->CredentialBlobSize == NULL) {
        continue;
      }

      hasSecrets = true;
    }
  }

  return true;
}

bool wapi::tryListSecrets(const std::string& label, std::vector<common::Secret>& secrets, std::string& errorMessage) {
  std::wstring service = common::str2wstr(label);
  std::wstring filterLabel(service + L'*');
  secrets.clear();

  DWORD credLength;
  CREDENTIALW** credentials{nullptr};

  auto cDel = common::OnScopeExit([&]() { CredFree(credentials); });
  DWORD flag = 0;
  LPWSTR filter = filterLabel.data();

  bool result = CredEnumerateW(filter, flag, &credLength, &credentials);
  if (!result) {
    DWORD errorCode = GetLastError();
    if (errorCode != ERROR_NOT_FOUND) {
      errorMessage = wapi::internal::getErrorMessage(errorCode);
      return false;
    }
  }

  if (credentials != nullptr) {
    std::wstring dsnKeyword = wapi::internal::getKeywordIdentifier(service, constants::CREDENTIAL_KEY_DSN);
    std::wstring iuaKeyword = wapi::internal::getKeywordIdentifier(service, constants::CREDENTIAL_KEY_IUA);

    for (uint32_t i = 0; i < credLength; ++i) {
      CREDENTIALW* cred = credentials[i];
      if (cred->UserName == NULL || cred->CredentialBlob == NULL || cred->CredentialBlobSize == NULL) {
        continue;
      }

      bool isUserAccount = false;
      std::string datasource;
      for (uint32_t j = 0; j < cred->AttributeCount; ++j) {
        const auto &attribute = cred->Attributes[j];
        if (attribute.ValueSize > 0) {
          std::wstring keyword(attribute.Keyword);
          if (keyword == dsnKeyword && attribute.ValueSize > 0) {
            std::vector<wchar_t> dValue(attribute.ValueSize);
            std::mbstowcs(&dValue[0], reinterpret_cast<char*>(attribute.Value), attribute.ValueSize);

            datasource = common::wstr2str(std::wstring(dValue.begin(), dValue.end()));
          } else if (keyword == iuaKeyword && attribute.ValueSize > 0) {
            isUserAccount = reinterpret_cast<bool*>(attribute.Value)[0];
          }
        }
      }

      common::Secret elem {
        common::lpwstr2str(cred->TargetName),
        common::lpwstr2str(cred->UserName),
        datasource,
        std::string(reinterpret_cast<char*>(cred->CredentialBlob), cred->CredentialBlobSize),
        isUserAccount,
        wapi::internal::filetime2timepoint(cred->LastWritten.dwHighDateTime, cred->LastWritten.dwLowDateTime)
      };

      secrets.push_back(std::move(elem));
    }
  }

  return true;
}

bool wapi::tryListSecrets(const std::string& label, const std::string& datasource, std::vector<common::Secret>& secrets, std::string& errorMessage) {
  std::wstring filterLabel = common::str2wstr(label + '/' + datasource + '*');
  secrets.clear();

  DWORD credLength;
  CREDENTIALW** credentials{nullptr};

  auto cDel = common::OnScopeExit([&]() { CredFree(credentials); });
  DWORD flag = 0;
  LPWSTR filter = filterLabel.data();

  bool result = CredEnumerateW(filter, flag, &credLength, &credentials);
  if (!result) {
    DWORD errorCode = GetLastError();
    if (errorCode != ERROR_NOT_FOUND) {
      errorMessage = wapi::internal::getErrorMessage(errorCode);
      return false;
    }
  }

  if (credentials != nullptr) {
    for (uint32_t i = 0; i < credLength; ++i) {
      CREDENTIALW* cred = credentials[i];
      if (cred->UserName == NULL || cred->CredentialBlob == NULL || cred->CredentialBlobSize == NULL) {
        continue;
      }

      common::Secret elem {
        common::lpwstr2str(cred->TargetName),
        common::lpwstr2str(cred->UserName),
        std::string(datasource),
        std::string(reinterpret_cast<char*>(cred->CredentialBlob), cred->CredentialBlobSize),
        false,
        wapi::internal::filetime2timepoint(cred->LastWritten.dwHighDateTime, cred->LastWritten.dwLowDateTime)
      };

      secrets.push_back(std::move(elem));
    }
  }

  return true;
}

bool wapi::tryCompareSecret(
  const std::string& label,
  const std::string& account,
  const std::string& datasource,
  const std::string& secret,
  bool& isEqual,
  std::string& errorMessage
) {
  std::wstring targetLabel = common::str2wstr(label + '/' + datasource + '/' + account);
  isEqual = false;

  DWORD flag = 0;
  CREDENTIALW* cred{nullptr};
  LPWSTR filter = targetLabel.data();

  auto cDel = common::OnScopeExit([&]() { CredFree(cred); });
  bool result = CredReadW(filter, CRED_TYPE_GENERIC, flag, &cred);
  if (!result) {
    DWORD errorCode = GetLastError();
    switch (errorCode) {
      case ERROR_NOT_FOUND:
      case ERROR_NO_SUCH_LOGON_SESSION:
      case ERROR_INVALID_FLAGS:
        break;

      default: {
        errorMessage = wapi::internal::getErrorMessage(errorCode);
        return false;
      }
    }
  }

  if (cred != nullptr) {
    std::string spwd(reinterpret_cast<char*>(cred->CredentialBlob), cred->CredentialBlobSize);
    isEqual = spwd.compare(secret);
  }

  return true;
}

bool wapi::tryGetSecret(
  const std::string& label,
  const std::string& account,
  const std::string& datasource,
  std::string& secret,
  std::string& errorMessage
) {
  std::wstring targetLabel = common::str2wstr(label + '/' + datasource + '/' + account);
  secret.clear();

  CREDENTIALW* cred{nullptr};
  auto cDel = common::OnScopeExit([&]() { CredFree(cred); });

  DWORD flag = 0;
  LPWSTR filter = targetLabel.data();

  bool result = CredReadW(filter, CRED_TYPE_GENERIC, flag, &cred);
  if (!result) {
    DWORD errorCode = GetLastError();
    switch (errorCode) {
      case ERROR_NOT_FOUND:
      case ERROR_NO_SUCH_LOGON_SESSION:
      case ERROR_INVALID_FLAGS:
        break;

      default: {
        errorMessage = wapi::internal::getErrorMessage(errorCode);
        return false;
      }
    }
  }

  if (cred != nullptr) {
    secret.assign(cred->CredentialBlob, cred->CredentialBlob + cred->CredentialBlobSize);
  }

  return true;
}

bool wapi::tryDeleteSecret(
  const std::string& label,
  const std::string& account,
  const std::string& datasource,
  std::string& errorMessage
) {
  std::wstring targetLabel = common::str2wstr(label + '/' + datasource + '/' + account);

  DWORD flag = 0;
  LPWSTR target = targetLabel.data();

  bool result = CredDeleteW(target, CRED_TYPE_GENERIC, flag);
  if (!result) {
    DWORD errorCode = GetLastError();
    switch (errorCode) {
      case ERROR_NOT_FOUND:
      case ERROR_NO_SUCH_LOGON_SESSION:
      case ERROR_INVALID_FLAGS:
        break;

      default: {
        errorMessage = wapi::internal::getErrorMessage(errorCode);
        return false;
      }
    }
  }

  return true;
}

bool wapi::tryStoreSecret(
  const std::string& label,
  const std::string& username,
  const std::string& account,
  const std::string& datasource,
  const std::string& secret,
  bool isUserAccount,
  std::string& errorMessage
) {
  std::wstring service = common::str2wstr(label);
  std::wstring userLabel = common::str2wstr(username);
  std::wstring datasourceLabel = common::str2wstr(datasource);

  std::wstring targetLabel = service + L'/' + datasourceLabel + L'/' + common::str2wstr(account);
  std::wstring credComment(service.data(), service.size());
  credComment.append(constants::CREDENTIAL_COMMENT);

  std::wstring dsnKeyword = wapi::internal::getKeywordIdentifier(service, constants::CREDENTIAL_KEY_DSN);
  std::wstring iuaKeyword = wapi::internal::getKeywordIdentifier(service, constants::CREDENTIAL_KEY_IUA);

  std::vector<char> iuaValue{ isUserAccount };
  std::vector<char> dsnValue(datasourceLabel.size() * 4);
  size_t dsnLen = std::wcstombs(&dsnValue[0], &datasourceLabel[0], dsnValue.size());

  DWORD flag = 0;
  SYSTEMTIME systime;
  GetSystemTime(&systime);

  CREDENTIAL_ATTRIBUTEW attributes[2];
  attributes[0].Keyword = dsnKeyword.data();
  attributes[0].Flags = flag;
  attributes[0].Value = (LPBYTE)dsnValue.data();
  attributes[0].ValueSize = dsnLen;

  attributes[1].Keyword = iuaKeyword.data();
  attributes[1].Flags = flag;
  attributes[1].Value = (LPBYTE)iuaValue.data();
  attributes[1].ValueSize = iuaValue.size();

  CREDENTIAL cred{0};
  cred.Type = CRED_TYPE_GENERIC;
  cred.Persist = CRED_PERSIST_ENTERPRISE;
  cred.Comment = credComment.data();
  cred.UserName = userLabel.data();
  cred.TargetName = targetLabel.data();
  cred.Attributes = attributes;
  cred.AttributeCount = std::size(attributes);
  cred.CredentialBlob = (LPBYTE)(secret.data());
  cred.CredentialBlobSize = secret.size();
  SystemTimeToFileTime(&systime, &cred.LastWritten);

  bool result = CredWriteW(&cred, flag);
  if (!result) {
    DWORD errorCode = GetLastError();
    errorMessage = wapi::internal::getErrorMessage(errorCode);
    return false;
  }

  return true;
}

#pragma endregion
