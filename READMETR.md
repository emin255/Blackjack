Raylib Blackjack (C Implementation)
Bu proje, C programlama dili ve Raylib kütüphanesi kullanılarak geliştirilmiş, görsel ağırlıklı ve çok oyunculu (yerel) bir Blackjack oyunudur.Standart konsol uygulamalarından farklı olarak, kumarhane masası perspektifine uygun döndürülmüş arayüz elemanları ve gelişmiş çarpışma algoritmaları içerir.
------------------------------------------------------------------------------------
Özellikler
Çoklu Oyuncu Desteği: Aynı masada 5 kişiye kadar oyuncu eklenebilir.

Dinamik UI Yerleşimi: Oyuncu kartları, butonlar ve metinler, masanın kavisli yapısına uygun olarak trigonometrik hesaplamalarla (Sin/Cos) konumlandırılır ve açılı olarak çizilir.

Gelişmiş Tıklama Kontrolü: Döndürülmüş butonlar için özel çarpışma (collision detection) algoritması.

Bahis Sistemi: Her oyuncu için ayrı bakiye ve bahis yönetimi.

Tam Oyun Döngüsü:
Oyuncu Ekleme -> Bahis -> Kart Dağıtma -> Oyuncu Turu (Hit/Stand/Double) -> Kasa Turu -> Sonuç Hesaplama.

Ses ve Müzik: Arkaplan müziği ve kart çekme efektleri.
------------------------------------------------------------------------------------
Kurulum ve Derleme
Bu projeyi derlemek için bilgisayarınızda bir C derleyicisi (GCC/Clang), CMake ve Raylib kütüphanesinin yüklü olması gerekir.

Gereksinimler
C99 uyumlu derleyici
CMake (3.20 veya üzeri)
Raylib 4.0+

Adım Adım Derleme (Linux/macOS/Windows Git Bash)
Projeyi klonlayın veya indirin:

Bash

git clone https://github.com/emin255/blackjack-raylib.git
cd blackjack-raylib
Build klasörü oluşturun ve CMake'i çalıştırın:

Bash

mkdir build
cd build
cmake ..
Projeyi derleyin:

Bash

make
Oyunu çalıştırın:

Not: cards.png, masa.png, ses.ogg ve arkaplan.ogg dosyalarının çalıştırılabilir dosya (.exe) ile aynı klasörde olduğundan emin olun.

Bash

./Blackjack
------------------------------------------------------------------------------------
Nasıl Oynanır?
Giriş Ekranı: Masadaki boş koltukları doldurmak için "Oyuncu Ekle" butonuna basın. Yeterli sayıya ulaşınca "Oyun Başlasın"a tıklayın.

Bahis Ekranı: Her oyuncu sırayla bahis miktarını belirler (+10, +50, +100 butonları ile) ve "Bahsi Koy" butonuna basar.

Oyun Turu:

HIT: Kart çek.

STAND: Eldeki skorda kal ve sırayı devret.

DOUBLE: Bahsi ikiye katla ve sadece bir kart çek (Sadece ilk iki kartta aktif).

Kasa Turu: Kasa 17'ye ulaşana kadar kart çeker.

Sonuç: Kazananlar belirlenir, bakiyeler güncellenir ve "Tekrar Oyna" seçeneği belirir.
------------------------------------------------------------------------------------
Proje Yapısı
main.c: Oyun döngüsü, Raylib çizim fonksiyonları, UI yerleşimi ve giriş (input) yönetimi.

blackjack.c: Oyunun temel mantığı (Deste oluşturma, karıştırma, puan hesaplama, kazananı belirleme).

blackjack.h: Veri yapıları (struct oyuncu, struct kart) ve fonksiyon prototipleri.

CMakeLists.txt: Build konfigürasyonu.

Teknik Detaylar (UI Matematiği)
Projedeki en kritik bölüm, UI elemanlarının masanın şekline göre konumlandırılmasıdır. main.c içerisindeki çizim döngüsünde aşağıdaki vektörel hesaplama kullanılır:

C

// Örnek: Butonun konumunu oyuncunun açısına göre hesaplama
float rad = aci * DEG2RAD;
Vector2 ileri = { -sinf(rad), cosf(rad) }; // Oyuncunun bakış yönü
Vector2 butonPos = { 
    koltukPos.x + (ileri.x * mesafe), 
    koltukPos.y + (ileri.y * mesafe) 
};
Bu sayede tüm butonlar ve yazılar, oyuncunun oturduğu koltuğun açısına göre otomatik olarak hizalanır.
