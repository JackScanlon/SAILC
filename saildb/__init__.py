"""saildb interface"""

from __future__ import annotations


from ._core import (  # type:ignore # isort:skip
  __doc__,
  try_dot_env
)

__version__ = '0.0.1'

__all__ = ['__doc__', '__version__', 'try_dot_env']
