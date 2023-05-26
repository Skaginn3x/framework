#include <boost/asio.hpp>
#include <boost/ut.hpp>
#include <gmock/gmock.h>

#include <tfc/progbase.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/mocks/confman/file_storage.hpp>
#include <tfc/utils/pragmas.hpp>

PRAGMA_CLANG_WARNING_PUSH_OFF(-Wkeyword-macro)
#define private public
PRAGMA_CLANG_WARNING_POP
#include <tfc/confman.hpp>

namespace ut = boost::ut;
namespace asio = boost::asio;

using ut::operator""_test;
using tfc::confman::observable;

struct storage {
  observable<int> a{ 33 };
  observable<int> b{ 44 };
  observable<std::string> c{ "c" };
};

template <>
struct glz::meta<storage> {
  static constexpr auto value{ glz::object("a", &storage::a, "b", &storage::b, "c", &storage::c) };
};

struct config_test {
  asio::io_context ctx{};
  tfc::confman::config<storage, testing::NiceMock<tfc::confman::mock_file_storage<storage>>> const config{ ctx, "foo" };
};

auto main(int argc, char** argv) -> int {
  tfc::base::init(argc, argv);

  "set changed"_test = [] {
    config_test const test{};
    auto presumed_err{ std::make_error_code(std::errc::bad_message) };
    ON_CALL(test.config.storage_, set_changed()).WillByDefault(testing::Return(presumed_err));
    EXPECT_CALL(test.config.storage_, set_changed()).Times(2); // todo why twice?
    auto returned_err{ test.config.set_changed() };
    ut::expect(returned_err == presumed_err);
  };


  return static_cast<int>(boost::ut::cfg<>.run({ .report_errors = true }));
}
