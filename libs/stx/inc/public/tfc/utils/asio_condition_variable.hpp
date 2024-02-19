#pragma once

#include <tfc/utils/pragmas.hpp>

#define BOOST_SAM_HEADER_ONLY 1

// clang-format off
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wundef)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wnon-virtual-dtor)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wextra-semi-stmt)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wshadow)
PRAGMA_CLANG_WARNING_PUSH_OFF(-Wweak-vtables)
// clang-format on
#include <boost/sam/condition_variable.hpp>
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP

namespace tfc::asio {

using condition_variable = boost::sam::condition_variable;

}  // namespace tfc::asio
