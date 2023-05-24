#pragma once
#include <gmock/gmock.h>
#include <tfc/confman/file_storage.hpp>

namespace tfc::mocks::confman {

namespace asio = boost::asio;

template <typename storage_t>
class file_storage : public tfc::confman::file_storage<storage_t> {
public:
  using type = storage_t;
  file_storage(asio::io_context& ctx, std::filesystem::path const&) : tfc::confman::file_storage<storage_t>{ ctx } {
    ON_CALL(*this, error()).WillByDefault(testing::Return(this->error_));
  }
  file_storage(asio::io_context& ctx, std::filesystem::path const& path, [[maybe_unused]] auto&& default_value)
      : file_storage{ ctx, path } {}

  MOCK_METHOD((std::error_code const&), error, (), (const noexcept));       // NOLINT
  MOCK_METHOD((std::filesystem::path const&), file, (), (const noexcept));  // NOLINT
};

}  // namespace tfc::mocks::confman
