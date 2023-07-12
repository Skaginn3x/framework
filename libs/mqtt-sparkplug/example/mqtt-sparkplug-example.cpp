
// specification https://sparkplug.eclipse.org/specification/version/3.0/documents/sparkplug-specification-3.0.0.pdf

#include <sparkplug_b/sparkplug_b.pb.h>
#include <filesystem>

using org::eclipse::tahu::protobuf::Payload_Metric;
using org::eclipse::tahu::protobuf::Payload;

using std::chrono::system_clock;
using std::chrono::milliseconds;
using std::chrono::duration_cast;

int main() {
  [[maybe_unused]] Payload_Metric metric{};
  [[maybe_unused]] Payload p{};
  metric.set_name("test");
  metric.set_timestamp(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());

  std::cout << metric.name() << std::endl;

  std::cout << "--------Sparkplug b DATA-----------" << std::endl;

  std::vector<uint8_t> data = {8,240,163,235,168,148,49,18,46,10,20,78,111,100,101,95,82,101,100,47,115,111,109,101,95,115,116,114,105,110,103,24,240,163,235,168,148,49,32,12,122,13,73,115,32,116,104,105,115,32,108,105,102,101,63,24,6};
  p.ParseFromArray(data.data(), data.size());
  p.PrintDebugString();

  std::cout << "--------Sparkplug b Nbirth-----------" << std::endl;

  data = {8,134,165,128,171,148,49,18,26,10,20,78,111,100,101,32,67,111,110,116,114,111,108,47,82,101,98,105,114,116,104,32,11,112,0,18,11,10,5,98,100,83,101,113,32,8,88,0,24,0};
  p.ParseFromArray(data.data(), data.size());
  p.PrintDebugString();

  std::cout << "--------Sparkplug b Nbirth #2-----------" << std::endl;
  data = {8,177,192,243,202,148,49,18,26,10,20,78,111,100,101,95,82,101,100,47,115,111,109,101,95,110,117,109,98,101,114,32,3,56,1,18,26,10,20,78,111,100,101,95,82,101,100,47,115,111,109,101,95,115,116,114,105,110,103,32,12,56,1,24,1};
  p.ParseFromArray(data.data(), data.size());
  p.PrintDebugString();

  std::cout << "-------------------" << std::endl;
  for(auto& k : p.metrics()) {
    std::cout << k.name() << std::endl;
  }
  std::cout << "-------------------" << std::endl;
  return 0;
}
