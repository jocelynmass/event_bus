
# MIT License
#
# Copyright (c) 2019 Jocelyn Masserot
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal with the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
#  1. The above copyright notice and this permission notice shall be included in all
#     copies or substantial portions of the Software.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimers in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of Jocelyn Masserot, nor the names of its contributors
#     may be used to endorse or promote products derived from this Software
#     without specific prior written permission.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# WITH THE SOFTWARE.


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/includes)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/port)

IF(USE_FREERTOS)
    SET(LIB_SRC "${LIB_SRC}"
        "${CMAKE_CURRENT_LIST_DIR}/port/eb_freertos.c")
ENDIF()

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DLIB_EVENT_BUS")
SET(LIB_SRC "${LIB_SRC}"
    "${CMAKE_CURRENT_LIST_DIR}/src/event_bus.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/event_bus_worker.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/event_bus_supv.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/event_bus_stats.c")