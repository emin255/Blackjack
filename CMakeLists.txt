cmake_minimum_required(VERSION 3.20)
project(Blackjack C)

set(CMAKE_C_STANDARD 99)
if(DEFINED ENV{GITHUB_ACTIONS})

    message(STATUS "GitHub Actions tespit edildi. Raylib internetten indiriliyor...")

    include(FetchContent)
    FetchContent_Declare(
            raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG 5.0
    )
    FetchContent_MakeAvailable(raylib)

else()

    # HAYIR: Emin'in bilgisayarındayız (Local Linux)
    message(STATUS "Local bilgisayar tespit edildi. Kurulu Raylib kullanılıyor...")

    find_package(raylib REQUIRED)

endif()

# ---------------------------

add_executable(Blackjack main.c blackjack.c)

# Kütüphaneleri Bağlama Kısmı
if(WIN32 OR DEFINED ENV{GITHUB_ACTIONS})
    # Windows/GitHub ayarları (winmm Windows ses kütüphanesidir)
    target_link_libraries(Blackjack raylib winmm)
else()
    # Linux ayarları (m matematik kütüphanesidir)
    target_link_libraries(Blackjack raylib m)
endif()