licenses(['notice'])
exports_files(['LICENSE'])

package(default_visibility = ['//saildb:__subpackages__'])

cc_library(
  name = 'typing',
  hdrs = ['typing.hpp'],
  include_prefix = 'sailc/common',
)

cc_library(
  name = 'data',
  srcs = ['data.cpp'],
  hdrs = ['data.hpp'],
  include_prefix = 'sailc/common',
)

cc_library(
  name = 'constants',
  hdrs = ['constants.hpp'],
  include_prefix = 'sailc/common',
)

cc_library(
  name = 'utils',
  hdrs = ['utils.hpp'],
  include_prefix = 'sailc/common',
)

cc_library(
  name = 'cstring',
  srcs = ['cstring.cpp'],
  hdrs = ['cstring.hpp'],
  deps = [':constants'],
  include_prefix = 'sailc/common',
)

cc_library(
  name = 'strutil',
  hdrs = ['strutil.hpp'],
  include_prefix = 'sailc/common',
)
