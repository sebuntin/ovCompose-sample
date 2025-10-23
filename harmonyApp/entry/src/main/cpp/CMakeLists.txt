# the minimum version of CMake.
cmake_minimum_required(VERSION 3.5.0)
project(harmonyApp)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include)
find_library(hilog-lib
hilog_ndk.z)

find_package(skikobridge)

add_library(entry SHARED napi_init.cpp)

#设置这个标志位，可以开启 debug 模式，快速验证和更新代码
set(COMPOS_ARK_UI_DEBUG TRUE)
#set(COMPOS_ARK_UI_DEBUG FALSE)
if (COMPOS_ARK_UI_DEBUG)
    set(MODULE_COMPOSE_UTILS_SOURCE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../ovCompose-multiplatform-core/compose/ui/ui-arkui/src/ohosArm64Main/cpp/compose/src/main/cpp)
    add_subdirectory(
        ${MODULE_COMPOSE_UTILS_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/module_b_build
    )
    target_link_libraries(entry PRIVATE compose_arkui_utils)
    message("开启 Compose_Arkui_utils 的 debugs 模式")
endif()


target_link_libraries(entry PUBLIC libace_napi.z.so)
target_link_libraries(entry PUBLIC libdeviceinfo_ndk.z.so)
target_link_libraries(entry PUBLIC librawfile.z.so)
target_link_libraries(entry PUBLIC ${NATIVERENDER_ROOT_PATH}/../../../libs/arm64-v8a/libkn.so)
target_link_libraries(entry PUBLIC skikobridge::skikobridge)
#target_link_libraries(entry PUBLIC ${EGL-lib} ${GLES-lib} ${hilog-lib} ${libace-lib} ${libnapi-lib} ${libuv-lib} libc++_shared.so)
target_link_libraries(entry PUBLIC ${EGL-lib} ${GLES-lib} ${hilog-lib} ${libace-lib} ${libnapi-lib} ${libuv-lib} libc++_shared.so libnative_display_manager.so
        libnative_drawing.so)