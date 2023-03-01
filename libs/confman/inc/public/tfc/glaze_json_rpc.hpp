#pragma once

/*
 * Implementation of json-rpc based on the glaze
 * json library.
 * See https://www.jsonrpc.org/specification
 *
 * Underlying transport not specified as per specification.
 * */

#include <iostream>
#include <array>
#include <glaze/glaze.hpp>
#include <variant>

namespace tfc::confman::rpc {

struct request_t {
  std::string jsonrpc;
  std::string method;
  std::optional<glz::json_t> params;
  std::optional<std::variant<double, std::string, glz::json_t::null_t>> id;
  struct glaze {
    using T = request_t;
    static constexpr std::string_view name = "Request";
    static constexpr auto value =
        glz::object("jsonrpc", &T::jsonrpc, "method", &T::method, "params", &T::params, "id", &T::id);
  };
};

enum response_error_code_e : int32_t {
  parse_error = -32700,
  invalid_request = -32600,
  method_not_found = -32601,
  invalid_params = -32602,
  internal_error = -32603,
};

struct response_error_t {
  int32_t code;
  std::string message;
  glz::json_t data;
  struct glaze {
    using T = response_error_t;
    static constexpr std::string_view name = "Response error";
    static constexpr auto value = glz::object("code", &T::code, "message", &T::message, "data", &T::data);
  };
};
struct response_t {
  std::string jsonrpc;
  std::optional<glz::json_t> result;
  std::optional<response_error_t> error;
  std::optional<std::variant<double, std::string, glz::json_t::null_t>> id;
  struct glaze {
    using T = response_t;
    static constexpr std::string_view name = "Response";
    static constexpr auto value =
        glz::object("jsonrpc", &T::jsonrpc, "result", &T::result, "error", &T::error, "id", &T::id);
  };
};

// Check that parameters are available on the request
// and that their types/values conform to the standard.
auto valid_request(glz::json_t const &rqst) -> bool {
  const auto* const object_ptr = std::get_if<glz::json_t::object_t>(&rqst.data);
  if (object_ptr == nullptr){
    return false;
  }

  // jsonrpc -> string must be "2.0"
  auto iterator = object_ptr->find("jsonrpc");
  if (iterator == object_ptr->end()){
    return false;
  }
  const auto* string_ptr = std::get_if<std::string>(&iterator->second.data);
  if (string_ptr == nullptr){
    return false;
  }
  if (*string_ptr != "2.0"){
    return false;
  }

  // method -> string not starting with rpc
  iterator = object_ptr->find("method");
  if (iterator == object_ptr->end()){
    return false;
  }
  string_ptr = std::get_if<std::string>(&iterator->second.data);
  if (string_ptr == nullptr){
    return false;
  }
  if (string_ptr->starts_with("rpc")){
    return false;
  }

  // params -> structure, either array or object. This may be omitted.
  iterator = object_ptr->find("params");
  if (iterator != object_ptr->end()) {
    const auto* const param_object_ptr = std::get_if<glz::json_t::object_t>(&iterator->second.data);
    const auto* const param_array_ptr = std::get_if<glz::json_t::array_t>(&iterator->second.data);
    if (param_object_ptr == nullptr && param_array_ptr == nullptr) { // Parameters should be named parameters
      return false;
    }
    // TODO: Check internal types of the map
    // TODO: Check internal types of array
  }

  iterator = object_ptr->find("id");
  if (iterator != object_ptr->end()) {
    // Check if id is a string, double or a null.
    const bool hold_double = std::holds_alternative<double>(iterator->second.data);
    if (!std::holds_alternative<std::string>(iterator->second.data) &&  !std::holds_alternative<glz::json_t::null_t>(iterator->second.data) && !hold_double) {
      return false;
    }
    if (hold_double){
      // id SHOULD NOT contain fractional parts.
      const auto* const double_ptr = std::get_if<double>(&iterator->second.data);
      if (double_ptr == nullptr) {
        return false;
      }
      double int_part;
      if (std::modf(*double_ptr, &int_part) != 0){
        return false;
      }
    }
  }

  // id may be omitted
  return true;
}

static auto build_error(const std::string& message, const response_error_code_e& error) -> response_t{
  response_error_t resp_error{error, message, {}};
  return response_t {"2.0", {}, resp_error, nullptr};
}

static auto build_string_error(const std::string&  message, const response_error_code_e& error) -> std::string {
  return glz::write_json<response_t>(build_error(message, error));
}

// A request is either a single request or a batch request.
// This method processes the request and verifies it.
// After which the results are returned from the function again.
// It has no side effect.
static auto handle_request(std::string rqst_string) -> std::string {
  // Parse the string as a generic type.
  // That is used to determine if this is a batch request or
  // a single request.
  glz::json_t generic_parse;
  try {
    glz::read_json(generic_parse, rqst_string);
  } catch (std::runtime_error const &err) {
    return build_string_error(err.what(), response_error_code_e::parse_error);
  }
  // If request is a single request. place it into an array and take same path.
  const bool is_single_request = std::holds_alternative<glz::json_t::object_t>(generic_parse.data);
  if (is_single_request){
    generic_parse = glz::json_t({generic_parse});
  }
  // Check if we have a batch request or a single request.
  if (auto* array_ptr = std::get_if<glz::json_t::array_t>(&generic_parse.data)) {
    // Check if the array is empty
    if (array_ptr->empty()){
      return build_string_error("Invalid Request", response_error_code_e::invalid_request);
    }
    // Deal with each request
    std::vector<response_t> responses;
    for (auto const& rqst : *array_ptr){
      // Validate the request
      if(!valid_request(rqst)){
        //TODO: Exchange message for more detail when valid_request implements std::expected
        //Optional data field in jsonrpc can be utilized
        responses.emplace_back(build_error("Invalid Request", response_error_code_e::invalid_request));
      }
    }
    return is_single_request ? glz::write_json(responses[0]) : glz::write_json(responses);
  }
  return "";
}
};  // namespace tfc::confman::rpc
