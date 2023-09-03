cmake_minimum_required(VERSION 3.16)
project(generic_mod_loader
	LANGUAGES C
)

set(CMAKE_C_STANDARD 99) # Our code is C99

add_executable(gml
  "src/main.c"
  "src/self_inject.c"
  "src/path.c"
  "src/console.c"
  "src/hooks.c"
)

add_subdirectory("ext/minhook")
target_link_libraries(gml "-s" "minhook")
target_include_directories(gml PRIVATE "ext/minhook/include")

