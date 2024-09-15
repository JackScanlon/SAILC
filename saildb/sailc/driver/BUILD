licenses(['notice'])
exports_files(['LICENSE'])

package(default_visibility = ['//saildb:__subpackages__'])

cc_library(
  name = 'environment',
  srcs = ['Environment.cpp'],
  hdrs = ['Environment.hpp'],
  deps = [
    '//saildb/sailc/common:data',
    '//saildb/sailc/common:cstring',
    '//saildb/sailc/wapi:wapi',
    '@com_github_nanodbc//:nanodbc',
  ],
  include_prefix = 'sailc/driver',
)
