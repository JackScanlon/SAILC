"""TODO: Setup pkg"""
from __future__ import annotations

import os
import posixpath
import re
import shutil
import sys
import setuptools

from distutils import sysconfig
from setuptools.command import build_ext

PKG_ROOT = os.path.dirname(os.path.abspath(__file__))
PKG_NAME = 'saildb'

def get_pkg_version(pkg: str) -> str:
  """Attempt to read the pkg version from the package"""
  with open(os.path.join(PKG_ROOT, pkg, '__init__.py')) as f:
    try:
      version = next(line for line in f if line.startswith('__version__'))
      version = version.split('=')
      version = version[1].strip().strip('\'')
    except StopIteration:
      raise ValueError('__init__.py of package %s does not define __version__' % (pkg,))
    except Exception as err:
      raise RuntimeError('Failed to parse version from %s/__init__.py with err: %s' % (pkg, str(err)))
    else:
      return version


class PkgExtension(setuptools.Extension):
  """Def ext"""

  def __init__(self, target: str):
    self.target = target
    self.relpath, self.target_name = (posixpath.relpath(target, '//').split(':'))
    ext_name = os.path.join(self.relpath.replace(posixpath.sep, os.path.sep), self.target_name)
    setuptools.Extension.__init__(self, ext_name, sources=[])


class BuildPkgExtension(build_ext.build_ext):
  """Build Pkg"""

  def run(self) -> None:
    for ext in self.extensions:
      self.build(ext)
    build_ext.build_ext.run(self)

  def build(self, ext: setuptools.Extension) -> None:
    if not os.path.exists(self.build_temp):
      os.makedirs(self.build_temp)

    bazel_argv = [
      'bazel',
      'build',
      ext.target,
      '--symlink_prefix=' + os.path.join(self.build_temp, 'bazel-'),
      '--compilation_mode=' + ('dbg' if self.debug else 'opt'),
    ]

    for library_dir in self.library_dirs:
      bazel_argv.append('--linkopt=/LIBPATH:' + library_dir)

    self.spawn(bazel_argv)

    ext_bazel_bin_path = os.path.join(
      self.build_temp, 'bazel-bin',
      ext.relpath, ext.target_name + '.pyd'
    )

    ext_dest_path = self.get_ext_fullpath(ext.name)
    ext_dest_dir = os.path.dirname(ext_dest_path)
    if not os.path.exists(ext_dest_dir):
      os.makedirs(ext_dest_dir)

    shutil.copyfile(ext_bazel_bin_path, ext_dest_path)


setuptools.setup(
  name=PKG_NAME,
  version=get_pkg_version(PKG_NAME),
  url='https://github.com/test/test',
  description='Test library',
  author='test',
  author_email='test@test.com',
  long_description=open(os.path.join(PKG_ROOT, 'README.md')).read(),
  long_description_content_type='text/markdown',
  packages=setuptools.find_packages(),
  cmdclass=dict(build_ext=BuildPkgExtension),
  ext_modules=[PkgExtension('//saildb:_core')],
  zip_safe=False,
  classifiers=[],
  license='MIT',
  keywords='test',
)
