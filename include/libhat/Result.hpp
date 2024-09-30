#pragma once

#include <variant>

#if __cpp_lib_variant >= 202106L
    #define LIBHAT_CONSTEXPR_RESULT constexpr
#else
    #define LIBHAT_CONSTEXPR_RESULT inline
#endif

namespace hat {

    template<class E>
    class result_error {
        template<typename T, typename R>
        friend class result;
    public:
        LIBHAT_CONSTEXPR_RESULT explicit result_error(const E& t) : error(t) {}
        LIBHAT_CONSTEXPR_RESULT explicit result_error(E&& t) : error(std::move(t)) {}

        LIBHAT_CONSTEXPR_RESULT result_error(const result_error<E>&) = default;
        LIBHAT_CONSTEXPR_RESULT result_error(result_error<E>&&) noexcept = default;
    private:
        E error;
    };

    /// Highly simplified version of C++23's std::expected, using std::variant for the underlying implementation.
    template<class T, class E>
    class result {
    public:
        LIBHAT_CONSTEXPR_RESULT result(const T& t) : impl(std::in_place_index<0>, t) {}
        LIBHAT_CONSTEXPR_RESULT result(T&& t) : impl(std::in_place_index<0>, std::move(t)) {}

        LIBHAT_CONSTEXPR_RESULT result(const result_error<E>& e) : impl(std::in_place_index<1>, e.error) {}
        LIBHAT_CONSTEXPR_RESULT result(result_error<E>&& e) : impl(std::in_place_index<1>, std::move(e.error)) {}

        LIBHAT_CONSTEXPR_RESULT result(const result<T, E>&) = default;
        LIBHAT_CONSTEXPR_RESULT result(result<T, E>&&) noexcept = default;

        [[nodiscard]] LIBHAT_CONSTEXPR_RESULT bool has_value() const {
            return impl.index() == 0;
        }

        LIBHAT_CONSTEXPR_RESULT T& value() noexcept {
            return std::get<0>(impl);
        }

        LIBHAT_CONSTEXPR_RESULT const T& value() const noexcept {
            return std::get<0>(impl);
        }

        LIBHAT_CONSTEXPR_RESULT E& error() noexcept {
            return std::get<1>(impl);
        }

        LIBHAT_CONSTEXPR_RESULT const E& error() const noexcept {
            return std::get<1>(impl);
        }
    private:
        std::variant<T, E> impl;
    };
}
