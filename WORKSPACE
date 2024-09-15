# Rules & Toolchains
load('@bazel_tools//tools/build_defs/repo:http.bzl', 'http_archive')
http_archive(
  name = 'bazel_features',
  sha256 = 'ba1282c1aa1d1fffdcf994ab32131d7c7551a9bc960fbf05f42d55a1b930cbfb',
  strip_prefix = 'bazel_features-1.15.0',
  url = 'https://github.com/bazel-contrib/bazel_features/releases/download/v1.15.0/bazel_features-v1.15.0.tar.gz',
)
load('@bazel_features//:deps.bzl', 'bazel_features_deps')
bazel_features_deps()

http_archive(
  name = 'rules_foreign_cc',
  sha256 = 'a2e6fb56e649c1ee79703e99aa0c9d13c6cc53c8d7a0cbb8797ab2888bbc99a3',
  strip_prefix = 'rules_foreign_cc-0.12.0',
  url = 'https://github.com/bazelbuild/rules_foreign_cc/releases/download/0.12.0/rules_foreign_cc-0.12.0.tar.gz',
)

load('@rules_foreign_cc//foreign_cc:repositories.bzl', 'rules_foreign_cc_dependencies')

rules_foreign_cc_dependencies()


# Third party
http_archive(
  name = 'com_github_nanodbc',
  build_file = '//third_party:nanodbc.BUILD',
  strip_prefix = 'nanodbc-2.14.0',
  urls = ['https://github.com/nanodbc/nanodbc/archive/refs/tags/v2.14.0.zip']
)

http_archive(
  name = 'com_github_nlohmann_json',
  build_file = '//third_party:json.BUILD',
  sha256 = '0d8ef5af7f9794e3263480193c491549b2ba6cc74bb018906202ada498a79406',
  strip_prefix = 'json-3.11.3',
  urls = ['https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.tar.gz']
)
