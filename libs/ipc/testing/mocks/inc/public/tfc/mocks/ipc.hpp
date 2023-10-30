#pragma once
#include <functional>
#include <string_view>
#include <system_error>
#include <type_traits>

#include <gmock/gmock.h>
#include <boost/asio/io_context.hpp>

#include <tfc/stx/concepts.hpp>

namespace tfc::ipc {

namespace asio = boost::asio;

template <typename type_desc, typename manager_client_type>
struct mock_signal {
  using value_t = typename type_desc::value_t;

  mock_signal(asio::io_context const&, manager_client_type&, std::string_view, std::string_view = "") {}

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

  // clang-format off
  MOCK_METHOD((std::error_code), send, (value_t const&), (const));  // NOLINT
  MOCK_METHOD((void), async_send_cb, (value_t const&, std::function<void(std::error_code, std::size_t)>), (const));  // NOLINT
  // clang-format on
};

template <typename type_desc, typename manager_client_type>
struct mock_slot {
  using value_t = typename type_desc::value_t;
  mock_slot(asio::io_context const&,
            manager_client_type&,
            std::string_view,
            std::string_view,
            tfc::stx::invocable<value_t> auto&& cb): callback{ std::forward<decltype(cb)>(cb) } {
    ON_CALL(*this, value()).WillByDefault(testing::ReturnRef(value_));
  }
  mock_slot(asio::io_context const&, manager_client_type&, std::string_view, tfc::stx::invocable<value_t> auto&&) {}

  MOCK_METHOD((std::optional<value_t> const&), value, (), (const));  // NOLINT
  std::optional<value_t> value_{ std::nullopt };
  std::function<void(value_t const&)> callback;
};

}  // namespace tfc::ipc
