###############################################################################
# MIT License
#
# Copyright (c) 2022 freemine <freemine@yeah.net>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
###############################################################################

cmake_minimum_required(VERSION 3.0)
project(taos_odbc)

set(TODBC_VERSION 0.1)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(TODBC_LINUX TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(TODBC_DARWIN TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(TODBC_WINDOWS TRUE)
else()
  message(FATAL_ERROR "Not ported on `${CMAKE_SYSTEM_NAME}`")
endif()

if(NOT TODBC_WINDOWS)
  # add_compile_options(-Wall -Wextra -pedantic -Werror)
  add_compile_options(-Wall -Wextra -Werror -O0 -DODBCVER=0x0351)
else()
  add_compile_options(/WX /DODBCVER=0x0351)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

include(CTest)
include(cmake/macros.cmake)

check_requirements()

include_directories(inc)
if(TODBC_DARWIN)
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
  link_libraries(iconv)
endif()
if(TODBC_WINDOWS)
  include_directories(C:/TDengine/include ${ICONV_INSTALL_PATH}/include)
  link_directories(C:/TDengine/driver ${ICONV_INSTALL_PATH}/lib)
  link_libraries(iconv)
  #better set /source-charset:utf-8 for source-file-specific compile_options
  #add_compile_options(/source-charset:utf-8)
  add_compile_options(/DICONV_CONST=)
endif()

add_subdirectory(common)
add_subdirectory(src)
add_subdirectory(templates)
add_subdirectory(tests)

