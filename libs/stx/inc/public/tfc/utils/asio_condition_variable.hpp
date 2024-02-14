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
#include <boost/sam/basic_condition_variable.hpp>
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP
PRAGMA_CLANG_WARNING_POP

namespace tfc::asio {

template <typename Executor = boost::asio::any_io_executor>
struct basic_condition_variable : boost::sam::basic_condition_variable<Executor> {
  using boost::sam::basic_condition_variable<Executor>::basic_condition_variable;
};

using condition_variable = basic_condition_variable<>;

}  // namespace tfc::asio
