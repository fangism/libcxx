#ifndef	__LIBCXX_TEST_SUPPORT_POWERPC_DARWIN_H__
#define	__LIBCXX_TEST_SUPPORT_POWERPC_DARWIN_H__

// control certain workarounds for the powerpc-darwin8 port

/**
	On PowerPC-darwin, ptrdiff_t is int, but size_t is unsigned long.
	Tests that rely on is_same<make_unsigned<ptrdiff_t>, size_t>
	will fail, but as long as the underlying integer types are
	the same size (which is true on PPC32), then it is still safe.
 */
#define	PTRDIFF_T_VS_SIZE_T_DIFFER

/**
	PPC32 doesn't have native 64b atomic operations.
	Thus, they must be emulated using mutex or lock.
	TODO: provide specializations of struct atomic using std::mutex.
	TODO: implement in compiler front-end
	Undefine the following to enable mutex-based emulated 64b atomics.
 */
// #define	MISSING_64B_ATOMIC_OPS

#endif	// __LIBCXX_TEST_SUPPORT_POWERPC_DARWIN_H__
