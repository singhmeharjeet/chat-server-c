project(multithreaded_server)
cmake_minimum_required(VERSION 3.0)


set (SOURCES
	main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_HOME_DIRECTORY}/include)