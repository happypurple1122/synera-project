# ──────────────────────────────────────────────────
# Synera —— 2D 自走棋 GUI 游戏  CMake 构建配置
# 目标：Qt6 MinGW + C++17 + Ninja
# ──────────────────────────────────────────────────

# 最低 CMake 版本要求（需 ≥ 3.20 才能用 qt_add_executable）
cmake_minimum_required(VERSION 3.20)

# 项目名 Synera，仅启用 C++（不需要 C）
project(Synera LANGUAGES CXX)

# ── 强制 C++17 ──────────────────────────────────
set(CMAKE_CXX_STANDARD 17)              # 使用 C++17 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # 编译器不支持 C++17 时报错，不降级
set(CMAKE_CXX_EXTENSIONS OFF)           # 禁用编译器特有扩展，保证跨平台一致性

# 输出 compile_commands.json，供 clangd / 代码分析工具使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Qt 自动处理 ──────────────────────────────────
set(CMAKE_AUTOMOC ON)   # 自动运行 MOC（处理 Q_OBJECT 宏的头文件）
set(CMAKE_AUTOUIC ON)   # 自动运行 UIC（把 .ui 文件转成 C++ 头文件）
set(CMAKE_AUTORCC ON)   # 自动运行 RCC（把 .qrc 资源文件编译进二进制）

# ── 查找 Qt6 模块 ────────────────────────────────
# 优先使用 VS Code settings.json 中的 CMAKE_PREFIX_PATH，
# 未设置时使用下方默认值作为回退，确保命令行也能正确找到 Qt
if(NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "D:/Qt/6.11.1/mingw_64")
    message(STATUS "CMAKE_PREFIX_PATH 未设置，使用默认路径: ${CMAKE_PREFIX_PATH}")
endif()
find_package(Qt6 REQUIRED COMPONENTS
    Core        # QCoreApplication 等基础类
    Gui         # QPainter、QColor 等图形相关
    Widgets     # QWidget、QMainWindow 等窗口组件
    Multimedia  # QMediaPlayer 等音效播放
)

# ── 自动搜集源码 ──────────────────────────────────
# 递归查找 src/ 下所有 .cpp 和 .h，确保 AUTOMOC 能检测到 Q_OBJECT
file(GLOB_RECURSE SRC_FILES src/*.cpp)
file(GLOB_RECURSE HDR_FILES src/*.h)

# ── 生成可执行文件 ────────────────────────────────
# 同时传入 cpp 和 h 文件，让 AUTOMOC/AUTOUIC 能扫描所有 Q_OBJECT
qt_add_executable(Synera ${SRC_FILES} ${HDR_FILES})

# ── 链接 Qt 库 ──────────────────────────────────
target_link_libraries(Synera PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
)

# ── 添加头文件搜索路径 ──────────────────────────
# 让 #include "core/Unit.h" 能找到 src/core/Unit.h
target_include_directories(Synera PRIVATE ${CMAKE_SOURCE_DIR}/src)

# ── 编译后自动复制 assets 到构建目录 ──────────────
# 每次构建都会执行，确保素材始终与 exe 在同一目录
add_custom_command(TARGET Synera POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_BINARY_DIR}/assets"
    COMMENT "正在复制 assets 素材到构建目录……"
)