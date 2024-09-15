load('@bazel_skylib//rules:common_settings.bzl', 'BuildSettingInfo')

def _local_define_flag_impl(ctx):
  return CcInfo(
    compilation_context = cc_common.create_compilation_context(
      defines = depset([ '%s=%s' % (ctx.attr.name, ctx.attr.value[BuildSettingInfo].value,) ]),
    )
  )

local_defines_flag = rule(
  implementation = _local_define_flag_impl,
  attrs = {
    'value': attr.label()
  }
)
