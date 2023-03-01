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
  PARSE_ERROR = -32700,
  INVALID_REQUEST = -32600,
  METHOD_NOT_FOUND = -32601,
  INVALID_PARAMS = -32602,
  INTERNAL_ERROR = -32603,
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
static bool valid_request(glz::json_t const &rqst) {
  auto object_ptr = std::get_if<glz::json_t::object_t>(&rqst.data);
  if (!object_ptr)
    return false;

  // jsonrpc -> string must be "2.0"
  auto it = object_ptr->find("jsonrpc");
  if (it == object_ptr->end())
    return false;
  auto string_ptr = std::get_if<std::string>(&it->second.data);
  if (!string_ptr)
    return false;
  if (*string_ptr != "2.0")
    return false;

  // method -> string not starting with rpc
  it = object_ptr->find("method");
  if (it == object_ptr->end())
    return false;
  string_ptr = std::get_if<std::string>(&it->second.data);
  if (!string_ptr)
    return false;
  if (string_ptr->starts_with("rpc"))
    return false;

  // params -> structure, either array or object. This may be omitted.
  it = object_ptr->find("params");
  if (it != object_ptr->end()) {
    auto param_object_ptr = std::get_if<glz::json_t::object_t>(&it->second.data);
    auto param_array_ptr = std::get_if<glz::json_t::array_t>(&it->second.data);
    if (!param_object_ptr && !param_array_ptr) { // Parameters should be named parameters
      return false;
    }
    // TODO: Check internal types of the map
    // TODO: Check internal types of array
  }

  it = object_ptr->find("id");
  if (it != object_ptr->end()) {
    // Check if id is a string, double or a null.
    if (std::holds_alternative<std::string>(it->second.data) ||  std::holds_alternative<glz::json_t::null_t>(it->second.data)) {
      return true;
    } else if (std::holds_alternative<double>(it->second.data)){
      // id SHOULD NOT contain fractional parts.
      auto double_ptr = std::get_if<double>(&it->second.data);
      if (double_ptr){
        double int_part;
        return std::modf(*double_ptr, &int_part) == 0;
      } else {
        return false;
      }
    }
    return false;
  }

  // id may be omitted
  return true;
}

static response_t build_error(const std::string& message, const response_error_code_e& error){
  response_error_t resp_error{error, message, {}};
  return response_t {"2.0", {}, resp_error, nullptr};
}

static std::string return_error(const std::string&  message, const response_error_code_e& error){
  return glz::write_json<response_t>(build_error(message, error));
}

// A request is either a single request or a batch request.
// This method processes the request and verifies it.
// After which the results are returned from the function again.
// It has no side effect.
static std::string handle_request(std::string rqst_string) {
  // Parse the string as a generic type.
  // That is used to determine if this is a batch request or
  // a single request.
  glz::json_t generic_parse;
  try {
    glz::read_json(generic_parse, rqst_string);
  } catch (std::runtime_error const &err) {
    return return_error(err.what(), response_error_code_e::PARSE_ERROR);
  }

  // Check if we have a batch request or a single request.
  if (auto array_ptr = std::get_if<glz::json_t::array_t>(&generic_parse.data)) {
    // Check if the array is empty
    if (array_ptr->empty())
      return return_error("Invalid Request", response_error_code_e::INVALID_REQUEST);
    // Deal with each request
    std::vector<response_t> responses;
    for (auto const& rqst : *array_ptr){
      // Validate the request
      if(!valid_request(rqst)){
        //TODO: Exchange message for more detail when valid_request implements std::expected
        //Optional data field in jsonrpc can be utilized
        responses.emplace_back(build_error("Invalid Request", response_error_code_e::INVALID_REQUEST));
      }
    }
    return glz::write_json(responses);
  } else if (auto object_ptr = std::get_if<glz::json_t::object_t>(&generic_parse.data)) {
    // Deal with request
    if(!valid_request(*object_ptr)){
      //TODO: Exchange message for more detail when valid_request implements std::expected
      return return_error("Invalid Request", response_error_code_e::INVALID_REQUEST);
    }
  } else {
    response_error_t resp_error{response_error_code_e::INVALID_REQUEST, "Request neither batch nor array.", {}};
    response_t resp{"2.0", {}, resp_error, {}};
    return glz::write_json<response_t>(std::move(resp));
  }
  return "";
}
};
