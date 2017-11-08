## ======================================================================== ##
##
##
##
## ======================================================================== ##
cmake_minimum_required(VERSION 3.1)
project(module_tfn)

option(OSPRAY_MODULE_TFN "Build 2D transfer function library for OSPRay" ON)
option(OSPRAY_MODULE_TFN_WIDGET
  "Build 2D transfer function widget for OSPRay Application" OFF)

OPTION(OSPRAY_MODULE_PARAVIEW_TFN_CVT
  "Build ParaView to OSPRay viewer transfer function converter" OFF)
OPTION(OSPRAY_MODULE_VISIT_TFN_CVT
  "Build VisIt to OSPRay viewer transfer function converter" OFF)

if(OSPRAY_MODULE_TFN)

  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

  ospray_create_library(ospray_module_tfn
    tfn/TransferFunctionModule.cpp
    LINK
    ospray_common)

  # Paraview Converter
  if(OSPRAY_MODULE_PARAVIEW_TFN_CVT)
    ospray_create_application(ospTFNConvertFromParaView
      tfn/ConvertTFNParaView.cpp
      tfn/jsoncpp.cpp
      LINK
      ospray_module_tfn)
  endif()

  # TODO VisIt Converter
  if(OSPRAY_MODULE_VISIT_TFN_CVT)
  endif()

  if(OSPRAY_MODULE_TFN_WIDGET)
    add_subdirectory(apps/widgets)
  endif()
  
endif()

