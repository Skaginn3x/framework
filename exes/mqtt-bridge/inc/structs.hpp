#pragma once

#include <any>
#include <cstdint>
#include <optional>
#include <string>

#include <sparkplug_b/sparkplug_b.pb.h>

namespace tfc::mqtt::structs {

enum struct ssl_active_e { yes, no };

struct spark_plug_b_variable {
  std::string name;
  org::eclipse::tahu::protobuf::DataType datatype;
  std::optional<std::any> value;
  std::string description;
};

}
