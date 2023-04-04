//
// This compilation unit is only here to store vtable references.
//
#include "tfc/ec/devices/base.hpp"
#include "tfc/ec/devices/abt/easycat.hpp"
#include "tfc/ec/devices/beckhoff/EK1xxx.hpp"
#include "tfc/ec/devices/schneider/atv320.hpp"

tfc::ec::devices::base::~base() = default;
tfc::ec::devices::default_device::~default_device() = default;
tfc::ec::devices::schneider::atv320::~atv320() = default;
tfc::ec::devices::abt::easyecat::~easyecat() = default;
tfc::ec::devices::beckhoff::ek1100::~ek1100() = default;
