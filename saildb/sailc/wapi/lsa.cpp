#include "wapi.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#pragma comment(lib, "netapi32.lib")
#include <wincred.h>
#include <Ntsecapi.h>
#include <lmcons.h>

#include "sailc/wapi/internal.hpp"
#include "sailc/common/utils.hpp"


namespace wapi = saildb::wapi;
namespace common = saildb::common;



/************************************************************
 *                                                          *
 *                          Utils                           *
 *                                                          *
 ************************************************************/
bool tryGetLocalIdentifier(LUID& pLuid, std::string& errorMessage) {
  DWORD tkLen;
  TOKEN_STATISTICS tkStats;
  ZeroMemory(&tkStats, sizeof(tkStats));

  bool ret = false;
  HANDLE hnd = NULL;

  auto closure = common::OnScopeExit([&]() { CloseHandle(hnd); });
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hnd)) {
    if (GetTokenInformation(hnd, TokenStatistics, &tkStats, sizeof(TOKEN_STATISTICS), &tkLen)) {
      ret = !ret;
      pLuid = tkStats.AuthenticationId;
    }

    CloseHandle(hnd);
  }

  if (!ret) {
    DWORD errorCode = GetLastError();
    errorMessage = wapi::internal::getErrorMessage(errorCode);
  }

  return ret;
}



/************************************************************
 *                                                          *
 *                     Users & Sessions                     *
 *                                                          *
 ************************************************************/

bool wapi::tryGetUsername(std::string& username, std::string& errorMessage) {
  WCHAR buf[UNLEN];
  DWORD len = UNLEN + 1;
  if (!GetUserNameW(buf, &len)) {
    DWORD errorCode = GetLastError();
    errorMessage = wapi::internal::getErrorMessage(errorCode);
    return false;
  }

  username = common::wstr2str(buf);
  return true;
}

bool wapi::tryGetDomainStatus(bool& isDomain, std::string& errorMessage) {
  LSA_HANDLE pHnd;
  isDomain = false;

  LSA_OBJECT_ATTRIBUTES attributes;
  ZeroMemory(&attributes, sizeof(attributes));

  auto hndDel = common::OnScopeExit([&]() { LsaClose(pHnd); });

  NTSTATUS status = LsaOpenPolicy(NULL, &attributes, POLICY_VIEW_LOCAL_INFORMATION, &pHnd);
  if (LSA_SUCCESS(status)) {
    PPOLICY_PRIMARY_DOMAIN_INFO dInfo;
    ZeroMemory(&dInfo, sizeof(dInfo));

    auto infoDel = common::OnScopeExit([&]() { LsaFreeMemory(dInfo); });
    status = LsaQueryInformationPolicy(pHnd, PolicyPrimaryDomainInformation, (LPVOID*)&dInfo);
    if (LSA_SUCCESS(status)) {
      isDomain = (dInfo != NULL);
    }
  }

  if (!LSA_SUCCESS(status)) {
    DWORD errorCode = wapi::internal::convertNtStatusToWin32Error(status);
    errorMessage = wapi::internal::getErrorMessage(errorCode);
    return false;
  }

  return true;
}

