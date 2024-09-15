#pragma once

#include <chrono>
#include <vector>
#include <string>

namespace saildb {
namespace common {

#pragma region session_decl

struct Session {
  std::chrono::system_clock::time_point lastSet;
  std::string username;
  std::string domainName;

  Session();
  Session(
    std::string uid,
    std::string dname,
    std::chrono::system_clock::time_point timepoint
  );
  Session(Session&& other);
  Session(const Session& other);
  Session& operator=(Session& other);
  Session& operator=(const Session& other);
};

#pragma endregion


#pragma region secret_decl

struct Secret {
  std::chrono::system_clock::time_point lastSet;
  std::string target;
  std::string account;
  std::string datasource;
  std::string password;
  bool isUserAccount;

  Secret();
  Secret(
    std::string trg,
    std::string uid,
    std::string pwd,
    std::string dsn,
    bool isUA,
    std::chrono::system_clock::time_point timepoint
  );
  Secret(Secret&& other);
  Secret(const Secret& other);
  Secret& operator=(Secret& other);
  Secret& operator=(const Secret& other);
};

#pragma endregion


} // namespace common
} // namespace saildb
