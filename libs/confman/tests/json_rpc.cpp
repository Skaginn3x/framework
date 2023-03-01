#include <boost/ut.hpp>
#include <glaze/glaze.hpp>
#include <iostream>
#include "tfc/glaze_json_rpc.hpp"

auto main() -> int {
  namespace rpc = tfc::confman::rpc;
  namespace ut = boost::ut;

  using boost::ut::operator""_test;
  using std::string_literals::operator""s;

  "Valid requests are valid"_test = []() {
    const std::array valid_requests = {
        std::make_pair(
        R"({
          "jsonrpc": "2.0",
          "method": "add",
          "params": [1, 2, 3],
          "id": 1
          })",
            rpc::request_t{"2.0", "add", {}, 1.0}
            ),
        std::make_pair(
        // No id is valid
         R"({
             "jsonrpc": "2.0",
             "method": "add",
             "params": [1, 2, 3]
         })",
         rpc::request_t{"2.0", "add", {}, {}}
            ),
        std::make_pair(
         R"({
             "jsonrpc": "2.0",
             "method": "add",
             "params": [1, 2, 3],
             "id": null
         })",

            rpc::request_t{"2.0", "add", {}, nullptr}
            ),
          std::make_pair(
        R"({
             "jsonrpc": "2.0",
             "method": "add",
             "params": [1, 2, 3],
             "id": 2.
         })",
            rpc::request_t{"2.0", "add", {}, 2.0}
            ),
        std::make_pair(
        R"({
             "jsonrpc": "2.0",
             "method": "add",
             "params": [1, 2, 3],
             "id": "some_client_22"
         })",
        rpc::request_t{"2.0", "add", {},  "some_client_22"}
        )
    };
    for(auto const& [raw_json, resulting_request] : valid_requests){
      glz::json_t object;
      glz::read_json(object, raw_json);
      auto val = rpc::valid_request(object);
      ut::expect(val.has_value());
      ut::expect(val.value().jsonrpc == resulting_request.jsonrpc);
      ut::expect(val.value().id == resulting_request.id);
      ut::expect(val.value().method == resulting_request.method);
    }
  };
  "Invalid requests are invalid"_test = []() {
    const std::array invalid_requests = {
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
      ut::expect(!rpc::valid_request( object ).has_value());
    }
  };
  "Test invalid requests"_test = []() {
    const std::array invalid_requests_and_response = {
        std::make_pair(
            R"(
          {"jsonrpc": "2.0", "method": "foobar, "params": "bar", "baz]
        )",
          rpc::response_error_code_e::parse_error),
        std::make_pair(
            R"(
          {"jsonrpc": "2.0", "method": 1, "params": "bar"}
        )",
            rpc::response_error_code_e::invalid_request),
        std::make_pair(
            R"(
          [
            {"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},
            {"jsonrpc": "2.0", "method"
          ]
        )",
            rpc::response_error_code_e::parse_error),
        std::make_pair(
            R"(
          []
        )",
            rpc::response_error_code_e::invalid_request),
    };
    for(auto const& [raw_json, expected_error] : invalid_requests_and_response){
      auto response_string = rpc::handle_request(raw_json);
      auto response = glz::read_json<rpc::response_t>(response_string);
      ut::expect(response.error->code == expected_error);
    }
  };
  "Test invalid batch requests"_test = []() {
    const std::array<std::pair<std::string, std::vector<rpc::response_error_code_e>>, 2> invalid_requests_and_response_batch = {
        std::make_pair<std::string, std::vector<rpc::response_error_code_e >>( //NOLINT
            R"(
          [1]
        )",
            {
                rpc::response_error_code_e::invalid_request,
            }
        ),
        std::make_pair<std::string, std::vector<rpc::response_error_code_e >>( //NOLINT
            R"(
          [1, 2, 3]
        )",
            {
                rpc::response_error_code_e::invalid_request,
                rpc::response_error_code_e::invalid_request,
                rpc::response_error_code_e::invalid_request,
            }
        ),
    };
    for(auto const& [raw_json, expected_error] : invalid_requests_and_response_batch){
      auto response_string = rpc::handle_request(raw_json);
      auto response = glz::read_json<std::vector<rpc::response_t>>(response_string);
      ut::expect(response.size() == expected_error.size());
      for (size_t i = 0; i < response.size(); i++){
        ut::expect(response[i].error->code == expected_error[i]);
      }
    }
  };
  "Test valid function calls"_test = []() {
    const std::unordered_map<std::string, std::function<std::optional<glz::json_t>(std::optional<glz::json_t>)>> methods {
                  {"mirror"s, [](std::optional<glz::json_t> params){
                        return params;
                     }
                  },
                  {"add"s, [](std::optional<glz::json_t> params){
                    // TODO: This method is very complicated and no error checking is being done
                    const auto* parameters = std::get_if<glz::json_t::array_t>(&params.value().data);
                    std::vector<double> values{};
                    for(const auto& param : *parameters) {
                      if(const auto* double_ptr = param.get_if<double>()){
                        values.emplace_back(*double_ptr);
                      }
                    }
                    const double result = std::accumulate(values.begin(), values.end(), 0.0);
                    return glz::json_t(result);
                  }
                  }
    };
    {
      glz::json_t test_parameters({1,2,3});
      rpc::request_t mirror_request{"2.0", "mirror", test_parameters, "client_test_1"};
      const std::string mirror_json = glz::write_json(mirror_request);
      ut::expect(R"({"jsonrpc":"2.0","result":[1,2,3],"id":"client_test_1"})" == rpc::handle_request(mirror_json, methods));
    }
    {
      //TODO: Not sure if this is a valid request. must look into spec better.
      double test_parameters = 1.0;
      rpc::request_t mirror_request{"2.0", "mirror", test_parameters, "client_test_2"};
      const std::string mirror_json = glz::write_json(mirror_request);
      std::cout << rpc::handle_request(mirror_json, methods) << std::endl;
      ut::expect(R"({"jsonrpc":"2.0","result":1,"id":"client_test_1"})" == rpc::handle_request(mirror_json, methods));
    }
    {
      //TODO: Not sure if this is a valid request. must look into spec better.
      glz::json_t test_parameters({1,2,3,4,5,6,7,8,9});
      rpc::request_t mirror_request{"2.0", "add", test_parameters, "client_test_2"};
      const std::string mirror_json = glz::write_json(mirror_request);
      std::cout << rpc::handle_request(mirror_json, methods) << std::endl;
      ut::expect(R"({"jsonrpc":"2.0","result":45,"id":"client_test_2"})" == rpc::handle_request(mirror_json, methods));
    }
  };
}
