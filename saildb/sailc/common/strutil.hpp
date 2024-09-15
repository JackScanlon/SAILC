#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <sstream>
#include <utility>
#include <functional>

namespace saildb {
namespace common {

template <typename T, typename... Args>
[[nodiscard]] auto concatTo(Args&&... args) -> T {
	static_assert((std::is_convertible_v<T, std::string> || std::is_convertible_v<T, std::wstring>));

	if constexpr(std::is_convertible_v<T, std::string>) {
		std::ostringstream stream;
		(stream << ... << std::forward<Args>(args));
		return stream.str();
	} else if constexpr(std::is_convertible_v<T, std::wstring>) {
		std::wostringstream stream;
		(stream << ... << std::forward<Args>(args));
		return stream.str();
	}
}

template <typename T>
class notWhitespace : public std::function<bool(T)> {
  static constexpr const T wschars[] = { T(' '), T('\t'), T('\n'), T('\f'), T('\v'), T('\r') };

  public:
    notWhitespace() { }

    bool operator()(T c) {
      return std::end(wschars) == std::find(std::begin(wschars), std::end(wschars), c);
    }
};

template <typename CharT, typename TraitsT, typename AllocT>
inline auto trimLeft(std::basic_string<CharT, TraitsT, AllocT>& input) -> uint32_t {
  typedef std::basic_string<CharT, TraitsT, AllocT> strType;
  typedef typename strType::iterator iterType;

  const iterType it = std::find_if(input.begin(), input.end(), notWhitespace<CharT>());
  uint32_t distance = std::distance(input.begin(), it);
  input.erase(input.begin(), it);

  return distance;
}

template <typename CharT, typename TraitsT, typename AllocT>
inline auto trimRight(std::basic_string<CharT, TraitsT, AllocT>& input) -> uint32_t {
  typedef std::basic_string<CharT, TraitsT, AllocT> strType;
  typedef typename strType::reverse_iterator iterType;

  const iterType it = std::find_if(input.rbegin(), input.rend(), notWhitespace<CharT>());
  uint32_t distance = std::distance(it.base(), input.end());
  input.erase(it.base(), input.end());

  return distance;
}

template <typename CharT, typename TraitsT, typename AllocT>
inline auto trim(std::basic_string<CharT, TraitsT, AllocT>& input) -> void {
  typedef std::basic_string<CharT, TraitsT, AllocT> strType;

  static constexpr CharT wschars[] = {
     CharT(' '), CharT('\t'), CharT('\n'),
    CharT('\f'), CharT('\v'), CharT('\r')
  };

  const auto first = input.find_first_not_of(wschars);
  if (first == strType::npos) {
    input.clear();
    return;
  }

  const auto last = input.find_last_not_of(wschars);
  input = input.substr(first, last - first + 1);
}

} // namespace common
} // namespace saildb
