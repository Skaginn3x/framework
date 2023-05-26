#pragma once
#include <concepts>
#include <filesystem>

#include <gmock/gmock.h>
#include <boost/asio/io_context.hpp>

#include <tfc/confman/file_storage.hpp>

namespace tfc::confman {

namespace asio = boost::asio;

template <typename storage_t>
struct mock_file_storage : public tfc::confman::file_storage<storage_t> {
  using type = storage_t;
  using change = detail::change<mock_file_storage>;

  mock_file_storage(asio::io_context& ctx, std::filesystem::path const&) : tfc::confman::file_storage<storage_t>{ ctx } {
    ON_CALL(*this, error()).WillByDefault(testing::ReturnRef(this->error_));
    ON_CALL(*this, file()).WillByDefault(testing::ReturnRef(this->config_file_));
    ON_CALL(*this, value()).WillByDefault(testing::ReturnRef(this->storage_));
    ON_CALL(*this, access()).WillByDefault(testing::ReturnRef(this->storage_));
    ON_CALL(*this, make_change()).WillByDefault(testing::Return(change{ *this }));
    ON_CALL(*this, set_changed()).WillByDefault(testing::Return(std::error_code{}));
  }
  mock_file_storage(asio::io_context& ctx, std::filesystem::path const& path, [[maybe_unused]] auto&& default_value)
      : mock_file_storage{ ctx, path } {
    this->storage_ = std::forward<decltype(default_value)>(default_value);
  }

  MOCK_METHOD((std::error_code const&), error, (), (const noexcept));       // NOLINT
  MOCK_METHOD((std::filesystem::path const&), file, (), (const noexcept));  // NOLINT
  MOCK_METHOD((storage_t const&), value, (), (const noexcept));             // NOLINT
  MOCK_METHOD((storage_t&), access, (), (const noexcept));                  // NOLINT
  MOCK_METHOD((void), on_change, (std::function<void()>), ());              // NOLINT, todo
  MOCK_METHOD((change), make_change, (), (noexcept));                       // NOLINT
  MOCK_METHOD((std::error_code), set_changed, (), (const noexcept));        // NOLINT
};

}  // namespace tfc::confman
