#include "Environment.hpp"

#include <nanodbc/nanodbc.h>
#include <nlohmann/json.hpp>

#include "sailc/wapi/wapi.hpp"
#include "sailc/common/cstring.hpp"

namespace wapi = saildb::wapi;
namespace common = saildb::common;

saildb::Environment::Environment(std::string& pkgname, common::Session& usesh)
  : m_serviceName(std::move(pkgname)), m_session(std::move(usesh)) { };

saildb::Environment::~Environment() = default;

std::shared_ptr<saildb::Environment> saildb::Environment::Create(std::string pkgname) {
  common::Session usesh;

  // TODO
  // -> Derive session from WAPI

  return std::shared_ptr<saildb::Environment>(new saildb::Environment(pkgname, usesh));
}
