## ======================================================================== ##
##
##
##
## ======================================================================== ##

cmake_minimum_required(VERSION 3.1)
project(module_tfn)

option(OSPRAY_MODULE_TFN "Build 2D transfer function library for OSPRay" ON)
option(OSPRAY_MODULE_TFN_WIDGET
  "Build 2D transfer function widget for OSPRay Application" ON)

if(OSPRAY_MODULE_TFN)

  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

  add_subdirectory(tfn_reader)

  if(OSPRAY_MODULE_TFN_WIDGET)
    add_subdirectory(apps/widgets)
  endif()
  
endif()

