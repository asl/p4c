/*
Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/* -*-c++-*- */

#ifndef LIB_STRINGIFY_H_
#define LIB_STRINGIFY_H_

#include <cstdarg>
#include <string>
#include <string_view>
#include <type_traits>

// FIXME: Replace with big_int_fwd.h with Boost 1.84+
#include "big_int.h"
#include "cstring.h"

namespace P4 {

class IHasDbPrint {
 public:
    virtual void dbprint(std::ostream &out) const = 0;
    void print() const;  // useful in the debugger
    virtual ~IHasDbPrint() = default;
};

inline std::ostream &operator<<(std::ostream &out, const IHasDbPrint &obj) {
    obj.dbprint(out);
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const IHasDbPrint *obj) {
    if (obj)
        obj->dbprint(out);
    else
        out << "<null>";
    return out;
}

/// SFINAE helper to check if given class has a `dbprint` method. Apparently,
/// not everything are descendants of IHasDbPrint...
template <class, class = void>
struct has_dbprint : std::false_type {};

template <class T>
struct has_dbprint<T,
                   std::void_t<decltype(std::declval<T>().dbprint(std::declval<std::ostream &>()))>>
    : std::true_type {};

template <class T>
inline constexpr bool has_dbprint_v = has_dbprint<T>::value;

template <class, class = void>
struct has_ostream_operator : std::false_type {};

template <class T>
struct has_ostream_operator<
    T, std::void_t<decltype(std::declval<std::ostream &>() << std::declval<T>())>>
    : std::true_type {};

template <class T>
inline constexpr bool has_ostream_operator_v = has_ostream_operator<T>::value;

// convert values to cstrings
namespace Util {

/// SFINAE helper to check if given class has a `toString` method.
template <class, class = void>
struct has_toString : std::false_type {};

template <class T>
struct has_toString<T, std::void_t<decltype(std::declval<T>().toString())>> : std::true_type {};

template <class T>
inline constexpr bool has_toString_v = has_toString<T>::value;

template <typename T, typename = decltype(std::to_string(std::declval<T>()))>
cstring toString(T value) {
    return cstring(std::to_string(value));
}

template <typename T>
auto toString(const T &value) -> typename std::enable_if_t<has_toString_v<T>, cstring> {
    return value.toString();
}

template <typename T>
auto toString(T &value) -> typename std::enable_if_t<has_toString_v<T>, cstring> {
    return value.toString();
}

template <typename T>
auto toString(const T *value) -> typename std::enable_if_t<has_toString_v<T>, cstring> {
    return value->toString();
}

template <typename T>
auto toString(T *value) -> typename std::enable_if_t<has_toString_v<T>, cstring> {
    return value->toString();
}

cstring toString(bool value);
cstring toString(const std::string &value);
cstring toString(const char *value);
cstring toString(cstring value);
cstring toString(std::string_view value);
/// A width of zero indicates that no width should be displayed.
cstring toString(const big_int &value, unsigned width, bool sign, unsigned int base = 10);
cstring toString(const void *value);

char DigitToChar(int digit);

}  // namespace Util
}  // namespace P4

#endif /* LIB_STRINGIFY_H_ */
