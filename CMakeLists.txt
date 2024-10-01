cmake_minimum_required(VERSION 3.10)
project(HLSDownloader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Установим новую политику для работы с Python
cmake_policy(SET CMP0148 NEW)

# Подключение pybind11 через find_package
find_package(pybind11 REQUIRED)

# Поиск Python3 (замена устаревших модулей FindPythonInterp и FindPythonLibs)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# Подключение spdlog и fmt
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# Найдем библиотеки FFmpeg с помощью pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libavutil libswscale libavdevice libavfilter)

# Установим include и link директории
include_directories(${FFMPEG_INCLUDE_DIRS} ${Python3_INCLUDE_DIRS})
link_directories(${FFMPEG_LIBRARY_DIRS})
add_definitions(${FFMPEG_CFLAGS_OTHER})

# Добавим исходный код
add_library(hls_downloader MODULE src/hls_downloader.cpp)

# Линкуем библиотеки FFmpeg, Pybind11, Spdlog, Fmt и Python3
target_link_libraries(hls_downloader PRIVATE ${FFMPEG_LIBRARIES} pybind11::module spdlog::spdlog fmt::fmt Python3::Python)

# Установим правильный префикс и суффикс для Python-модуля
set_target_properties(hls_downloader PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}" SUFFIX ".so")

