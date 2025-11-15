# --- 1. Proje Tanımı ---
cmake_minimum_required(VERSION 3.10)
project(Blackjack C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- 2. Raylib Kütüphanesini Manuel Olarak Tanımla ---
# (vcpkg'yi unut. Kütüphaneyi C:\raylib'e koyduğumuzu varsayıyoruz)
set(RAYLIB_PATH "C:/dev/raylib")

# CMake'e .h (header) dosyalarının nerede olduğunu söyle
include_directories(${RAYLIB_PATH}/include)

# CMake'e .lib/.a (kütüphane) dosyalarının nerede olduğunu söyle
link_directories(${RAYLIB_PATH}/lib)
# --- Bitti ---

# --- 3. Programımızı (Executable) Oluştur ---
add_executable(Blackjack
        main.c
        blackjack.c
)

# --- 4. Kütüphaneleri Programımıza Bağla ---
# ÖNEMLİ: 'raylib'e ek olarak Windows'un 'winmm' ve 'gdi32'
# kütüphanelerine de ihtiyacımız var.
target_link_libraries(Blackjack PRIVATE
        raylib
        winmm
        gdi32
)
