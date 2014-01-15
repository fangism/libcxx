//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// traps

#include <limits>

template <class T, bool expected>
void
test()
{
    static_assert(std::numeric_limits<T>::traps == expected, "traps test 1");
    static_assert(std::numeric_limits<const T>::traps == expected, "traps test 2");
    static_assert(std::numeric_limits<volatile T>::traps == expected, "traps test 3");
    static_assert(std::numeric_limits<const volatile T>::traps == expected, "traps test 4");
}

// see <limits>:
#if	(defined(__i386__) || defined(__x86_64__))
static const bool is_x86 = true;
#else
static const bool is_x86 = false;
#endif

int main()
{
    test<bool, false>();
    test<char, is_x86>();
    test<signed char, is_x86>();
    test<unsigned char, is_x86>();
    test<wchar_t, is_x86>();
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t, is_x86>();
    test<char32_t, is_x86>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short, is_x86>();
    test<unsigned short, is_x86>();
    test<int, is_x86>();
    test<unsigned int, is_x86>();
    test<long, is_x86>();
    test<unsigned long, is_x86>();
    test<long long, is_x86>();
    test<unsigned long long, is_x86>();
    test<float, false>();
    test<double, false>();
    test<long double, false>();
}
