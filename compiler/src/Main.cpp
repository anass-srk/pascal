#include <cstring>
#include <exception>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include "Generator.hpp"
#include "Parser.hpp"

using namespace pascal_compiler;

void write_file(const std::string& filename, const std::vector<uint8_t>& data) {
  std::ofstream out(filename, std::ios::binary);

  if (!out) {
    throw std::runtime_error("Cannot write to file '" + filename + "' !\n");
  }

  size_t size = data.size();
  out.write(reinterpret_cast<const char*>(&size), sizeof(size));
  out.write(reinterpret_cast<const char*>(data.data()), size);
  
  out.close();
}

std::string read_file(const std::string& filename) {
  std::ifstream in(filename, std::ios::binary);

  std::string str;

  in.seekg(0, std::ios::end);
  str.reserve(in.tellg());
  in.seekg(0, std::ios::beg);
  str.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  
  return str;
}

int main(int argc, char* argv[]) {
  if(argc != 2 && argc != 4) {
    std::cout << "Expected *.pas file containing pascal code with/without *.bin file to generate (available with -o) !\n";
    exit(-1);
  }
  auto source_path = std::filesystem::exists(argv[1]) ? argv[1] : std::filesystem::path(argv[1]).replace_extension("pas");
  if(!std::filesystem::exists(source_path)) {
    std::cout << "Cannot read file '" << argv[1] << "' or '"
              << std::filesystem::path(argv[1]).replace_extension("pas")
              << "' !\n";
    exit(-1);
  }

  auto out_path = std::filesystem::path(source_path).replace_extension("bin");

  if(argc == 4) {
    if(std::strcmp(argv[2], "-o") != 0) {
      std::cout << "The only option is -o !\n";
      exit(-1);
    }
    out_path = argv[3];
  }

  try {
    Parser parser(read_file(source_path.string()));
    Generator gen(std::move(parser));
    write_file(out_path.string(), gen.data());
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
}