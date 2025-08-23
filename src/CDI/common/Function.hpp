/** \file Function.hpp
 * \brief Constexpr implementation of std::function.
 * Source:
 * - https://x.com/krisjusiak/status/1628383374833229827
 * - https://godbolt.org/z/Pq4bcKvPv
 * Modified to include a default constructor.
 */

#ifndef CDI_COMMON_FUNCTION_HPP
#define CDI_COMMON_FUNCTION_HPP

#include <memory>

/** \brief Constexpr implementation of std::function. */
template <class>
class Function;

template <class R, class... TArgs>
class Function<R(TArgs...)> {
    struct interface {
        constexpr virtual auto operator()(TArgs...) -> R = 0;
        constexpr virtual ~interface() = default;
    };

    template <class Fn>
    struct implementation final : interface {
        constexpr explicit implementation(Fn fn) : fn{fn} {}
        constexpr auto operator()(TArgs... args) -> R { return fn(args...); }

    private:
        Fn fn{};
    };

public:
    constexpr Function() {}
    constexpr Function(std::nullptr_t) {}

    template <class Fn>
    constexpr Function(Fn fn) : fn{std::make_unique<implementation<Fn>>(fn)} {}

    constexpr auto operator()(TArgs... args) const -> R {
        return (*fn)(args...);
    }

private:
    std::unique_ptr<interface> fn{};
};

template <class> struct function_traits {};

template <class R, class B, class... TArgs>
struct function_traits<R (B::*)(TArgs...) const> {
    using type = R(TArgs...);
};

#endif // CDI_COMMON_FUNCTION_HPP
