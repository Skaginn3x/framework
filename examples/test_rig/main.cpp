#include <string>
#include <string_view>

#include <boost/asio.hpp>
#include <glaze/glaze.hpp>

#include <tfc/confman.hpp>
#include <tfc/ipc.hpp>
#include <tfc/operation_mode.hpp>
#include <tfc/progbase.hpp>
#include <tfc/stx/glaze_meta.hpp>
#include <tfc/utils/json_schema.hpp>

namespace asio = boost::asio;

struct config_struct {
  std::chrono::milliseconds servo_cycle{};
  uint8_t count{};
};

template <>
struct glz::meta<config_struct> {
  static constexpr std::string_view name{ "test rig config" };
  static constexpr auto value{ glz::object(
      "servo_cycle",
      &config_struct::servo_cycle,
      tfc::json::schema{ .description = "Duration between moving the servo",
                         .default_value = 1000,
                         .minimum = 20,
                         .maximum = 5000 },
      "count",
      &config_struct::count,
      tfc::json::schema{ .description = "Servo wink count", .default_value = 10, .minimum = 1, .maximum = 180 }) };
};

struct app {
  app(asio::io_context& io) : ctx{ io } {
    // std::size_t idx{};
    // for (auto& slot : test) {
    //   slot = std::make_shared<tfc::ipc::uint_slot>(ctx, client, fmt::format("test_{}", idx++), "test", [](auto) {});
    // }
  }
  asio::io_context& ctx;
  tfc::confman::config<config_struct> config{ ctx, "test_rig_config",
                                              config_struct{ .servo_cycle = std::chrono::seconds{ 1 }, .count = 10 } };
  tfc::operation::interface operation_mode {
    ctx
  };
  tfc::logger::logger logger{ "app" };
  tfc::ipc_ruler::ipc_manager_client client{ ctx };
  tfc::ipc::uint_signal servo{ ctx, client, "servo", "Wink out" };
  tfc::ipc::uint_slot servo_manual{ ctx, client, "servo_manual", "Maintenance mode manual servo control",
                                    std::bind_front(&app::on_servo_manual, this) };
  std::array<std::shared_ptr<tfc::ipc::uint_slot>, 2096> test;
  asio::steady_timer run_servo_timer{ ctx };
  uint8_t servo_count{};
  bool maintenance_mode{ false };
  void on_run(tfc::operation::mode_e, tfc::operation::mode_e old_mode) {
    logger.trace("on_run, came from: {}", enum_name(old_mode));
    run_servo_timer.expires_after(config->servo_cycle);
    run_servo_timer.async_wait(std::bind_front(&app::run_servo_timer_cb, this));
  }
  void leave_run(tfc::operation::mode_e new_mode, tfc::operation::mode_e) {
    logger.trace("leave_run, going to: {}", enum_name(new_mode));
    run_servo_timer.cancel();
  }

  void on_servo_manual(std::uint64_t new_value) {
    if (maintenance_mode) {
      logger.trace("on_servo_manual: {}", new_value);
      servo.async_send(
          new_value, [this](auto err, auto) { this->logger.info("on_servo_manual: async_send failed: {}", err.message()); });
    } else {
      logger.trace("Won't respond to input, mode is: {}", "foo");
    }
  }

  void run_servo_timer_cb(std::error_code const& err) {
    if (err) {
      logger.error("run_servo_timer_cb: {}", err.message());
      return;
    }
    servo_count = (servo_count + config->count) % 180;
    logger.trace("Send to servo value: {}", servo_count);
    servo.send(servo_count);
    run_servo_timer.expires_after(config->servo_cycle);
    run_servo_timer.async_wait(std::bind_front(&app::run_servo_timer_cb, this));
  }

  void init() {
    operation_mode.on_enter(tfc::operation::mode_e::maintenance, [this](auto, auto) { maintenance_mode = true; });
    operation_mode.on_leave(tfc::operation::mode_e::maintenance, [this](auto, auto) { maintenance_mode = false; });

    operation_mode.on_enter(tfc::operation::mode_e::running, std::bind_front(&app::on_run, this));
    operation_mode.on_leave(tfc::operation::mode_e::running, std::bind_front(&app::leave_run, this));
  }
};

int main(int argc, char** argv) {
  tfc::base::init(argc, argv);

  asio::io_context ctx{};

  [[maybe_unused]] app my_app{  ctx };

  my_app.init();

  asio::co_spawn(ctx, tfc::base::exit_signals(ctx), asio::detached);

  ctx.run();

  return EXIT_SUCCESS;
}
