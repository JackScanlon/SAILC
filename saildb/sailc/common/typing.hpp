#include <string_view>

namespace saildb {
namespace common {

// Implementation derived from: https://stackoverflow.com/a/66551751
template <typename T>
constexpr auto getRawTypeName() -> std::string_view {
	return __FUNCSIG__;
}

struct TypeNameInfo {
	static constexpr auto typeProbe = std::string_view("double");
	static constexpr auto sentinelFn = getRawTypeName<double>();
	static constexpr size_t prefixOffset = sentinelFn.find(typeProbe);
	static constexpr size_t suffixOffset = sentinelFn.size() - prefixOffset - typeProbe.size();
};

template <typename T>
constexpr auto getTypeName() -> std::string_view {
	std::string_view raw = getRawTypeName<T>();
	return raw.substr(
		TypeNameInfo::prefixOffset,
		(raw.size() - TypeNameInfo::suffixOffset) - TypeNameInfo::prefixOffset
	);
}

} // namespace common
} // namespace saildb
