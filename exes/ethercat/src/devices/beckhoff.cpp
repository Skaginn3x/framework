#include <tfc/ec/devices/beckhoff/EL1xxx_impl.hpp>
#include <tfc/ec/devices/beckhoff/EL2xxx_impl.hpp>
#include <tfc/ipc/details/dbus_client_iface.hpp>

namespace tfc::ec::devices::beckhoff {

using imc = tfc::ipc_ruler::ipc_manager_client;

template class el1xxx<imc, el1002<imc>::size_v, el1002<imc>::product_code>;
template class el1xxx<imc, el1008<imc>::size_v, el1008<imc>::product_code>;
template class el1xxx<imc, el1809<imc>::size_v, el1809<imc>::product_code>;

template class el2xxx<imc, el2004<imc>::size_v, el2004<imc>::product_code, el2004<imc>::name_v>;
template class el2xxx<imc, el2008<imc>::size_v, el2008<imc>::product_code, el2008<imc>::name_v>;
template class el2xxx<imc, el2809<imc>::size_v, el2809<imc>::product_code, el2809<imc>::name_v>;

}  // namespace tfc::ec::devices::beckhoff
