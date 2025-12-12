cmake_minimum_required(VERSION 3.20)
project(Blackjack C)

set(CMAKE_C_STANDARD 99)

# 1. Raylib paketini bul (Fedora'da yüklü olduğu için otomatik bulur)
find_package(raylib REQUIRED)

# 2. Çalıştırılabilir dosyanı oluştur (Burada main.c senin kod dosyan)
add_executable(Blackjack main.c
        blackjack.c)

# 3. Kütüphaneyi bağla (Linker'a Raylib'i kullan diyoruz)
target_link_libraries(Blackjack raylib)