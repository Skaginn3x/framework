#pragma once

// PRAGMA_GCC_WARNING_PUSH
// PRAGMA_GCC_WARNING_POP
// PRAGMA_GCC_WARNING_OFF(-Wwarning_name)
// PRAGMA_GCC_WARNING_ON(-Wwarning_name)
// PRAGMA_GCC_WARNING_PUSH_OFF (works as PUSH + OFF needs POP)

// PRAGMA_CLANG_WARNING_PUSH
// PRAGMA_CLANG_WARNING_POP
// PRAGMA_CLANG_WARNING_ON(-Wwarning_name)
// PRAGMA_CLANG_WARNING_OFF(-Wwarning_name)
// PRAGMA_CLANG_WARNING_PUSH_OFF(-Wwarning_name) (works as PUSH + OFF needs POP)

#if defined(__GNUC__) && !defined(__clang__)
#define GCC_DO_PRAGMA(x) _Pragma(#x)

// diagnostic[ warning error ignored]
#define PRAGMA_GCC_WARNING_OFF(name) GCC_DO_PRAGMA(GCC diagnostic ignored #name)
#define PRAGMA_GCC_WARNING_ON(name) GCC_DO_PRAGMA(GCC diagnostic warning #name)
#define PRAGMA_GCC_WARNING_PUSH GCC_DO_PRAGMA(GCC diagnostic push)
#define PRAGMA_GCC_WARNING_POP GCC_DO_PRAGMA(GCC diagnostic pop)
#define PRAGMA_GCC_WARNING_PUSH_OFF(name) PRAGMA_GCC_WARNING_PUSH PRAGMA_GCC_WARNING_OFF(name)
#else
#define PRAGMA_GCC_WARNING_PUSH
#define PRAGMA_GCC_WARNING_POP
#define PRAGMA_GCC_WARNING_ON(name)
#define PRAGMA_GCC_WARNING_OFF(name)
#define PRAGMA_GCC_WARNING_PUSH_OFF(name)
#endif

#if defined(__clang__)
#define CLANG_DO_PRAGMA(x) _Pragma(#x)
#define PRAGMA_CLANG_WARNING_OFF(name) CLANG_DO_PRAGMA(clang diagnostic ignored #name)
#define PRAGMA_CLANG_WARNING_ON(name) CLANG_DO_PRAGMA(clang diagnostic warning #name)
#define PRAGMA_CLANG_WARNING_PUSH CLANG_DO_PRAGMA(clang diagnostic push)
#define PRAGMA_CLANG_WARNING_POP CLANG_DO_PRAGMA(clang diagnostic pop)
#define PRAGMA_CLANG_WARNING_PUSH_OFF(name) PRAGMA_CLANG_WARNING_PUSH PRAGMA_CLANG_WARNING_OFF(name)
#else
#define PRAGMA_CLANG_WARNING_PUSH
#define PRAGMA_CLANG_WARNING_POP
#define PRAGMA_CLANG_WARNING_ON(name)
#define PRAGMA_CLANG_WARNING_OFF(name)
#define PRAGMA_CLANG_WARNING_PUSH_OFF(name)
#endif
