#pragma once

#include <string>
#include <optional>
#include <any>
#include <cstdint>

#include <sparkplug_b/sparkplug_b.pb.h>

struct spark_plug_b_variable {
  std::string name;
  org::eclipse::tahu::protobuf::DataType datatype;
  std::optional<std::any> value;
  std::string description;
};
