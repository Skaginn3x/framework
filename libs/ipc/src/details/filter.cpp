#include "tfc/ipc/details/filter_impl.hpp"

namespace tfc::ipc::filter {

template <typename value_t>
using callback_t = std::function<void (value_t &)>;

template class filters<bool, callback_t<bool>>;
template class filters<std::uint64_t, callback_t<std::uint64_t>>;
template class filters<std::int64_t, callback_t<std::int64_t>>;
template class filters<double, callback_t<double>>;
template class filters<std::string, callback_t<std::string>>;

}  // namespace tfc::ipc::filter
