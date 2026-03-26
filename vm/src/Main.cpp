#include "vm.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>

using namespace pascal_vm;
using Int = int64_t;
using Real = double;

int main(int argc, char* argv[]) {
  if(argc != 2) {
    std::cout << "Expected *.bin file containing bytecode, found " << (argc-1) << "arguments !\n";
    exit(-1);
  }
  VM vm(std::filesystem::exists(argv[1]) ? argv[1] : std::filesystem::path(argv[1]).replace_extension("bin").string());
  if(!vm) {
    std::cout << "Cannot read file '" << argv[1] << "' or '" << std::filesystem::path(argv[1]).replace_extension("bin") << "' !\n";
    exit(-1);
  }
  vm.run();
}