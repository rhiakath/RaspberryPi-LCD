cmake_minimum_required(VERSION 2.8)
project ( C_Logger )
add_library ( C_Logger Logger.c ConsoleColor.c Logger.h ConsoleColor.h )
install_targets ( /lib/Novabase/ ${PROJECT_NAME} )
