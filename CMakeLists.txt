## ======================================================================== ##
##
##
##
## ======================================================================== ##

cmake_minimum_required(VERSION 3.1)
project(module_tfn)
option(OSPRAY_MODULE_TFN "Transfer Function Module" OFF)

if(OSPRAY_MODULE_TFN)

  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

  add_subdirectory(tfn)
  
endif()

