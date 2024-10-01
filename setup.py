from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
import pybind11

ext_modules = [
    Pybind11Extension(
        "hls_downloader",
        ["src/hls_downloader.cpp"],  # Ваш C++ файл
        libraries=["avformat", "avcodec", "avutil", "swscale", "avdevice", "avfilter", "spdlog", "fmt"],  # Линковка других библиотек
        include_dirs=[pybind11.get_include()],  # Добавляем pybind11 заголовки
        extra_compile_args=['-std=c++17'],  # Дополнительные флаги компиляции
    ),
]

# Описание пакета
setup(
    name="hls_downloader",  # Имя вашего пакета
    version="0.0.1",  # Версия пакета
    author="Alex Bralnin",  # Ваше имя
    author_email="shurajangit@gmail.com",  # Ваш email
    description="HLS Downloader with FFmpeg and Pybind11",  # Описание
    long_description=open("README.md").read(),  # Длинное описание (обычно README)
    long_description_content_type="text/markdown",  # Тип длинного описания
    ext_modules=ext_modules,  # Скомпилированные модули
    cmdclass={"build_ext": build_ext},  # Команда сборки
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: Linux",
    ],
    python_requires=">=3.12",  # Минимальная версия Python
    zip_safe=False,  # Пакет не должен быть запакован как архив
)
