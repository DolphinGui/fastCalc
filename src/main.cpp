#include <fmt/core.h>
#include <span>
#include <string_view>
#include <vector>

void main_fun(std::span<std::string_view> args);

int main(int argc, char *argv[]) {
  std::vector<std::string_view> args;
  args.reserve(argc);
  for (int i = 0; i < argc; i++) {
    args.push_back(std::string_view(argv[i]));
  }

  main_fun(args);
}

void main_fun(std::span<std::string_view> args){
  if(args.size() == 0){
    fmt::print("Please enter a file name\n");
  }else{
    
  }
}