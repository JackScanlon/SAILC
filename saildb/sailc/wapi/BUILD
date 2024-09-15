licenses(['notice'])
exports_files(['LICENSE'])

package(default_visibility = ['//saildb:__subpackages__'])

cc_library(
  name = 'internal',
  srcs = ['internal.cpp'],
  hdrs = ['internal.hpp'],
  deps = [
    '//saildb/sailc/common:cstring',
    '//saildb/sailc/common:constants',
  ],
  include_prefix = 'sailc/wapi',
  visibility = ['//visibility:private']
)

cc_library(
  name = 'wapi',
  srcs = ['secrets.cpp', 'lsa.cpp', 'sys.cpp'],
  hdrs = ['wapi.hpp'],
  deps = [
    ':internal',
    '//saildb/sailc/common:data',
    '//saildb/sailc/common:utils',
    '//saildb/sailc/common:typing',
    '//saildb/sailc/common:cstring',
    '//saildb/sailc/common:strutil',
    '//saildb/sailc/common:constants',
  ],
  linkopts = [
    'advapi32.lib',
    'Secur32.lib',
    'netapi32.lib',
  ],
  include_prefix = 'sailc/wapi',
)
