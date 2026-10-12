add_library(project_options INTERFACE)

target_compile_features(project_options INTERFACE cxx_std_20)

if(MSVC)
	target_compile_options(project_options INTERFACE /W4 /permissive-)
else()
	target_compile_options(project_options INTERFACE -Wall -Wextra -Wpedantic)
endif()

option(RTYPE_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)

if(RTYPE_ENABLE_SANITIZERS AND NOT MSVC)
	target_compile_options(project_options INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
	target_link_options(project_options INTERFACE -fsanitize=address,undefined)
endif()

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(project_options INTERFACE
		-Wno-c++98-compat
		-Wno-c++98-compat-pedantic
		-Wno-pre-c++17-compat)
endif()