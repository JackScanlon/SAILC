#pragma once

#include <memory>

namespace saildb {
namespace common {

template <typename Fn>
class OnScopeExitImpl final {
  public:
    explicit OnScopeExitImpl(Fn&& fn) : m_fn(std::move(fn)), m_active(true) { }

    ~OnScopeExitImpl() {
      if (m_active) {
        m_fn();
      }
    }

    OnScopeExitImpl(OnScopeExitImpl&& other)
      : m_fn(std::move(other.m_fn)), m_active(other.m_active)
    {
      other.m_active = false;
    }

    OnScopeExitImpl(const OnScopeExitImpl& other) = delete;
    OnScopeExitImpl& operator=(const OnScopeExitImpl& other) = delete;

  public:
    Fn m_fn;
    bool m_active;
};

template <typename Fn>
inline _Check_return_ auto OnScopeExit(Fn&& fn) -> OnScopeExitImpl<Fn> {
  return OnScopeExitImpl<Fn>{ std::move(fn) };
}

} // namespace common
} // namespace saildb
