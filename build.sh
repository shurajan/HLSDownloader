#!/bin/bash

# Остановка скрипта при любой ошибке
set -e

# Имя проекта
PROJECT_NAME="hls_downloader_project"

# Папка сборки
BUILD_DIR="build"

# Удаляем старую папку build, если она есть
if [ -d "$BUILD_DIR" ]; then
  echo "Удаление старой сборки..."
  rm -rf "$BUILD_DIR"
fi

# Создаем папку build
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Запуск CMake для генерации файлов сборки
echo "Генерация файлов сборки с помощью CMake..."
cmake ..

# Запуск сборки с помощью Make
echo "Сборка проекта..."
make

# Возвращаемся в корень проекта для установки Python расширения
cd ..

# Установка Python расширения
echo "Установка Python-расширения..."
pip install . --no-build-isolation

# Запуск тестов или примеров (например, example.py)

echo "Сборка и установка завершены успешно!"
