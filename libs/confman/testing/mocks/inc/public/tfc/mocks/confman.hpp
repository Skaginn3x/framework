#pragma once
#include <concepts>
#include <filesystem>
#include <string_view>

#include <gmock/gmock.h>

#include <tfc/confman.hpp>
#include <tfc/dbus/sdbusplus_fwd.hpp>
#include <tfc/mocks/confman/detail/config_dbus_client.hpp>
#include <tfc/mocks/confman/file_storage.hpp>

namespace tfc::confman {

namespace asio = boost::asio;

template <typename storage_t, typename = void, typename = void>
struct mock_config : public config<storage_t, mock_file_storage<storage_t>, detail::mock_config_dbus_client> {
  using type = storage_t;
  using change = detail::change<mock_config>;

  mock_config(std::shared_ptr<sdbusplus::asio::connection> conn, std::string_view key)
      : config<storage_t, mock_file_storage<storage_t>, detail::mock_config_dbus_client>{ conn, key } {
    ON_CALL(*this, value()).WillByDefault(::testing::ReturnRef(this->storage_.value()));
    ON_CALL(*this, access()).WillByDefault(::testing::ReturnRef(this->storage_.access()));
    ON_CALL(*this, string()).WillByDefault(::testing::Return(""));
    ON_CALL(*this, schema()).WillByDefault(::testing::Return(""));
    ON_CALL(*this, file()).WillByDefault(::testing::ReturnRef(this->storage_.file()));
    ON_CALL(*this, make_change()).WillByDefault(::testing::Return(change{ *this }));
    ON_CALL(*this, from_string(::testing::_)).WillByDefault(::testing::Return(std::error_code{}));
  }
  mock_config(std::shared_ptr<sdbusplus::asio::connection> conn, std::string_view key, auto&& default_value)
      : mock_config{ conn, key } {
    this->storage_ = mock_file_storage<storage_t>{ std::forward<decltype(default_value)>(default_value) };
  }

  MOCK_METHOD((storage_t const&), value, (), (const noexcept));                             // NOLINT
  auto operator->() const noexcept -> storage_t const* { return std::addressof(value()); }  // NOLINT
  MOCK_METHOD((storage_t&), access, (), (const noexcept));                                  // NOLINT
  MOCK_METHOD((std::string), string, (), (const));
  MOCK_METHOD((std::string), schema, (), (const));
  MOCK_METHOD((std::filesystem::path const&), file, (), (const noexcept));  // NOLINT
  MOCK_METHOD((change), make_change, (), (noexcept));                       // NOLINT
  MOCK_METHOD((std::error_code), from_string, (std::string_view), ());      // NOLINT, todo
};

}  // namespace tfc::confman
