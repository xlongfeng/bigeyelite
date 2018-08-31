#############################################################################
#
# Bigeye Lite - Accessorial Tool for Daily Test
# Copyright (C) 2018 xlongfeng <xlongfeng@126.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#############################################################################

if (WIN32)
    include_directories("${PROJECT_SOURCE_DIR}/libusb-1.0.22/include")
    set (LIB_USB libusb-1.0)
    # Copy DLL files to output directory
    if (CMAKE_CL_64)
        link_directories("${PROJECT_SOURCE_DIR}/libusb-1.0.22/MS64/dll")
    else ()
        link_directories("${PROJECT_SOURCE_DIR}/libusb-1.0.22/MS32/dll")
    endif ()
else ()
    set (LIB_USB usb-1.0)
endif ()

add_executable(${PROJECT_NAME}
    main.cpp
    bigeye.cpp
    bigeyelite.cpp
    bigeyelinker.cpp
    logmodel.cpp
    abstracttest.cpp
    simpleonofftest.cpp
    quickpresstest.cpp
    autoonofftest.cpp
    )

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Quick ${LIB_USB} gui)

if (WIN32)
    # Copy DLL files to output directory
    if (CMAKE_CL_64)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/libusb-1.0.22/MS64/dll/libusb-1.0.dll"
            "${PROJECT_BINARY_DIR}/src/libusb-1.0.dll")
    else ()
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/libusb-1.0.22/MS32/dll/libusb-1.0.dll"
            "${PROJECT_BINARY_DIR}/src/libusb-1.0.dll")
    endif ()
endif ()
