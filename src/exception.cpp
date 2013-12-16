//===------------------------ exception.cpp -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <stdlib.h>
#include <stdio.h>

#include "exception"
#include "new"

#ifndef __has_include
#define __has_include(inc) 0
#endif

// darwin8: there is no __cxxabiapple namespace anywhere
#if defined(__APPLE__) && !defined(DARWIN8_LIBSUPCXX)
  #include <cxxabi.h>

  using namespace __cxxabiv1;
  #define HAVE_DEPENDENT_EH_ABI 1
  #ifndef _LIBCPPABI_VERSION
    using namespace __cxxabiapple;
    // On Darwin, there are two STL shared libraries and a lower level ABI
    // shared library.  The globals holding the current terminate handler and
    // current unexpected handler are in the ABI library.
    #define __terminate_handler  __cxxabiapple::__cxa_terminate_handler
    #define __unexpected_handler __cxxabiapple::__cxa_unexpected_handler
  #endif  // _LIBCPPABI_VERSION
#elif defined(LIBCXXRT) || __has_include(<cxxabi.h>)
  #include <cxxabi.h>
  #if defined(DARWIN8_LIBSUPCXX)
  namespace __cxxabiv1 {
    extern std::terminate_handler  __terminate_handler;
    extern std::unexpected_handler __unexpected_handler;
  }
  #endif
  using namespace __cxxabiv1;
  #if defined(LIBCXXRT) || defined(_LIBCPPABI_VERSION)
    #define HAVE_DEPENDENT_EH_ABI 1
  #endif
#elif !defined(__GLIBCXX__) // __has_include(<cxxabi.h>)
  static std::terminate_handler  __terminate_handler;
  static std::unexpected_handler __unexpected_handler;
#endif // __has_include(<cxxabi.h>)

#if defined(__APPLE__) && defined(DARWIN8_LIBSUPCXX)
// should clang provide -DPIC
#define	PIC	1
#include <bits/os_defines.h>
// see https://www.opensource.apple.com/source/libstdcxx/libstdcxx-39/libstdcxx/libstdc++-v3/libsupc++/eh_terminate.cc
#endif

