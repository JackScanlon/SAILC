"""Pybind smoke test via Bazel"""

from __future__ import annotations

import saildb

if __name__ == '__main__':
  print(f'Version: {saildb.__version__}')

  some_dot_env: str = saildb.try_dot_env("valueWithMixedInterpValues")
  print(some_dot_env)
