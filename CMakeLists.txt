cmake_minimum_required(VERSION 2.8)
project ( C_LCDShield_Test )
set ( CMAKE_BUILD_TYPE DEBUG )

include_directories(${PROJECT_SOURCE_DIR})
add_definitions(-DLOGGER_SUPPORT)
add_subdirectory ( C_Logger )
add_subdirectory ( C_I2C )
add_subdirectory ( C_LCDShield )

add_executable ( Test main.c )
target_link_libraries ( Test C_LCDShield C_I2C C_Logger )
