# Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

if(NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
  cmake_minimum_required(VERSION 3.12)
  project(double_server)
  find_package(YARP REQUIRED COMPONENTS os dev)
endif()

add_executable(double_server)
target_sources(double_server PRIVATE double_server.cpp)
target_link_libraries(double_server PRIVATE YARP::YARP_os
                                            YARP::YARP_init
                                            YARP::YARP_dev)
set_property(TARGET double_server PROPERTY FOLDER "Examples/dev")
