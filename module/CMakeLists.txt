## ======================================================================== ##
##
## Standalone Transfer Function Widget using ImGui
##
## ======================================================================== ##
cmake_minimum_required(VERSION 3.1)
project(TransferFunctionModule)
option(OSPRAY_MODULE_TFN_WIDGET
  "Build 2D transfer function widget for OSPRay Application" OFF)
OPTION(OSPRAY_MODULE_PARAVIEW_TFN_CVT
  "Build ParaView transfer function converter" OFF)
OPTION(OSPRAY_MODULE_VISIT_TFN_CVT
  "Build VisIt transfer function converter" OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH 
  ${CMAKE_MODULE_PATH} 
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

ospray_create_library(tfn_module_core
  core/TransferFunctionModule.cpp
  LINK
  # TODO we need to fix this linkage
  ospray_common) 

# Paraview Converter
if(OSPRAY_MODULE_PARAVIEW_TFN_CVT)
  ospray_create_application(tfnConvertFromParaView
    core/ConvertTFNParaView.cpp
    core/jsoncpp.cpp
    LINK
    tfn_module_core)
endif()

# TODO VisIt Converter
if(OSPRAY_MODULE_VISIT_TFN_CVT)
endif()

if(OSPRAY_MODULE_TFN_WIDGET)
  add_subdirectory(widgets)
endif()
