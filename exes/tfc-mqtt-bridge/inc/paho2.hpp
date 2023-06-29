#pragma once
#include <mqtt/async_client.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/unpack_properties.hpp>
#include <string>
#include <tfc/confman.hpp>
#include <tfc/confman/observable.hpp>
#include <tfc/dbus/string_maker.hpp>
#include <tfc/ipc.hpp>
#include <tfc/logger.hpp>
#include <tfc/stx/to_tuple.hpp>
#include <thread>
#include "impl.hpp"
#include "mqtt/async_client.h"

// using namespace std;
namespace asio = boost::asio;
namespace details = tfc::ipc::details;

struct config {
  tfc::confman::observable<std::vector<std::string>> _banned_topics{};
  struct glaze {
    static constexpr auto value{ glz::object("_banned_topics", &config::_banned_topics) };
    static constexpr auto name{ "mqtt_broadcaster" };
  };
};

const std::string SERVER_ADDRESS("tcp://localhost:1883");
const std::string CLIENT_ID("paho_cpp_async_subcribe");
const std::string TOPIC("#");

const int QOS = 1;
const int N_RETRY_ATTEMPTS = 5;

/////////////////////////////////////////////////////////////////////////////

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

template <class ipc_client_type, class match_client>
class mqtt_broadcaster {
public:
  mqtt_broadcaster(mqtt::async_client& mqtt_client) : mqtt_client_{ mqtt_client }, cb{*this} {

    mqtt::connect_options connOpts;
    connOpts.set_clean_session(false);


    mqtt_client_.set_callback(cb);

    MyActionListener listener;
    mqtt_client_.connect(connOpts, nullptr, listener);
  }

  void connected(const std::string& cause) {
    std::cout << "Connection success\n";
    std::cout << "\tcause: " << cause << std::endl;
  }

  void connection_lost(const std::string& cause) {
    std::cout << "\nConnection lost\n";
    if (!cause.empty())
      std::cout << "\tcause: " << cause << std::endl;
    std::cout << std::endl;
  }

  void message_arrived(mqtt::message::const_ptr_t msg) {
    std::cout << "Message arrived" << std::endl;
    std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
    std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
  }

  void delivery_complete(mqtt::delivery_token::ptr_t ptr) {
        std::cout << "Delivery complete for token: " << (ptr ? ptr->get_message_id() : -1) << std::endl;
  }

private:

  mqtt::async_client& mqtt_client_;
  MyCallback<ipc_client_type, match_client> cb;

};
