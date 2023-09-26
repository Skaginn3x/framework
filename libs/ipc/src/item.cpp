
#include <fmt/format.h>
#include <glaze/json.hpp>
#include <pcg_extras.hpp>
#include <pcg_random.hpp>

#include <tfc/ipc/details/item_glaze_meta.hpp>
#include <tfc/ipc/item.hpp>

namespace tfc::ipc::item {

auto make(pcg_extras::seed_seq_from<std::random_device>& seed_source) -> item {
  pcg64 random_engine(seed_source);  // result_type uint64_t
  uuids::basic_uuid_random_generator<pcg64> random_generator{ random_engine };
  return { .item_id = random_generator(), .entry_timestamp = std::chrono::system_clock::now() };
}

auto make() -> item {
  pcg_extras::seed_seq_from<std::random_device> seed_source;
  return make(seed_source);
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
