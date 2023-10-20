#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <config/writeable_signals.hpp>
#include <tfc/ipc/enums.hpp>

namespace tfc::mqtt::config {

namespace asio = boost::asio;
using tfc::ipc::details::type_e;

struct writeable_signals_owner_mock {
  std::vector<signal_definition> writeable_signals{};
};

class writeable_signals_mock {
  writeable_signals_owner_mock owner_;

public:
  writeable_signals_mock(asio::io_context const&, std::string const&) {
    const signal_definition writeable_signal{ "test_signal", "", type_e::_bool };
    owner_.writeable_signals = { writeable_signal };
  }

  [[nodiscard]] auto value() const -> writeable_signals_owner_mock { return owner_; }
};

}  // namespace tfc::mqtt::config
