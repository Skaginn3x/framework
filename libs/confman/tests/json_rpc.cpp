#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <iostream>
#include "tfc/glaze_json_rpc.hpp"

auto main(int argc, char **argv) -> int {
  using namespace boost::ut;
  using namespace tfc::confman::rpc;

  "Valid requests are valid"_test = []() {
    std::array valid_requests = {
        R"({
          "jsonrpc": "2.0",
          "method": "add",
          "params": [1, 2, 3],
          "id": 1
          })",
        // No id is valid
         R"({
             "jsonrpc": "2.0",
             "method": "add",
             "params": [1, 2, 3]
         })",
         R"({
             "jsonrpc": "2.0",
             "method": "add",
             "params": [1, 2, 3],
             "id": null
         })",
        R"({
             "jsonrpc": "2.0",
             "method": "add",
             "params": [1, 2, 3],
             "id": 2.
         })",
    };
    for(auto const& raw_json : valid_requests){
      glz::json_t object;
      glz::read_json(object, raw_json);
      expect(valid_request( object ));
    }
  };
  "Invalid requests are invalid"_test = []() {
    std::array invalid_requests = {
        // method name invalid, should be string and params should be structured type
        R"(
          {"jsonrpc": "2.0", "method": 1, "params": "bar"}
        )",
        // Invalid param object
        R"(
          {"jsonrpc": "2.0", "method": "add", "params": "bar"}
        )",
        // Invalid version string
        R"(
          {"jsonrpc": "2.1", "method": "add", "params": ["bar"]}
        )",
        // Invalid method name
        R"(
          {"jsonrpc": "2.1", "method": 1, "params": ["bar"]}
        )",
        R"({
             "jsonrpc": "2.0",
             "method": "add",
             "params": [1, 2, 3],
             "id": 2.1
         })",
    };
    for(auto const& raw_json : invalid_requests){
      glz::json_t object;
      glz::read_json(object, raw_json);
      expect(!valid_request( object ));
    }
  };
  "Test invalid requests"_test = []() {
    std::array invalid_requests_and_response = {
        std::make_pair(
            R"(
          {"jsonrpc": "2.0", "method": "foobar, "params": "bar", "baz]
        )",
          response_error_code_e::PARSE_ERROR
            ),
        std::make_pair(
            R"(
          {"jsonrpc": "2.0", "method": 1, "params": "bar"}
        )",
            response_error_code_e::INVALID_REQUEST
        ),
        std::make_pair(
            R"(
          [
            {"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},
            {"jsonrpc": "2.0", "method"
          ]
        )",
            response_error_code_e::PARSE_ERROR
        ),
        std::make_pair(
            R"(
          []
        )",
            response_error_code_e::INVALID_REQUEST
        ),
    };
    for(auto const& [raw_json, expected_error] : invalid_requests_and_response){
      auto response_string = handle_request(raw_json);
      auto response = glz::read_json<response_t>(response_string);
      expect(response.error->code == expected_error);
    }
  };
  "Test invalid batch requests"_test = []() {
    std::array<std::pair<std::string, std::vector<response_error_code_e>>, 2> invalid_requests_and_response_batch = {
        std::make_pair<std::string, std::vector<response_error_code_e >>(
            R"(
          [1]
        )",
            {
                response_error_code_e::INVALID_REQUEST,
            }
        ),
        std::make_pair<std::string, std::vector<response_error_code_e >>(
            R"(
          [1, 2, 3]
        )",
            {
                response_error_code_e::INVALID_REQUEST,
                response_error_code_e::INVALID_REQUEST,
                response_error_code_e::INVALID_REQUEST,
            }
        ),
    };
    for(auto const& [raw_json, expected_error] : invalid_requests_and_response_batch){
      auto response_string = handle_request(raw_json);
      auto response = glz::read_json<std::vector<response_t>>(response_string);
      expect(response.size() == expected_error.size());
      for (size_t i = 0; i < response.size(); i++){
        expect(response[i].error->code == expected_error[i]);
      }
    }
  };
}
