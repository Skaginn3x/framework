#pragma once
#include <string_view>


namespace tfc::confman {
  class base {
    // exe.id.key
    void set(std::string_view id, std::string_view json_object){
      map[id] = json_object;
    }
    std::string_view get(std::string_view id){
      return map[id];
    }
  };
}