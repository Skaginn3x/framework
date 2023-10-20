#include <string_view>

namespace tfc::mqtt::constants {

using std::string_view_literals::operator""sv;

static constexpr auto namespace_element{ "spBv1.0"sv };
static constexpr auto ndata{ "NDATA"sv };
static constexpr auto nbirth{ "NBIRTH"sv };
static constexpr auto ndeath{ "NDEATH"sv };
static constexpr auto ncmd{ "NCMD"sv };
static constexpr auto rebirth_metric{ "Node Control/Rebirth"sv };

}  // namespace tfc::mqtt::constants
