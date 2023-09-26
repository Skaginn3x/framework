#include <random>

#include <fmt/format.h>
#include <glaze/json.hpp>

#include <tfc/ipc/details/item_glaze_meta.hpp>
#include <tfc/ipc/item.hpp>
#include <tfc/progbase.hpp>

namespace tfc::ipc::item {

item make() {
  // todo how about storing the random engine in shared memory?
  static std::mt19937_64 random_engine{};
  static bool first_call{ true };
  if (first_call) {
    first_call = false;
    auto proc_id{ std::hash<std::string>{}(fmt::format("{}{}", tfc::base::get_exe_name(), tfc::base::get_proc_name())) };
    random_engine.seed(proc_id);
  }
  uuids::basic_uuid_random_generator<std::mt19937_64> random_generator{ random_engine };

  return { .item_id = random_generator(), .entry_timestamp = std::chrono::system_clock::now() };
}
auto item::from_json(std::string_view json) -> std::expected<item, glz::parse_error> {
  return glz::read_json<item>(json);
}
auto item::to_json() const -> std::string {
  return glz::write_json(*this);
}
auto item::id() const -> std::string {
  if (item_id.has_value()) {
    return uuids::to_string(item_id.value());
  }
  return {};
}

}  // namespace tfc::ipc::item