namespace std
{

#define	__PRED__	!defined(LIBCXXRT) && !defined(_LIBCPPABI_VERSION) && !defined(__GLIBCXX__)
#if __PRED__

// libcxxrt provides implementations of these functions itself.
unexpected_handler
set_unexpected(unexpected_handler func) _NOEXCEPT
{
    return __sync_lock_test_and_set(&__unexpected_handler, func);
}
#endif
#if __PRED__ || defined(DARWIN8_LIBSUPCXX)
unexpected_handler
get_unexpected() _NOEXCEPT
{
#if defined(__APPLE__) && defined(__ppc__) && defined(PIC)
    const std::unexpected_handler
	old(reinterpret_cast<std::unexpected_handler>(
		_keymgr_get_per_thread_data(KEYMGR_UNEXPECTED_HANDLER_KEY)));
    return old ? old : __unexpected_handler;
#else
    return __sync_fetch_and_add(&__unexpected_handler, (unexpected_handler)0);
#endif /* __APPLE__ etc. */
}
#endif
#if __PRED__
_LIBCPP_NORETURN
void
unexpected()
{
    (*get_unexpected())();
    // unexpected handler should not return
    terminate();
}

terminate_handler
set_terminate(terminate_handler func) _NOEXCEPT
{
    return __sync_lock_test_and_set(&__terminate_handler, func);
}

#endif
#if __PRED__ || defined(DARWIN8_LIBSUPCXX)
terminate_handler
get_terminate() _NOEXCEPT
{
// FIXME: see https://www.opensource.apple.com/source/libstdcxx/libstdcxx-39/libstdcxx/libstdc++-v3/libsupc++/eh_terminate.cc
#if defined(__APPLE__) && defined(__ppc__) && defined(PIC)
    const std::terminate_handler
	old(reinterpret_cast<std::terminate_handler>(
		_keymgr_get_per_thread_data(KEYMGR_TERMINATE_HANDLER_KEY)));
    return old ? old : __terminate_handler;
#else
    return __sync_fetch_and_add(&__terminate_handler, (terminate_handler)0);
#endif
}
#endif
#if __PRED__
#ifndef __EMSCRIPTEN__ // We provide this in JS
_LIBCPP_NORETURN
void
terminate() _NOEXCEPT
{
#ifndef _LIBCPP_NO_EXCEPTIONS
    try
    {
#endif  // _LIBCPP_NO_EXCEPTIONS
        (*get_terminate())();
        // handler should not return
        printf("terminate_handler unexpectedly returned\n");
        ::abort();
#ifndef _LIBCPP_NO_EXCEPTIONS
    }
    catch (...)
    {
        // handler should not throw exception
        printf("terminate_handler unexpectedly threw an exception\n");
        ::abort();
    }
#endif  // _LIBCPP_NO_EXCEPTIONS
}
#endif // !__EMSCRIPTEN__
#endif // !defined(LIBCXXRT) && !defined(_LIBCPPABI_VERSION)
#undef	__PRED__

#if !defined(LIBCXXRT) && !defined(__GLIBCXX__) && !defined(__EMSCRIPTEN__)
#if !defined(DARWIN8_LIBSUPCXX)
bool uncaught_exception() _NOEXCEPT
{
#if defined(__APPLE__) || defined(_LIBCPPABI_VERSION)
    // on Darwin, there is a helper function so __cxa_get_globals is private
    return __cxa_uncaught_exception();
#else  // __APPLE__
#   if defined(_MSC_VER) && ! defined(__clang__)
        _LIBCPP_WARNING("uncaught_exception not yet implemented")
#   else
#       warning uncaught_exception not yet implemented
#   endif
    printf("uncaught_exception not yet implemented\n");
    ::abort();
#endif  // __APPLE__
}
#endif


#ifndef _LIBCPPABI_VERSION

exception::~exception() _NOEXCEPT
{
}

const char* exception::what() const _NOEXCEPT
{
  return "std::exception";
}

#endif  // _LIBCPPABI_VERSION
#endif //LIBCXXRT
#if !defined(_LIBCPPABI_VERSION) && !defined(__GLIBCXX__)

bad_exception::~bad_exception() _NOEXCEPT
{
}
#endif
#if !defined(_LIBCPPABI_VERSION) && !defined(__GLIBCXX__) || defined(DARWIN8_LIBSUPCXX)
const char* bad_exception::what() const _NOEXCEPT
{
  return "std::bad_exception";
}

#endif

#define	GLIBCXX_HAS___EXCEPTION_PTR		!defined(DARWIN8_LIBSUPCXX)

#if defined(__GLIBCXX__) && GLIBCXX_HAS___EXCEPTION_PTR

// libsupc++ does not implement the dependent EH ABI and the functionality
// it uses to implement std::exception_ptr (which it declares as an alias of
// std::__exception_ptr::exception_ptr) is not directly exported to clients. So
// we have little choice but to hijack std::__exception_ptr::exception_ptr's
// (which fortunately has the same layout as our std::exception_ptr) copy
// constructor, assignment operator and destructor (which are part of its
// stable ABI), and its rethrow_exception(std::__exception_ptr::exception_ptr)
// function.

namespace __exception_ptr
{

struct exception_ptr
{
    void* __ptr_;

