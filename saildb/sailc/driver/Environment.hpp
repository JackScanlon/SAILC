#pragma once

#include "sailc/common/data.hpp"

#include <list>
#include <string>
#include <vector>
#include <memory>

namespace common = saildb::common;

namespace saildb {

// TODO
class Environment : public std::enable_shared_from_this<Environment> {
  public:
    static std::shared_ptr<Environment> Create(std::string pkgname);

  public:
    Environment(Environment const&) = delete;
    Environment &operator=(Environment const&) = delete;
    virtual ~Environment();

  protected:
    Environment(std::string& pkgname, common::Session& usesh);

};

} // namespace saildb
