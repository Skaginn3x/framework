#include <tfc/ec/devices/beckhoff/EL1xxx_impl.hpp>
#include <tfc/ec/devices/beckhoff/EL2xxx_impl.hpp>
#include <tfc/ipc/details/dbus_client_iface.hpp>

namespace tfc::ec::devices::beckhoff {

using imc = tfc::ipc_ruler::ipc_manager_client;

template class el1xxx<imc, el1002<imc>::size_v, el1002<imc>::entries_v, el1002<imc>::product_code, el1002<imc>::name>;
template class el1xxx<imc, el1008<imc>::size_v, el1008<imc>::entries_v, el1008<imc>::product_code, el1008<imc>::name>;
template class el1xxx<imc, el1809<imc>::size_v, el1809<imc>::entries_v, el1809<imc>::product_code, el1809<imc>::name>;

template class el2xxx<imc, el2794<imc>::size_v, el2794<imc>::entries_v, el2794<imc>::product_code, el2794<imc>::name>;
template class el2xxx<imc, el2004<imc>::size_v, el2004<imc>::entries_v, el2004<imc>::product_code, el2004<imc>::name>;
template class el2xxx<imc, el2008<imc>::size_v, el2008<imc>::entries_v, el2008<imc>::product_code, el2008<imc>::name>;
template class el2xxx<imc, el2809<imc>::size_v, el2809<imc>::entries_v, el2809<imc>::product_code, el2809<imc>::name>;

}  // namespace tfc::ec::devices::beckhoff
