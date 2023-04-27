#include <tfc/operation_mode.hpp>

#include <sdbusplus/sdbus.hpp>

namespace tfc::operation {

interface::interface(): logger_("operation") {}
interface::interface(std::string_view log_key): logger_(log_key) {}

void interface::set(tfc::operation::mode_e) const {


}

}
