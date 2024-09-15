load('@rules_foreign_cc//foreign_cc:defs.bzl', 'cmake')

package(default_visibility = ['//visibility:public'])

filegroup(
  name = 'nanodbc_src',
  srcs = glob(['**']),
)

cmake(
  name = 'nanodbc',
  cache_entries = {
    'CMAKE_BUILD_TYPE': 'Release',
    'BUILD_SHARED_LIBS': 'OFF',
    'NANODBC_DISABLE_ASYNC': 'OFF',
    'NANODBC_DISABLE_EXAMPLES': 'ON',
    'NANODBC_DISABLE_INSTALL': 'OFF',
    'NANODBC_DISABLE_LIBCXX': 'ON',
    'NANODBC_DISABLE_TESTS': 'ON',
    'NANODBC_ENABLE_BOOST': 'OFF',
    'NANODBC_ENABLE_UNICODE': 'ON',
    'NANODBC_ENABLE_WORKAROUND_NODATA': 'OFF',
    'NANODBC_OVERALLOCATE_CHAR': 'OFF',
  },
  install = True,
  lib_source = '//:nanodbc_src',
  out_static_libs = ['nanodbc.lib'],
  linkopts = ['ODBC32.lib'],
  defines = ['NANODBC_ENABLE_UNICODE'],
)
