cmake_minimum_required(VERSION 3.15)
project(test_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# vcpkg 路径配置
set(Protobuf_DIR "E:/vcpkg/installed/x64-windows/share/protobuf")
set(quirc_DIR "E:/vcpkg/installed/x64-windows/share/quirc")
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
# 包含路径
include_directories("E:/vcpkg/installed/x64-windows/lib/glib-2.0/include")
include_directories("C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207/include")

# pkg-config 配置
set(GLib_DIR "E:/vcpkg/installed/x64-windows/share/glib-2.0")
set(PKG_CONFIG_EXECUTABLE "D:/pkg-config_0.23-2_win64/bin/pkg-config.exe")
find_package(PkgConfig REQUIRED)

# 查找 OpenCV
find_package(OpenCV REQUIRED PATHS "E:/vcpkg/installed/x64-windows" NO_DEFAULT_PATH)

# 添加 nlohmann_json 支持
find_package(nlohmann_json CONFIG REQUIRED)

# ================== [ LibTorch 设置 ] ==================
# 设置 LibTorch 根目录
set(LIBTORCH_ROOT "D:/libtorch")
# 自动检测 Debug / Release 模式
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type." FORCE)
endif()

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
find_library(TORCH_LIBRARY NAMES torch PATHS ${TORCH_LIB_DIR} REQUIRED)
find_library(TORCH_CPU_LIBRARY NAMES torch_cpu PATHS ${TORCH_LIB_DIR} REQUIRED)

# 如果你使用了 CUDA，请确保 torch_cuda.lib 存在
 find_library(TORCH_CUDA_LIBRARY NAMES torch_cuda PATHS ${TORCH_LIB_DIR} REQUIRED)

# 设置 TORCH_LIBRARIES
set(TORCH_LIBRARIES ${TORCH_LIBRARY} ${TORCH_CPU_LIBRARY})

# 如果你使用了 CUDA，启用下面一行
set(TORCH_LIBRARIES ${TORCH_LIBRARY} ${TORCH_CPU_LIBRARY} ${TORCH_CUDA_LIBRARY})
message(STATUS "TORCH_INCLUDE_DIRS = ${TORCH_INCLUDE_DIRS}")
message(STATUS "TORCH_LIBRARIES = ${TORCH_LIBRARIES}")
# GTK 4.0 配置
set(GTK4_LIB_DIR "E:/vcpkg/installed/x64-windows/lib")
set(GTK4_INCLUDE_DIR "E:/vcpkg/installed/x64-windows/include/gtk-4.0")
set(GDK4_INCLUDE_DIR "E:/vcpkg/installed/x64-windows/include/gdk-4.0")
include_directories(${GTK4_INCLUDE_DIR} ${GDK4_INCLUDE_DIR})
# 查找 GTK 4.0
pkg_check_modules(GTK4 REQUIRED gtk4)


include_directories("E:/vcpkg/installed/x64-windows/include/glib-2.0")
include_directories("E:/vcpkg/installed/x64-windows/include/gtk-4.0")
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

target_include_directories(test_cpp PUBLIC gtk-4
        ${glib2_inc}
        ${pando_lib}
        ${hb_lib}
        ${cairo_lib}
        ${gdk_pixbuf_lib}
        ${atk_lib}
)

# 使用 OpenCV 和 Torch 头文件
target_include_directories(test_cpp PUBLIC
        ${GTK4_INCLUDE_DIRS}
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
        c10
)

# 链接 nlohmann_json
target_link_libraries(test_cpp PUBLIC nlohmann_json::nlohmann_json)

target_link_directories(test_cpp PRIVATE
        ${TORCH_LIB_DIR}
)

## 如果你使用了 CUDA：
target_link_libraries(
        test_cpp
        PUBLIC
        torch_cuda
)


target_link_libraries(test_cpp PUBLIC
        gtk-4
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
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "My GTK+ Application")
set(CPACK_PACKAGE_VENDOR "Your Company Name")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MyGTKApp")
set(CPACK_PACKAGE_NAME "test_cpp") # 明确指定包名，避免默认值导致问题
# 设置安装路径为 packaging/Release
set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/Release")
set(CPACK_PACKAGING_INSTALL_PREFIX "/Program Files/MyApp")


# NSIS 特定设置
set(CPACK_NSIS_DISPLAY_NAME "My GTK+ Application") # 安装界面显示名称
set(CPACK_NSIS_HELP_LINK "https://yourcompany.com/support") # 可选支持链接
set(CPACK_NSIS_URL_INFO_ABOUT "https://yourcompany.com/appinfo") # 可选关于页面
set(CPACK_NSIS_CONTACT "support@yourcompany.com") # 可选联系邮箱
set(CPACK_NSIS_MODIFY_PATH ON) # 允许用户修改安装路径
set(CPACK_NSIS_ENABLE_UNINSTALL_ON_REINSTALL ON) # 覆盖安装时自动卸载旧版本

# 根据平台选择合适的打包格式
set(CPACK_GENERATOR "NSIS") # Windows 上可以使用 NSIS 生成安装程序
# 自动收集 Release/bin/ 下的所有 DLL（根据你的实际 DLL 路径调整）
file(GLOB GTK_DLLS "${CMAKE_SOURCE_DIR}/Release/*.dll")
file(GLOB OPENCV_DLLS "E:/vcpkg/installed/x64-windows/bin/*.dll")
file(GLOB LIBTORCH_DLLS "D:/libtorch/lib/*.dll" "D:/libtorch/bin/*.dll")
# 合并所有 DLL
set(ALL_DLLS ${GTK_DLLS} ${OPENCV_DLLS} ${LIBTORCH_DLLS})

# 安装 DLL 到 bin 目录
install(FILES ${ALL_DLLS}
        DESTINATION bin
        OPTIONAL
)

include(CPack)