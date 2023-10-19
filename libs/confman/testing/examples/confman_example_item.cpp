#include <fmt/core.h>
#include <mp-units/systems/si/si.h>
#include <boost/asio.hpp>

#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/ipc/details/item_glaze_meta.hpp>
#include <tfc/ipc/item.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/units_glaze_meta.hpp>

namespace asio = boost::asio;

int main(int argc, char** argv) {
  tfc::base::init(argc,
                  argv);

  asio::io_context ctx{};

  tfc::confman::config<tfc::confman::observable<tfc::ipc::item::item>> const config{ ctx, "key" };
  config->observe([](auto new_value, auto old_value) {
    fmt::print("new value: {}, old value: {}\n", new_value.to_json(), old_value.to_json());
  });

  fmt::print("Schema is: {}\n", config.schema());
  fmt::print("Config is: {}\n", config.string());

  ctx.run();
  return 0;
}
