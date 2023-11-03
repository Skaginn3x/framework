import std;

int main(){
  std::vector<int> a;
  a.emplace_back(10);
  a.emplace_back(10);
  a.emplace_back(10);
  a.emplace_back(10);
  a.emplace_back(11);
  std::cout << "This is some string" << std::endl;
  for(auto& d : a)
    std::cout << d << std::endl;
}
