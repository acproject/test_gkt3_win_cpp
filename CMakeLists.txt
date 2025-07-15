cmake_minimum_required(VERSION 3.15)
project(test_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_COMPILER "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe")
set(CMAKE_CXX_COMPILER "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe")
set(Protobuf_DIR "E:/vcpkg/installed/x64-windows/share/protobuf")
set(quirc_DIR "E:/vcpkg/installed/x64-windows/share/quirc")
# 设置 abseil 路径（根据你的实际路径修改）
set(absl_DIR "E:/vcpkg/installed/x64-windows/lib/cmake/abseil")
set(absl_DIR "E:/vcpkg/installed/x64-windows/share/abseil")
find_package(Protobuf REQUIRED)
# 设置 TIFF 头文件路径
set(TIFF_INCLUDE_DIR "E:/vcpkg/installed/x64-windows/include/tiff")

# 设置 TIFF 库文件路径（根据构建类型选择 debug 或 release）
set(TIFF_LIBRARY "E:/vcpkg/installed/x64-windows/lib/libtiff.lib")

# 如果你使用 Debug 构建：
# set(TIFF_LIBRARY_DEBUG "E:/vcpkg/installed/x64-windows/debug/lib/libtiff.lib")

# 将 TIFF 标记为已找到
set(TIFF_FOUND TRUE)

# 可选：如果你使用的是多配置构建系统（如 Visual Studio），可以这样写：
if(MSVC)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(TIFF_LIBRARY "E:/vcpkg/installed/x64-windows/debug/lib/libtiff.lib")
    else()
        set(TIFF_LIBRARY "E:/vcpkg/installed/x64-windows/lib/libtiff.lib")
    endif()
endif()
include_directories("E:/vcpkg/installed/x64-windows/lib/glib-2.0/include")
include_directories("C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207/include")

set(GLib_DIR "E:/vcpkg/installed/x64-windows/share/glib-2.0")
set(PKG_CONFIG_EXECUTABLE "D:/pkg-config_0.23-2_win64/bin/pkg-config.exe")
find_package(PkgConfig REQUIRED)

# 查找 OpenCV
find_package(OpenCV REQUIRED PATHS "E:/vcpkg/installed/x64-windows" NO_DEFAULT_PATH)

# ...（前面的内容保持不变，如设置编译器、GTK 包等）...

# ================== [ LibTorch 设置 ] ==================

# 设置 LibTorch 根目录（请根据你的实际路径修改）
set(LIBTORCH_ROOT "D:/libtorch")

# ...（前面的内容保持不变，如设置编译器、GTK 包等）...

# ================== [ LibTorch 设置 ] ==================

# 自动检测 Debug / Release 模式（默认使用 Release）
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type." FORCE)
endif()

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# 设置 Torch 库路径（debug/release 分开）
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(TORCH_LIB_DIR "${LIBTORCH_ROOT}/lib")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(TORCH_LIB_DIR "${LIBTORCH_ROOT}/debug/lib")
else()
    message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}")
endif()

# 设置 Torch 头文件路径
set(TORCH_INCLUDE_DIRS
        "${LIBTORCH_ROOT}/include"
        "${LIBTORCH_ROOT}/include/torch/csrc/api/include"
)

# 查找 LibTorch 必要库
find_library(TORCH_LIBRARY
        NAMES torch
        PATHS ${TORCH_LIB_DIR}
        REQUIRED
)

find_library(TORCH_CPU_LIBRARY
        NAMES torch_cpu
        PATHS ${TORCH_LIB_DIR}
        REQUIRED
)

# 可选：如果你使用 CUDA，请添加 cuda 相关库
find_library(TORCH_CUDA_LIBRARY NAMES torch_cuda PATHS ${TORCH_LIB_DIR} REQUIRED)

# 设置 TORCH_LIBRARIES
set(TORCH_LIBRARIES
        ${TORCH_LIBRARY}
        ${TORCH_CPU_LIBRARY}
)

# 如果你使用了分布式训练或其他模块，可以在这里继续添加
list(APPEND TORCH_LIBRARIES torch_cuda ...)

