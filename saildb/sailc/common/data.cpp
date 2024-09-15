#include "data.hpp"

#include <memory>
#include <utility>

namespace common = saildb::common;

// struct Session implementation
#pragma region session_impl

common::Session::Session() { };

common::Session::Session(std::string uid, std::string dname, std::chrono::system_clock::time_point timepoint)
  : username(std::move(uid)), domainName(std::move(dname)), lastSet(std::move(timepoint)) { };

common::Session::Session(common::Session&& other)
  : username(std::move(other.username)), domainName(std::move(other.domainName)), lastSet(std::move(other.lastSet)) { };

common::Session::Session(const common::Session& other) = default;

common::Session& common::Session::operator=(common::Session& other) = default;

common::Session& common::Session::operator=(const common::Session& other) = default;

#pragma endregion


// struct Secret implementation
#pragma region secret_impl

common::Secret::Secret() { };

common::Secret::Secret(std::string trg, std::string uid, std::string dsn, std::string pwd, bool isUA, std::chrono::system_clock::time_point timepoint)
  : target(std::move(trg)), account(std::move(uid)), datasource(std::move(dsn)),
    password(std::move(pwd)), isUserAccount(std::exchange(isUA, 0)), lastSet(std::move(timepoint)) { };

common::Secret::Secret(common::Secret&& other)
  : target(std::move(other.target)), account(std::move(other.account)), datasource(std::move(other.datasource)),
    password(std::move(other.password)), isUserAccount(std::exchange(other.isUserAccount, 0)), lastSet(std::move(other.lastSet)) { };

common::Secret::Secret(const common::Secret& other) = default;

common::Secret& common::Secret::operator=(common::Secret& other) = default;

common::Secret& common::Secret::operator=(const common::Secret& other) = default;

#pragma endregion
