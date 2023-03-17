#include <memory>
#include <iostream>
#include <string>
#include <sstream>

#include <ecrt.h>

#include "tfc/ec.hpp"

namespace tfc::ec {
static constexpr auto deleter = [](auto* master_to_delete){ ecrt_release_master(master_to_delete); };

//master::master(uint32_t master_index)
//    : master_{ std::unique_ptr<ec_master_t, decltype(deleter)>(ecrt_request_master(master_index), deleter) } {}

auto master::create_domain() -> void {
  ecrt_master_create_domain(master_.get());
}
auto master::request(uint32_t const& master_index) -> void {//std::expected<master, std::error_code> {
  std::stringstream error_buffer;
  std::ostream::sync_with_stdio(true);
  std::streambuf * old = std::cerr.rdbuf(error_buffer.rdbuf())  ;
  ec_master_t* res = ecrt_request_master(master_index);
  if (res == nullptr ){
    std::string  const text = error_buffer.str();
    std::cerr << "CAPTURED: " << text << std::endl;
    std::cerr.rdbuf(old);
  }
}
}  // namespace tfc::ec