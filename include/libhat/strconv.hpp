#pragma once

#include <string_view>
#include <type_traits>

#include "concepts.hpp"
#include "result.hpp"

namespace hat {

    enum class parse_int_error {
        invalid_base,
        illegal_char
    };

    template<std::integral Integer, detail::char_iterator Iter>
    LIBHAT_CONSTEXPR_RESULT result<Integer, parse_int_error> parse_int(Iter begin, Iter end, uint8_t base = 10) noexcept {
        if (base < 2 || base > 36) {
            return result_error{parse_int_error::invalid_base};
        }

        Integer sign = 1;
        Integer value = 0;
        const int digits = base < 10 ? base : 10;
        const int letters = base > 10 ? base - 10 : 0;

        for (auto iter = begin; iter != end; iter++) {
            const char ch = *iter;

            if constexpr (std::is_signed_v<Integer>) {
                if (iter == begin) {
                    if (ch == '+') {
                        continue;
                    } else if (ch == '-') {
                        sign = -1;
                        continue;
                    }
                }
            }

            value *= base;
            if (ch >= '0' && ch < '0' + digits) {
                value += static_cast<Integer>(ch - '0');
            } else if (ch >= 'A' && ch < 'A' + letters) {
                value += static_cast<Integer>(ch - 'A' + 10);
            } else if (ch >= 'a' && ch < 'a' + letters) {
                value += static_cast<Integer>(ch - 'a' + 10);
            } else {
                // Throws an exception at runtime AND prevents constexpr evaluation
                return result_error{parse_int_error::illegal_char};
            }
        }
        return sign * value;
    }

    template<typename Integer>
    LIBHAT_CONSTEXPR_RESULT result<Integer, parse_int_error> parse_int(std::string_view str, uint8_t base = 10) noexcept {
        return parse_int<Integer>(str.cbegin(), str.cend(), base);
    }
}