include_directories("E:/vcpkg/installed/x64-windows/include/glib-2.0")
include_directories("E:/vcpkg/installed/x64-windows/include/gtk-3.0")
link_directories("E:/vcpkg/installed/x64-windows/lib")


set(glib2_lib "E:/vcpkg/installed/x64-windows/lib/glib-2.0/")
set(glib2_inc "E:/vcpkg/installed/x64-windows/lib/glib-2.0/include")
set(pando_lib "E:/vcpkg/installed/x64-windows/include/pango-1.0")
set(hb_lib "E:/vcpkg/installed/x64-windows/include/harfbuzz")
set(cairo_lib "E:/vcpkg/installed/x64-windows/include/cairo")
set(gdk_pixbuf_lib "E:/vcpkg/installed/x64-windows/include/gdk-pixbuf-2.0")
set(atk_lib "E:/vcpkg/installed/x64-windows/include/atk-1.0")


execute_process(
        COMMAND pkg-config --cflags glib-2.0
        OUTPUT_VARIABLE GLIB_CFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCHALL "-I[^ ]+" GLIB_INCLUDE_PATHS "${GLIB_CFLAGS}")
string(REPLACE "-I" "" GLIB_INCLUDE_PATHS "${GLIB_INCLUDE_PATHS}")

add_executable(test_cpp WIN32 main.cpp)


target_include_directories(test_cpp PUBLIC gtk-3
        ${glib2_inc}
        ${pando_lib}
        ${hb_lib}
        ${cairo_lib}
        ${gdk_pixbuf_lib}
        ${atk_lib}
)

# 使用 OpenCV 和 Torch 头文件
target_include_directories(test_cpp PUBLIC
        ${OpenCV_INCLUDE_DIRS}
        ${TORCH_INCLUDE_DIRS}
)

# 链接 OpenCV
target_link_libraries(test_cpp PUBLIC
        ${OpenCV_LIBS}
)

# 链接 LibTorch
target_link_libraries(test_cpp PUBLIC
        "${TORCH_LIBRARIES}"
)


#target_link_directories(test_cpp PRIVATE
#        ${TORCH_LIB_DIR}
#)

## 如果你使用了 CUDA：
#target_link_libraries(
#        test_cpp
#        PUBLIC
#        torch_cuda
#)


target_link_libraries(test_cpp PUBLIC
        gtk-3
        gdk-3
        glib-2.0
        gobject-2.0
        gio-2.0
        intl
        pango-1.0
        pangocairo-1.0
        harfbuzz
        atk-1.0
        cairo
        gdk_pixbuf-2.0)
set_target_properties(test_cpp PROPERTIES WIN32_EXECUTABLE TRUE)
# 设置 CPack 包信息
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,.")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "My GTK+ Application")
set(CPACK_PACKAGE_VENDOR "Your Company Name")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MyGTKApp")
set(CPACK_PACKAGE_NAME "test_cpp") # 明确指定包名，避免默认值导致问题

# NSIS 特定设置
set(CPACK_NSIS_DISPLAY_NAME "My GTK+ Application") # 安装界面显示名称
set(CPACK_NSIS_HELP_LINK "https://yourcompany.com/support") # 可选支持链接
set(CPACK_NSIS_URL_INFO_ABOUT "https://yourcompany.com/appinfo") # 可选关于页面
set(CPACK_NSIS_CONTACT "support@yourcompany.com") # 可选联系邮箱
set(CPACK_NSIS_MODIFY_PATH ON) # 允许用户修改安装路径
set(CPACK_NSIS_ENABLE_UNINSTALL_ON_REINSTALL ON) # 覆盖安装时自动卸载旧版本

# 根据平台选择合适的打包格式
if(WIN32)
    set(CPACK_GENERATOR "NSIS") # Windows 上可以使用 NSIS 生成安装程序
endif()

# 安装可执行文件
install(TARGETS test_cpp
        RUNTIME DESTINATION bin
)

## 安装 GTK 运行时 DLL（确保 gtk_dlls 是通过 ldd 收集生成的）
#file(GLOB GTK_DLLS "${CMAKE_SOURCE_DIR}/Release/*.dll")
#if(GTK_DLLS)
#    install(FILES ${GTK_DLLS} DESTINATION bin)
#endif()
#
include(CPack)