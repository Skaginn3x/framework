#pragma once
#include <functional>
#include <string_view>
#include <system_error>
#include <type_traits>

#include <gmock/gmock.h>
#include <boost/asio/io_context.hpp>

namespace tfc::ipc {

namespace asio = boost::asio;

template <typename type_desc, typename manager_client_type>
struct mock_signal {
  using value_t = typename type_desc::value_t;

  mock_signal(asio::io_context& ctx, manager_client_type& client, std::string_view name, std::string_view description = "") {
  }

  // todo can this be done differently?
  template <typename completion_token_t>
  auto async_send(value_t const& value, completion_token_t&& token) -> auto {
    if constexpr (std::is_invocable_v<std::remove_cvref_t<completion_token_t>, std::error_code, std::size_t>) {
      async_send_cb(value, std::forward<completion_token_t>(token));
      return;
    } else {
      []<bool flag = false>() {
        static_assert(flag, "todo implement for other types");
      }
      ();
    }
  }

  MOCK_METHOD((std::error_code), send, (value_t const&), ());  // NOLINT
  MOCK_METHOD((std::error_code),
              async_send_cb,
              (value_t const&, std::function<void(std::error_code, std::size_t)>),
              ());  // NOLINT
};

}  // namespace tfc::ipc
