// #pragma once
//
// #include <cstdint>
// #include <string>
// #include <variant>
//
// #include <config/broker.hpp>
// #include <structs.hpp>
//
// namespace tfc::mqtt::config {
//
// namespace asio = boost::asio;
//
// struct broker_owner_mock {
//   std::string address{};
//   std::variant<tfc::mqtt::config::port_e, uint16_t> port{};
//   tfc::mqtt::structs::ssl_active_e ssl_active{};
//   std::string username{};
//   std::string password{};
//   std::string client_id{};
//
//   static auto get_port() -> std::string { return "1883"; }
// };
//
// class broker_mock {
//   broker_owner_mock owner_;
//
// public:
//   broker_mock(asio::io_context const&, std::string const&) {
//     owner_.address = "localhost";
//     owner_.port = static_cast<uint16_t>(1883);
//     owner_.ssl_active = tfc::mqtt::structs::ssl_active_e::no;
//     owner_.username = "username";
//     owner_.password = "password";
//     owner_.client_id = "client_id";
//   }
//
//   [[nodiscard]] auto value() const -> broker_owner_mock { return owner_; }
// };
// }  // namespace tfc::mqtt::config
//