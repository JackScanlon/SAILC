load('@pybind11_bazel//:build_defs.bzl', 'pybind_extension')
load('@rules_python//python:defs.bzl', 'py_library', 'py_test')
load('@rules_python//python:packaging.bzl', 'py_package')
load('@bazel_skylib//rules:common_settings.bzl', 'string_flag')
load('defs.bzl', 'local_defines_flag')

licenses(['notice'])
exports_files(['LICENSE'])

package(default_visibility = ['//saildb:__subpackages__'])


# Ext
pybind_extension(
  name = '_core',
  srcs = ['//saildb/bindings:core'],
  deps = [
    '//saildb:PKG_VERSION',
    '//saildb/sailc/wapi:wapi',
    # '//saildb/sailc/common:data',
    # '//saildb/sailc/driver:environment',
    # '@com_github_nlohmann_json//:json',
  ],
)

py_library(
  name = 'saildb',
  srcs = [
    '__init__.py',
  ],
  srcs_version = 'PY3',
  imports = ['.'],
  data = [
    ':_core',
  ],
  visibility = ['//visibility:public'],
)


# Tests
py_test(
  name = 'basic_test',
  srcs = ['basic_test.py'],
  deps = [':saildb'],
  data = ['//resources:env_data']
)


# Flags
local_defines_flag(
  name = 'PKG_NAME',
  value = ':pkg_name',
)

local_defines_flag(
  name = 'PKG_VERSION',
  value = ':pkg_version',
)

string_flag(
  name = 'pkg_name',
  build_setting_default = 'Dev',
)

string_flag(
  name = 'pkg_version',
  build_setting_default = 'dev',
)
