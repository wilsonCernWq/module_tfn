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

  # include_directories(${GLFW_INCLUDE_DIR}
  #   ${MPI_CXX_INCLUDE_PATH}
  #   ${PIDX_INCLUDE_DIRS}
  #   ${TURBOJPEG_INCLUDE_DIR})

  # include_directories(${CMAKE_SOURCE_DIR}/apps/common
  #   ${CMAKE_SOURCE_DIR}/apps/exampleViewer)
    
  # add_library(pidx_app_util
  #   pidx_volume.cpp
  #   util.cpp
  #   image_util.cpp
  #   )
  # target_link_libraries(pidx_app_util PUBLIC
  #   ospray
  #   ospray_common
  #   ospray_mpi_common
  #   ospray
  #   ${PIDX_LIBRARIES}
  #   ${MPI_CXX_LIBRARIES}
  #   ${TURBOJPEG_LIBRARY}
  #   )

  # ospray_create_application(pidx_movie_renderer
  #   pidx_movie_renderer.cpp
  #   LINK
  #   pidx_app_util
  #   ospray
  #   ospray_common
  #   ospray_mpi_common
  #   ospray
  #   ${PIDX_LIBRARIES}
  #   ${MPI_CXX_LIBRARIES}
  #   ${TURBOJPEG_LIBRARY}
  #   )

  # ospray_create_application(pidx_render_worker
  #   pidx_render_worker.cpp
  #   LINK
  #   pidx_app_util
  #   ospray
  #   ospray_common
  #   ospray_mpi_common
  #   ospray
  #   ${PIDX_LIBRARIES}
  #   ${MPI_CXX_LIBRARIES}
  #   ${TURBOJPEG_LIBRARY}
  #   )

endif()

