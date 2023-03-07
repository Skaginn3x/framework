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

namespace tfc::rpc {


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

using method_base = glz::json_t;
using method_signature = std::function<const method_base& (std::expected<method_base, response_error_code_e>)>;

    // Check that parameters are available on the request
// and that their types/values conform to the standard.
auto validate_request(request_t const & request) -> std::expected<bool, std::string> {
  if (request.jsonrpc != "2.0"){
    return std::unexpected<std::string>("Invalid request - jsonrpc != 2.0");
  }

  // method -> string not starting with rpc
  if (request.method.starts_with("rpc")){
    return std::unexpected<std::string>("Invalid request - method starts with rpc");
  }

 // std::optional<glz::json_t> params;
 // iterator = object_ptr->find("params");
 // if (iterator != object_ptr->end()) {
 //   const auto* const param_object_ptr = iterator->second.get_if<glz::json_t::object_t>();
 //   const auto* const param_array_ptr = iterator->second.get_if<glz::json_t::array_t>();
 //   if (param_object_ptr == nullptr && param_array_ptr == nullptr) { // Parameters should be named parameters
 //     return std::unexpected<std::string>("Invalid request - params is not a structured type");
 //   }
 //   params = iterator->second;
 //   // TODO: Check internal types of the map
 //   // TODO: Check internal types of array
 // }

 // std::optional<std::variant<double, std::string, glz::json_t::null_t>> id; //NOLINT
 // iterator = object_ptr->find("id");
 // if (iterator != object_ptr->end()) {
 //   // Check if id is a string, double or a null.
 //   const bool hold_double = iterator->second.holds<double>();
 //   if (!iterator->second.holds<std::string>() &&  !iterator->second.holds<glz::json_t::null_t>() && !hold_double) {
 //     return std::unexpected<std::string>("Invalid request - id is neither string, double nor null");
 //   }
 //   if (hold_double){
 //     // id SHOULD NOT contain fractional parts.
 //     const auto* const double_ptr = iterator->second.get_if<double>();
 //     if (double_ptr == nullptr) {
 //       return std::unexpected<std::string>("Invalid request - id is null");
 //     }
 //     double int_part;
 //     if (std::modf(*double_ptr, &int_part) != 0){
 //       return std::unexpected<std::string>("Invalid request - id is not integral");
 //     }
 //     id = *double_ptr;
 //   } else if (const auto* string_ptr = iterator->second.get_if<std::string>()){
 //     id = *string_ptr;
 //   } else {
 //     id = nullptr;
 //   }
 // }

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
static auto handle_request(const std::string& rqst_string, const std::unordered_map<std::string, std::function<std::optional<glz::json_t>(std::optional<glz::json_t>)>>& methods = {}) -> std::string {
  if (auto error_code = glz::validate_json(rqst_string)) {
    std::string buffer;
    std::string const descriptive_error = glz::format_error(error_code, buffer);
    return build_string_error(descriptive_error, response_error_code_e::parse_error);
  }

  // Parse the string as a generic type.
  // That is used to determine if this is a batch request or
  // a single request.
  std::variant<request_t, std::vector<request_t>> request_object{};
  std::cout << "'" << rqst_string << "'" << std::endl;
  if (auto error_code = glz::read_json<std::variant<request_t, std::vector<request_t>>>(request_object, rqst_string)) {
    std::string buffer;
    std::string const descriptive_error = glz::format_error(error_code, buffer);
    return build_string_error(descriptive_error, response_error_code_e::parse_error);
  }

  // If request is a single request. place it into an array and take same path.
  const bool is_single_request = std::holds_alternative<request_t>(request_object);
  std::vector<request_t> requests;
  if (is_single_request) {
    requests.emplace_back(std::get<request_t>(request_object));
  } else if (const auto* requests_ptr = std::get_if<std::vector<request_t>>(&request_object)) {
    requests.assign(requests_ptr->begin(), requests_ptr->end());
    requests = *requests_ptr;
  } else {
    return build_string_error("This really should not happen at this stage", response_error_code_e::parse_error);
  }

  // Check if the array is empty
  if (requests.empty()) {
    return build_string_error("Invalid Request", response_error_code_e::invalid_request);
  }
  // Deal with each request
  std::vector<response_t> responses;
  for (auto const& rqst : requests) {
    // Validate the request
    if (auto request_valid = validate_request(rqst)) {
      auto method_itr = methods.find(rqst.method);
      if (method_itr == methods.end()) {
        return build_string_error("Method not found", response_error_code_e::method_not_found);
      }
      try {
        auto result = method_itr->second.operator()(rqst.params);
        responses.emplace_back(response_t{"2.0", result, {}, rqst.id});  // NOLINT
      } catch (...) {
        return build_string_error("Internal error", response_error_code_e::internal_error);
      }
    } else {
      responses.emplace_back(build_error(request_valid.error(), response_error_code_e::invalid_request));
    }
  }
  return is_single_request ? glz::write_json(responses[0]) : glz::write_json(responses);
  }
};  // namespace tfc::rpc

