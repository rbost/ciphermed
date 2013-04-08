#pragma once

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)
#define GCC_AT_LEAST_47 1
#else
#define GCC_AT_LEAST_47 0
#endif

// g++-4.6 does not support override
#if GCC_AT_LEAST_47
#define OVERRIDE override
#else
#define OVERRIDE
#endif

#ifdef NDEBUG
#define ALWAYS_ASSERT(expr) (likely(e) ? (void)0 : abort())
#else
#define ALWAYS_ASSERT(expr) assert(expr)
#endif /* NDEBUG */

#define NEVER_INLINE  __attribute__((noinline))
#define ALWAYS_INLINE __attribute__((always_inline))
#define UNUSED __attribute__((unused))
