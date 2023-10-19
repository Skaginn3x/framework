#pragma once

namespace tfc::motor::types {
class ethercat_motor {
private:
  struct config {
    using impl = ethercat_motor;
    uint16_t slave_id;
    struct glaze {
      using T = config;
      static constexpr auto value = glz::object("slave_id", &T::slave_id);
      static constexpr std::string_view name{ "ethercat_motor" };
    };
    auto operator==(const config&) const noexcept -> bool = default;
  };

public:
  using config_t = config;
  explicit ethercat_motor(boost::asio::io_context&, const config&) {}
  void pump() {}
};
}  // namespace tfc::motor::types