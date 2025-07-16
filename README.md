## GTK4 示例程序
Windows下建立使用 GTK4 编写的简单 C++ 桌面应用程序，点击按钮可显示当前时间。
### 📝 项目说明
本项目演示了如何使用 GTK4 创建一个简单的 GUI 应用程序，主要功能包括：
点击按钮更新时间标签；
使用 GTK4 的信号与回调机制；
适配 Windows 平台的构建与打包流程。
### 🧰 开发环境
编程语言：C++
GUI 框架：GTK+ 4
构建系统：CMake
编译器：GCC（MSYS2/MinGW）
打包工具：CPack + NSIS（Windows）
### 📁 项目结构
test_cpp/
├── CMakeLists.txt        # CMake 构建配置
├── main.cpp              # 主程序源码
├── README.md             # 本说明文档
└── app_icon.ico          # 应用图标（可选）
### 🛠️ 构建步骤（Windows）
安装 MSYS2 并配置环境变量；
安装 GTK4 开发库：
```shell
vcpkg install gtk:x64-windows
vcpkg install gtkmm
vcpkg integrate install
```
```shell
    mkdir packaging && cd packaging
    cmake "-DCMAKE_C_COMPILER=C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe" "-DCMAKE_CXX_COMPILER=C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe" -G "Visual Studio 17 2022" -DPKG_CONFIG_EXECUTABLE=d:/pkg-config_0.23-2_win64/bin/pkg-config.exe -DCMAKE_TOOLCHAIN_FILE=E:\vcpkg\scripts\buildsystems\vcpkg.cmake -S E:\workspace\cpp_projects\test_gkt3_win_cpp -B E:\workspace\cpp_projects\test_gkt3_win_cpp\packaging
    cmake --build E:\workspace\cpp_projects\test_gkt3_win_cpp\packaging --target test_cpp --config Release
    cpack -G NSIS --config ./packaging/CPackConfig.cmake --verbose #or#    cpack -G NSIS --config ./CPackConfig.cmake --verbose
```
### 📎 依赖说明
本项目依赖 GTK3 运行时库；
打包时会自动将所需的 DLL 和资源文件包含在安装包中；
安装后程序可独立运行，无需额外安装 GTK 运行时。