
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

if(CONFIG_EVENT_BUS)

set(EVENT_BUS_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

zephyr_interface_library_named(EVENT_BUS)
zephyr_library()

zephyr_include_directories(
    ${EVENT_BUS_DIR}/includes
    ${EVENT_BUS_DIR}/port)

zephyr_compile_definitions(WITH_ZEPHYR=1)

zephyr_library_sources(
    ${EVENT_BUS_DIR}/port/zephyr/eb_zephyr.c
    ${EVENT_BUS_DIR}/src/event_bus.c
    ${EVENT_BUS_DIR}/src/event_bus_worker.c
    ${EVENT_BUS_DIR}/src/eb_dispatcher.c
    ${EVENT_BUS_DIR}/src/event_bus_stats.c
)

zephyr_library_link_libraries(EVENT_BUS)
target_link_libraries(EVENT_BUS INTERFACE zephyr_interface)

endif(CONFIG_EVENT_BUS)