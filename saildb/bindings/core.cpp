#include <pybind11/pybind11.h>

#include <string>
#include <exception>
#include <filesystem>

#include "sailc/wapi/wapi.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace common = saildb::common;


std::string tryDotEnv(std::string keyname) {
  auto fp = std::filesystem::current_path();
  fp /= "resources/.saildb.env";

  saildb::wapi::DotEnv env(fp);

  auto value = env.Get<std::string>(keyname);
  return value;
}


PYBIND11_MODULE(_core, m) {
  #ifdef PKG_NAME
    std::string pkgname(MACRO_STRINGIFY(PKG_NAME));
  #else
    std::string pkgname("PKG");
  #endif

  #ifdef PKG_VERSION
    m.attr("__version__") = MACRO_STRINGIFY(PKG_VERSION);
  #else
    m.attr("__version__") = "dev";
  #endif

	m.doc() = "Some documentation";

	m.def("try_dot_env", &tryDotEnv, "Some method doc");
}