bool wapi::tryGetUsernameAndDomain(std::string& username, std::string& domain, std::string& errorMessage) {
  if (!tryGetUsername(username, errorMessage)) {
    return false;
  }

  LSA_HANDLE pHnd;
  LSA_OBJECT_ATTRIBUTES attributes;
  ZeroMemory(&attributes, sizeof(attributes));

  auto hndDel = common::OnScopeExit([&]() { LsaClose(pHnd); });
  bool isDomain = false;
  NTSTATUS status = LsaOpenPolicy(NULL, &attributes, POLICY_VIEW_LOCAL_INFORMATION, &pHnd);

  if (LSA_SUCCESS(status)) {
    PPOLICY_DNS_DOMAIN_INFO dInfo;
    ZeroMemory(&dInfo, sizeof(dInfo));

    auto infoDel = common::OnScopeExit([&]() { LsaFreeMemory(dInfo); });
    status = LsaQueryInformationPolicy(pHnd, PolicyDnsDomainInformation, (LPVOID*)&dInfo);
    if (LSA_SUCCESS(status)) {
      if (dInfo->Sid) {
        isDomain = true;
        domain = common::lsastr2str(dInfo->DnsDomainName.Buffer, dInfo->DnsDomainName.Length);
      } else {
        PPOLICY_ACCOUNT_DOMAIN_INFO aInfo;
        ZeroMemory(&aInfo, sizeof(aInfo));

        auto accDel = common::OnScopeExit([&]() { LsaFreeMemory(aInfo); });
        status = LsaQueryInformationPolicy(pHnd, PolicyAccountDomainInformation, (LPVOID*)&aInfo);
        if (LSA_SUCCESS(status)) {
          isDomain = false;
          domain = common::lsastr2str(aInfo->DomainName.Buffer, aInfo->DomainName.Length);
        }
      }
    }
  }

  if (!LSA_SUCCESS(status)) {
    DWORD errorCode = wapi::internal::convertNtStatusToWin32Error(status);
    errorMessage = wapi::internal::getErrorMessage(errorCode);
    return false;
  }

  return true;
}

bool wapi::tryGetPasswordChangedTimepoint(std::chrono::system_clock::time_point& timepoint, std::string& errorMessage) {
  LUID pLuid;
  ZeroMemory(&pLuid, sizeof(pLuid));

  NTSTATUS status = NULL;
  errorMessage.clear();

  bool success = false;
  if (::tryGetLocalIdentifier(pLuid, errorMessage)) {
    PSECURITY_LOGON_SESSION_DATA lsData;
    ZeroMemory(&lsData, sizeof(lsData));

    auto dtDel = common::OnScopeExit([&]() { LsaFreeReturnBuffer(lsData); });
    status = LsaGetLogonSessionData(&pLuid, &lsData);
    if (LSA_SUCCESS(status)) {
      try {
        timepoint = wapi::internal::filetime2timepoint(lsData->PasswordLastSet.HighPart, lsData->PasswordLastSet.LowPart);
        success = true;
      }
      catch (const std::exception& err) {
        errorMessage = err.what();
      }
    }
  }

  if (errorMessage.empty() && !LSA_SUCCESS(status)) {
    DWORD errorCode = GetLastError();
    errorMessage = wapi::internal::getErrorMessage(errorCode);
  }

  return success;
}

bool wapi::tryGetSessionInfo(common::Session& session, std::string& errorMessage) {
  LUID pLuid;
  ZeroMemory(&pLuid, sizeof(pLuid));

  NTSTATUS status = NULL;
  errorMessage.clear();

  if (::tryGetLocalIdentifier(pLuid, errorMessage)) {
    PSECURITY_LOGON_SESSION_DATA lsData;
    ZeroMemory(&lsData, sizeof(lsData));

    auto dtDel = common::OnScopeExit([&]() { LsaFreeReturnBuffer(lsData); });
    status = LsaGetLogonSessionData(&pLuid, &lsData);
    if (LSA_SUCCESS(status)) {
      try {
        session.username = common::lsastr2str(lsData->UserName.Buffer, lsData->UserName.Length);
        session.domainName = common::lsastr2str(lsData->LogonDomain.Buffer, lsData->LogonDomain.Length);
        session.lastSet = wapi::internal::filetime2timepoint(lsData->PasswordLastSet.HighPart, lsData->PasswordLastSet.LowPart);
        return true;
      }
      catch (const std::exception& err) {
        errorMessage = err.what();
      }
    }
  }

  if (errorMessage.empty() && !LSA_SUCCESS(status)) {
    DWORD errorCode = GetLastError();
    errorMessage = wapi::internal::getErrorMessage(errorCode);
  }

  return false;
}
