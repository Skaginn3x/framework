#pragma once

#include <tfc/utils/pragmas.hpp>

#define BOOST_SAM_HEADER_ONLY 1

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wundef)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wnon-virtual-dtor)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wextra-semi-stmt)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wshadow)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wweak-vtables)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wmissing-noreturn)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wreserved-identifier)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wdocumentation)
PRAGMA_GCC_WARNING_OFF(-Werror=unused-parameter)
// clang-format on
#include <boost/sam/mutex.hpp>
#include <boost/sam/guarded.hpp>
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_GCC_WARNING_POP

namespace tfc::asio {

using mutex = boost::sam::mutex;

}  // namespace tfc::asio
