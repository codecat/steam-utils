project(SteamUtils)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_STANDARD 11)

find_package(Readline)

add_executable(steamutils
	main.cpp
)

target_link_libraries(steamutils
	${Readline_LIBRARY}
	${CMAKE_SOURCE_DIR}/steamworks/redistributable_bin/linux64/libsteam_api.so
	pthread
)
include_directories(
	${Readline_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/steamworks/public/steam
	${CMAKE_SOURCE_DIR}/scratch/Scratch
)
