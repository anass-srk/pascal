add_executable(
  vm
  vm.cpp
  Main.cpp
)

target_compile_definitions(
  vm
  PRIVATE
  DEBUG_VM=1
)