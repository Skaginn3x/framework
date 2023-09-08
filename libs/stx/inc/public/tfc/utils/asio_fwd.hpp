#pragma once

namespace boost::asio {
template <typename Executor>
class strand;
class any_io_executor;
template <typename return_t, typename executor_t>
class awaitable;
class io_context;
template <typename internet_protocol, typename executor = any_io_executor>
class basic_resolver;
namespace ip {
class tcp;
using resolve = basic_resolver<tcp>;
}  // namespace ip
}  // namespace boost::asio
