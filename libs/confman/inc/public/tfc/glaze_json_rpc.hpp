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
#include <expected>

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
auto valid_request(glz::json_t const &rqst) -> std::expected<request_t, std::string> {
  const auto* const object_ptr = rqst.get_if<glz::json_t::object_t>();
  if (object_ptr == nullptr){
    return std::unexpected<std::string>("Invalid request");
  }

  // jsonrpc -> string must be "2.0"
  auto iterator = object_ptr->find("jsonrpc");
  if (iterator == object_ptr->end()){
    return std::unexpected<std::string>("Invalid request - jsonrpc not found");
  }
  const auto* const json_rpc_ptr = iterator->second.get_if<std::string>();
  if (json_rpc_ptr == nullptr){
    return std::unexpected<std::string>("Invalid request - jsonrpc is null");
  }
  if (*json_rpc_ptr != "2.0"){
    return std::unexpected<std::string>("Invalid request - jsonrpc != 2.0");
  }

  // method -> string not starting with rpc
  iterator = object_ptr->find("method");
  if (iterator == object_ptr->end()){
    return std::unexpected<std::string>("Invalid request - Method not found");
  }
  const auto* const method_ptr = iterator->second.get_if<std::string>();
  if (method_ptr == nullptr){
    return std::unexpected<std::string>("Invalid request - method is null");
  }
  if (method_ptr->starts_with("rpc")){
    return std::unexpected<std::string>("Invalid request - method starts with rpc");
  }

  // params -> structure, either array or object. This may be omitted.
  std::optional<glz::json_t> params;
  iterator = object_ptr->find("params");
  if (iterator != object_ptr->end()) {
    const auto* const param_object_ptr = iterator->second.get_if<glz::json_t::object_t>();
    const auto* const param_array_ptr = iterator->second.get_if<glz::json_t::array_t>();
    if (param_object_ptr == nullptr && param_array_ptr == nullptr) { // Parameters should be named parameters
      return std::unexpected<std::string>("Invalid request - params is not a structured type");
    }
    params = iterator->second;
    // TODO: Check internal types of the map
    // TODO: Check internal types of array
  }

  std::optional<std::variant<double, std::string, glz::json_t::null_t>> id; //NOLINT
  iterator = object_ptr->find("id");
  if (iterator != object_ptr->end()) {
    // Check if id is a string, double or a null.
    const bool hold_double = iterator->second.holds<double>();
    if (!iterator->second.holds<std::string>() &&  !iterator->second.holds<glz::json_t::null_t>() && !hold_double) {
      return std::unexpected<std::string>("Invalid request - id is neither string, double nor null");
    }
    if (hold_double){
      // id SHOULD NOT contain fractional parts.
      const auto* const double_ptr = iterator->second.get_if<double>();
      if (double_ptr == nullptr) {
        return std::unexpected<std::string>("Invalid request - id is null");
      }
      double int_part;
      if (std::modf(*double_ptr, &int_part) != 0){
        return std::unexpected<std::string>("Invalid request - id is not integral");
      }
      id = *double_ptr;
    } else if (const auto* string_ptr = iterator->second.get_if<std::string>()){
      id = *string_ptr;
    } else {
      id = nullptr;
    }
  }

  // Return a request object
  return request_t{*json_rpc_ptr, *method_ptr, params, id};
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
static auto handle_request(std::string rqst_string, const std::unordered_map<std::string, std::function<std::optional<glz::json_t>(std::optional<glz::json_t>)>>& methods = {}) -> std::string {
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
  const bool is_single_request = generic_parse.holds<glz::json_t::object_t>();
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
    for (auto const& raw_rqst: *array_ptr){
      // Validate the request
      if(auto validated_rqst = valid_request(raw_rqst)){
        auto method_itr = methods.find(validated_rqst->method);
        if (method_itr == methods.end()) {
          return build_string_error("Method not found", response_error_code_e::method_not_found);
        }
        try{
          auto result = method_itr->second.operator()(validated_rqst->params);
          responses.emplace_back(response_t{"2.0", result, {}, validated_rqst->id}); //NOLINT
        } catch (...){
          return build_string_error("Internal error", response_error_code_e::internal_error);
        }
      } else {
        responses.emplace_back(build_error(validated_rqst.error(), response_error_code_e::invalid_request));
      }
    }
    return is_single_request ? glz::write_json(responses[0]) : glz::write_json(responses);
  }
  return "";
}
};  // namespace tfc::confman::rpc