    exception_ptr(const exception_ptr&) _NOEXCEPT;
    exception_ptr& operator=(const exception_ptr&) _NOEXCEPT;
    ~exception_ptr() _NOEXCEPT;
};

}

_LIBCPP_NORETURN void rethrow_exception(__exception_ptr::exception_ptr);

#endif

exception_ptr::~exception_ptr() _NOEXCEPT
{
#if HAVE_DEPENDENT_EH_ABI
    __cxa_decrement_exception_refcount(__ptr_);
#elif defined(__GLIBCXX__) && GLIBCXX_HAS___EXCEPTION_PTR
    reinterpret_cast<__exception_ptr::exception_ptr*>(this)->~exception_ptr();
#else
#   if defined(_MSC_VER) && ! defined(__clang__)
        _LIBCPP_WARNING("exception_ptr not yet implemented")
#   else
#       warning exception_ptr not yet implemented
#   endif
    printf("exception_ptr not yet implemented\n");
    ::abort();
#endif
}

exception_ptr::exception_ptr(const exception_ptr& other) _NOEXCEPT
    : __ptr_(other.__ptr_)
{
#if HAVE_DEPENDENT_EH_ABI
    __cxa_increment_exception_refcount(__ptr_);
#elif defined(__GLIBCXX__) && GLIBCXX_HAS___EXCEPTION_PTR
    new (reinterpret_cast<void*>(this)) __exception_ptr::exception_ptr(
        reinterpret_cast<const __exception_ptr::exception_ptr&>(other));
#else
#   if defined(_MSC_VER) && ! defined(__clang__)
        _LIBCPP_WARNING("exception_ptr not yet implemented")
#   else
#       warning exception_ptr not yet implemented
#   endif
    printf("exception_ptr not yet implemented\n");
    ::abort();
#endif
}

exception_ptr& exception_ptr::operator=(const exception_ptr& other) _NOEXCEPT
{
#if HAVE_DEPENDENT_EH_ABI
    if (__ptr_ != other.__ptr_)
    {
        __cxa_increment_exception_refcount(other.__ptr_);
        __cxa_decrement_exception_refcount(__ptr_);
        __ptr_ = other.__ptr_;
    }
    return *this;
#elif defined(__GLIBCXX__) && GLIBCXX_HAS___EXCEPTION_PTR
    *reinterpret_cast<__exception_ptr::exception_ptr*>(this) =
        reinterpret_cast<const __exception_ptr::exception_ptr&>(other);
    return *this;
#else
#   if defined(_MSC_VER) && ! defined(__clang__)
        _LIBCPP_WARNING("exception_ptr not yet implemented")
#   else
#       warning exception_ptr not yet implemented
#   endif
    printf("exception_ptr not yet implemented\n");
    ::abort();
#endif
}

nested_exception::nested_exception() _NOEXCEPT
    : __ptr_(current_exception())
{
}

#if !defined(__GLIBCXX__) || defined(DARWIN8_LIBSUPCXX)

nested_exception::~nested_exception() _NOEXCEPT
{
}

#endif

_LIBCPP_NORETURN
void
nested_exception::rethrow_nested() const
{
    if (__ptr_ == nullptr)
        terminate();
    rethrow_exception(__ptr_);
}

#if !defined(__GLIBCXX__) || defined(DARWIN8_LIBSUPCXX)

exception_ptr current_exception() _NOEXCEPT
{
#if HAVE_DEPENDENT_EH_ABI
    // be nicer if there was a constructor that took a ptr, then
    // this whole function would be just:
    //    return exception_ptr(__cxa_current_primary_exception());
    exception_ptr ptr;
    ptr.__ptr_ = __cxa_current_primary_exception();
    return ptr;
#else
#   if defined(_MSC_VER) && ! defined(__clang__)
        _LIBCPP_WARNING( "exception_ptr not yet implemented" )
#   else
#       warning exception_ptr not yet implemented
#   endif
    printf("exception_ptr not yet implemented\n");
    ::abort();
#endif
}

#endif  // !__GLIBCXX__

_LIBCPP_NORETURN
void rethrow_exception(exception_ptr p)
{
#if HAVE_DEPENDENT_EH_ABI
    __cxa_rethrow_primary_exception(p.__ptr_);
    // if p.__ptr_ is NULL, above returns so we terminate
    terminate();
#elif defined(__GLIBCXX__) && GLIBCXX_HAS___EXCEPTION_PTR
    rethrow_exception(reinterpret_cast<__exception_ptr::exception_ptr&>(p));
#else
#   if defined(_MSC_VER) && ! defined(__clang__)
        _LIBCPP_WARNING("exception_ptr not yet implemented")
#   else
#       warning exception_ptr not yet implemented
#   endif
    printf("exception_ptr not yet implemented\n");
    ::abort();
#endif
}
} // std
