
#include <cstdint>
#include <string_view>
#include <tfc/stx/string_view_join.hpp>
#include <tfc/stx/basic_fixed_string.hpp>

namespace tfc::operation {

enum struct mode_e : std::uint8_t {
  unknown = 0,
  stopped = 1,
  running = 2,
  specialized_running_1 = 3,
  specialized_running_2 = 4,
  specialized_running_3 = 5,
  fault = 6,
  cleaning = 7,
  emergency = 8,
  maintenance = 9,
};

namespace sender {
static constexpr std::string_view update{ "org.tfc.operation_mode.update" };
}

struct update_message {
  mode_e new_mode{mode_e::unknown};
  mode_e old_mode{mode_e::unknown};
};

}  // namespace tfc::operation
