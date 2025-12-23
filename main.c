#include "raylib.h"
#include "blackjack.h"
#include "string.h"
#include "math.h"
const int MAX_SEATS = 5;
const float CARD_WIDTH = 84.0f;
const float CARD_HEIGHT = 120.0f;
const Rectangle tekrarOynaButton = { 850, 750, 220, 40 };
const Rectangle bahis10arttirbutton = { 780, 700, 40, 40 };
const Rectangle bahis50arttirbutton = { 830, 700, 40, 40 };
const Rectangle bahis100arttirbutton = { 880, 700, 40, 40 };
const Rectangle bahissifirlabutton = { 930, 700, 100, 40 };
const Rectangle bahiskoy = { 1040, 700, 100, 40 };
const Rectangle oyuncuekle = { 880, 350, 100, 40  };
const Rectangle oyuncuTamam = { 880, 400, 100, 40  };
//Yazıları Döndüren Fonksiyon
void YazıCizDondur(const char* text, float x,float y, float aci, int fontSize,Color color) {
    Font font = GetFontDefault();

    Vector2 textSize = MeasureTextEx(font,text,fontSize,1);

    Vector2 position = {x,y};

    Vector2 origin = {textSize.x/2, textSize.y/2};
    DrawTextPro(font,text,position,origin,aci,fontSize,1,color);
}
//Dönmüş butonlara basılıp basılmadığını kontrol eder
bool CheckCollisionRotatedRec(Vector2 point, Rectangle rec, float rotation) {
    // rec.x ve rec.y butonun MERKEZİ kabul edilir
    float rad = -rotation * DEG2RAD;
    float s = sinf(rad);
    float c = cosf(rad);

    // Noktayı merkeze göre ötele
    float tx = point.x - rec.x;
    float ty = point.y - rec.y;

    // Noktayı ters döndür
    float newX = tx * c - ty * s;
    float newY = tx * s + ty * c;

    // Kontrol
    if (newX >= -rec.width/2 && newX <= rec.width/2 &&
        newY >= -rec.height/2 && newY <= rec.height/2) {
        return true;
        }
    return false;
}
Vector2 koltuk_konumlari[5] = {
    {410, 390},  // 0. Koltuk (En Sol)
    {635, 585},  // 1. Koltuk (Sol)
    {930, 650},  // 2. Koltuk (Orta)
    {1220, 580}, // 3. Koltuk (Sağ)
    {1450, 400}  // 4. Koltuk (En Sağ)
};

typedef enum{
    STATE_OYUNCU_Ekle,
    STATE_BAHİS,         // Bahis Zamanı
    STATE_KART_DAGIT,    // El başlıyor
    STATE_OYUNCU_TURU,   // Oyuncu Hit/Stand bekliyor
    STATE_KASA_TURU,     // Kasa oynuyor
    STATE_SONUC
}GAME_STATE;

Rectangle kart_degerini_al(struct kart* kart) {//cardsheet içindeki kartların konumunu döndürür
    return (Rectangle){ (kart->konumx)*CARD_WIDTH, kart->konumy*CARD_HEIGHT, CARD_WIDTH, CARD_HEIGHT };
}
float koltuk_acilari[5] = { 54.0f, 27.0f, 0.0f, -27.0f, -54.0f };
int dolum_sirasi[5] = { 2, 1, 3, 0, 4 };
//Yeni oyuncunun değerlerini ayarlar
void yenioyuncu(struct oyuncu* oyuncu) {
    oyuncu->bahis =0;
    oyuncu->bakiye =1000;
    oyuncu->isDoubled = 0;
    oyuncu->value=0;
    oyuncu->kart_sayi=0;
    strcpy(oyuncu->sonuc,"   ");
}
//kartların bitip bitmediğini kontrol eder
void kartlarbittimi(int *kart_sayisi,struct kart* deste) {
    if (*kart_sayisi >42) {
        *kart_sayisi = 0;
        deste_olustur(deste);
        desteyi_karistir(deste);
    }
}
//Yeni el başlatır kartları dağıtır
void yeni_el(struct oyuncu oyuncular[], struct oyuncu* krupiyer, struct kart* deste, int* kart_sayisi) {
    krupiyer->kart_sayi = 0;
    for (int i = 0;i<MAX_SEATS;i++) {
        if (oyuncular[i].isActive == 1) {
            kartlarbittimi(kart_sayisi,deste);
            oyuncular[i].kart_sayi = 0;
            (*kart_sayisi)++;
            oyuncular[i].el[0] = deste[*kart_sayisi];
            (*kart_sayisi)++;
            oyuncular[i].el[1] = deste[*kart_sayisi];
            (*kart_sayisi)++;
            oyuncular[i].kart_sayi += 2;
        }
    }
    krupiyer->el[0] = deste[*kart_sayisi];
    (*kart_sayisi)++;
    krupiyer->el[1] = deste[*kart_sayisi];
    (*kart_sayisi)++;
    krupiyer->kart_sayi += 2;
}
//Kryupiyerin kartlarını ekrana çizer
void krupiyer_el_ciz(struct oyuncu* oyuncu,Texture2D spritesheet,Vector2 vector2,int gizle) {
    for (int i = 0; i<oyuncu->kart_sayi;i++) {
        Rectangle kapali_kart = {13.0f*CARD_WIDTH,3.0f*CARD_HEIGHT,CARD_WIDTH,CARD_HEIGHT};
        if (gizle == 0 && i == 0) {
            Vector2 drawPos = { vector2.x + (i * (CARD_WIDTH + 10.0f)), vector2.y };
            DrawTextureRec(spritesheet, kapali_kart, drawPos, WHITE);
        }else {
            Rectangle sourcerec = kart_degerini_al(&oyuncu->el[i]);
            Vector2 drawPos = { vector2.x + (i * (CARD_WIDTH + 10.0f)), vector2.y };
            DrawTextureRec(spritesheet, sourcerec, drawPos, WHITE);
        }
    }
}
//Oyuncuların ellerini oturdukları konuma göre çizer
void oyuncu_el_ciz(struct oyuncu* oyuncu, Texture2D spritesheet, Vector2 pos, float aci) {
    float radyan = aci * DEG2RAD; // Dereceyi Radyana çevir
    float kart_araligi = 30.0f;   // Kartlar arası mesafe

    for (int i = 0; i < oyuncu->kart_sayi; i++) {
        Rectangle sourcerec = kart_degerini_al(&oyuncu->el[i]);

        float shiftX = (i * kart_araligi) * cosf(radyan);
        float shiftY = (i * kart_araligi) * sinf(radyan);

        Rectangle destRec = {
            pos.x + shiftX,
            pos.y + shiftY,
            CARD_WIDTH,
            CARD_HEIGHT
        };

        Vector2 origin = { CARD_WIDTH / 2.0f, CARD_HEIGHT / 2.0f };

        DrawTexturePro(spritesheet, sourcerec, destRec, origin, aci, WHITE);
    }
}
//Ana Fonksiyon
int main(void)
{
    int sonuc;
    int siradaki_oyuncu = 0;
    struct oyuncu oyuncular[5];
    int oyuncu_sayisi = 0;
    char oyun_sonucu[30];
    double kasaCekmeZamani = 0.0;
    const double kasaBeklemeSuresi = 1.0;
    int kart_sayisi = 0;
    int kart_kapali_mi = 0;
    const int screenWidth = 1900;
    const int screenHeight = 1000;
    GAME_STATE mevcutDurum = STATE_OYUNCU_Ekle;
    int hesaplandi_mi = 0;
    struct kart deste[52];
    struct oyuncu krupiyer;

    deste_olustur(deste);
    desteyi_karistir(deste);
    InitWindow(screenWidth, screenHeight, "Raylib - Blackjack");
    InitAudioDevice();
    krupiyer.kart_sayi = 0;
    for(int i=0; i<5; i++) {
        yenioyuncu(&oyuncular[i]);
        oyuncular[i].isActive = 0; // Başta hepsi pasif
    }
    SetTargetFPS(60);
    //Gerekli dosyaları raylib'e tanıtır
    Texture2D cardSpriteSheet = LoadTexture("cards.png");
    Texture2D masa = LoadTexture("masa.png");
    Sound kartcekmesesi = LoadSound("ses.ogg");
    Music arkaplan = LoadMusicStream("arkaplan.ogg");
    PlayMusicStream(arkaplan);
    DrawTexture(masa,0,0,WHITE);

    if (cardSpriteSheet.id == 0) {
        TraceLog(LOG_FATAL, "HATA: 'cards.png' yüklenemedi! .exe'nin yanına kopyaladığınızdan emin olun.");
        CloseWindow();
        return -1;
    }
    while (!WindowShouldClose())
    {
        UpdateMusicStream(arkaplan);
        Vector2 mousepos = GetMousePosition();
        //State Machine
        switch (mevcutDurum) {
            case STATE_OYUNCU_Ekle: //Oyuncu ekleme
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (CheckCollisionPointRec(mousepos, oyuncuekle)){
                        kartlarbittimi(&kart_sayisi, deste);
                        int gercek_koltuk_no = dolum_sirasi[oyuncu_sayisi];
                        oyuncular[gercek_koltuk_no].isActive=1;//Oyuncu konumlarını ortadan başlayarak ekler orta-sol-sağ-en sol-en sağ
                        oyuncu_sayisi++;
                    }
                    if (CheckCollisionPointRec(mousepos, oyuncuTamam)) {
                        mevcutDurum = STATE_BAHİS;
                        siradaki_oyuncu = 0; // İlk koltuktan başla
                        // Eğer ilk koltuk boşsa dolu olanı bulana kadar ilerle
                        while(oyuncular[siradaki_oyuncu].isActive == 0&& siradaki_oyuncu< 5) {
                            siradaki_oyuncu++;
                        }
                    }
                }
                break;
            case STATE_BAHİS:// Bahis
                hesaplandi_mi = 0;
                struct oyuncu *suanki_oyuncu = &oyuncular[siradaki_oyuncu];//Sıradakı Oyuncuyu bulur
                if (suanki_oyuncu->isActive == 1) {//oyuncu aktifse fonksiyonları çalıştır
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (CheckCollisionPointRec(mousepos, bahis10arttirbutton)) {
                            suanki_oyuncu->bahis += 10;
                        }
                        if (CheckCollisionPointRec(mousepos, bahis50arttirbutton)) {
                            suanki_oyuncu->bahis += 50;
                        }
                        if (CheckCollisionPointRec(mousepos, bahis100arttirbutton)) {
                            suanki_oyuncu->bahis += 100;
                        }
                        if (CheckCollisionPointRec(mousepos, bahiskoy)) {
                            suanki_oyuncu->bakiye -= suanki_oyuncu->bahis;

                            // Bir sonraki aktif oyuncuyu bul
                            do {
                                siradaki_oyuncu++;
                            } while (siradaki_oyuncu< 5 && !oyuncular[siradaki_oyuncu].isActive);
                            // Eğer herkes bahsini koyduysa (index 5 olduysa) kart dağıt
                            if (siradaki_oyuncu>= 5) {
                                mevcutDurum = STATE_KART_DAGIT;
                            }
                        }
                        if (CheckCollisionPointRec(mousepos, bahissifirlabutton)) {
                            suanki_oyuncu->bahis = 0;
                        }
                    }
                }

                break;

            case STATE_KART_DAGIT://kart dağıtma
                yeni_el(oyuncular, &krupiyer, deste, &kart_sayisi);
                kartlarbittimi(&kart_sayisi, deste);
                // Değerleri hesapla (Herkes için)
                for(int i=0; i<5; i++) {
                    if(oyuncular[i].isActive) oyuncular[i].value = oyuncu_el_degeri(&oyuncular[i]);
                }

                // Sırayı tekrar başa al
                siradaki_oyuncu = 0;
                while(!oyuncular[siradaki_oyuncu].isActive && siradaki_oyuncu < 5) siradaki_oyuncu++;

                mevcutDurum = STATE_OYUNCU_TURU;
                break;

            case STATE_OYUNCU_TURU: {
                kart_kapali_mi = 0;
                if(siradaki_oyuncu >= MAX_SEATS) {
                    mevcutDurum = STATE_KASA_TURU;
                    kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                    break;
                }

                struct oyuncu *aktif_oyuncu = &oyuncular[siradaki_oyuncu];
                Vector2 pos = koltuk_konumlari[siradaki_oyuncu];
                float aci = koltuk_acilari[siradaki_oyuncu];
                float rad = aci * DEG2RAD;

                // Vektörler
                float s = sinf(rad);
                float c = cosf(rad);
                Vector2 ileri = { -s, c };
                Vector2 sag   = { c, s };

                // --- YENİ KONUMLANDIRMA ---
                // 1. Double Butonu: Kartlara daha yakın (100 birim)
                float doubleDist = 100.0f;
                Vector2 doublePos = {
                    pos.x + (ileri.x * doubleDist),
                    pos.y + (ileri.y * doubleDist)
                };

                // 2. Hit ve Stand: Double'ın altında (150 birim)
                float hitStandDist = 150.0f;
                float ayrilik = 55.0f; // Sağa sola açılma

                // Ortak merkez (Hit/Stand hizası)
                Vector2 btnMerkez = {
                    pos.x + (ileri.x * hitStandDist),
                    pos.y + (ileri.y * hitStandDist)
                };

                Vector2 hitPos = {
                    btnMerkez.x - (sag.x * ayrilik),
                    btnMerkez.y - (sag.y * ayrilik)
                };

                Vector2 standPos = {
                    btnMerkez.x + (sag.x * ayrilik),
                    btnMerkez.y + (sag.y * ayrilik)
                };

                // Rectangle Tanımları
                Rectangle hitRect = { hitPos.x, hitPos.y, 100, 40 };
                Rectangle standRect = { standPos.x, standPos.y, 100, 40 };
                Rectangle doubleRect = { doublePos.x, doublePos.y, 100, 40 };

                // 21 ve Üstü Kontrolü
                if (aktif_oyuncu->value >= 21) {
                    do { siradaki_oyuncu++; }
                    while (siradaki_oyuncu < MAX_SEATS && !oyuncular[siradaki_oyuncu].isActive);

                    if (siradaki_oyuncu >= MAX_SEATS) {
                        mevcutDurum = STATE_KASA_TURU;
                        kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                    }
                }
                else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    // --- TIKLAMA KONTROLLERİ ---

                    // HIT BUTONU
                    if(CheckCollisionRotatedRec(mousepos, hitRect, aci)) {
                        kartlarbittimi(&kart_sayisi, deste);
                        kart_cek(aktif_oyuncu, deste, &kart_sayisi);
                        PlaySound(kartcekmesesi);
                        aktif_oyuncu->value = oyuncu_el_degeri(aktif_oyuncu);
                    }

                    // STAND BUTONU
                    if(CheckCollisionRotatedRec(mousepos, standRect, aci)) {
                        do { siradaki_oyuncu++; }
                        while (siradaki_oyuncu < MAX_SEATS && !oyuncular[siradaki_oyuncu].isActive);

                        if (siradaki_oyuncu >= MAX_SEATS) {
                            mevcutDurum = STATE_KASA_TURU;
                            kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                        }
                    }

                    // DOUBLE BUTONU
                    if(CheckCollisionRotatedRec(mousepos, doubleRect, aci)) {
                        if (aktif_oyuncu->kart_sayi == 2 && aktif_oyuncu->bakiye >= aktif_oyuncu->bahis) {
                            aktif_oyuncu->bakiye -= aktif_oyuncu->bahis;
                            aktif_oyuncu->bahis *= 2;
                            kart_cek(aktif_oyuncu, deste, &kart_sayisi);
                            PlaySound(kartcekmesesi);
                            aktif_oyuncu->value = oyuncu_el_degeri(aktif_oyuncu);

                            do { siradaki_oyuncu++; }
                            while (siradaki_oyuncu < MAX_SEATS && !oyuncular[siradaki_oyuncu].isActive);
                            if (siradaki_oyuncu >= MAX_SEATS) {
                                mevcutDurum = STATE_KASA_TURU;
                                kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                            }
                        }
                    }
                }
                break;
            }
            //Kasa Sırası 17nin altına kart çeker
            case STATE_KASA_TURU:
                kart_kapali_mi = 1;
                int kasa_durumu = oyuncu_el_degeri(&krupiyer);
                if(kasa_durumu<17) {
                    if (GetTime() > kasaCekmeZamani) {
                        kartlarbittimi(&kart_sayisi, deste);
                        PlaySound(kartcekmesesi);
                        kart_cek(&krupiyer,deste,&kart_sayisi);
                        kasa_durumu = oyuncu_el_degeri(&krupiyer);
                        kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                    }
                }
                if (kasa_durumu>=17) {
                    mevcutDurum = STATE_SONUC;
                }
                break;

            //Oyuncuların Sonuclarını hesaplar ve bakiyeleri ayarlar
            case STATE_SONUC:
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousepos, tekrarOynaButton)) {
                        mevcutDurum = STATE_OYUNCU_Ekle; // Tekrar başlat!
                        for (int i = 0;i<MAX_SEATS;i++) {
                            oyuncular[i].bahis = 0;
                            oyuncular[i].value = 0;
                            oyuncular[i].isDoubled=0;
                            strcpy(oyuncular[i].sonuc,"   ");
                        }
                    }
                }
                //sonuçarlın 1 kere hesaplnamasını sağlar ki bakiyeler uçmasın
                if (hesaplandi_mi == 0) {
                    for (int i = 0;i<MAX_SEATS;i++) {
                        if (oyuncular[i].isActive == 1) {
                            sonuc = kazanan(&oyuncular[i],&krupiyer);
                            if (sonuc==0) {
                                strcpy(oyuncular[i].sonuc,"kasa kazandi");
                            }
                            else if (sonuc==1) {
                                strcpy(oyuncular[i].sonuc,"oyuncu kazandi");
                                oyuncular[i].bakiye += oyuncular[i].bahis*2;
                            }
                            else if (sonuc==2) {
                                strcpy(oyuncular[i].sonuc,"oyuncu patladi");
                            }
                            else if (sonuc==3) {
                                strcpy(oyuncular[i].sonuc,"berabere");
                                oyuncular[i].bakiye += oyuncular[i].bahis;
                            }
                            else if (sonuc==4) {
                                strcpy(oyuncular[i].sonuc,"Kasa patladı");
                                oyuncular[i].bakiye += oyuncular[i].bahis*2;
                            }
                        }
                    }
                    hesaplandi_mi = 1;
                }
                break;
        }
        BeginDrawing();//Çizim Başlangıcı
        ClearBackground(DARKGREEN);
        DrawTexture(masa,-450,-270,WHITE);//Arkaya masa Görüntüsü ekler
        for (int i = 0;i<MAX_SEATS;i++) {
            //sırdaki oyuncu aktif ise işlem yapar
            if (oyuncular[i].isActive == 1) {
                float aci = koltuk_acilari[i];
                Vector2 pos = koltuk_konumlari[i];
                float rad = aci * DEG2RAD;
                float s = sinf(rad);
                float c = cosf(rad);

                // 1. Kartları Çiz
                oyuncu_el_ciz(&oyuncular[i], cardSpriteSheet, pos, aci);

                // --- KOORDİNAT HESAPLARI (Açıya göre öteleme) ---

                // SKOR: Kartların Üstünde (Mesafe: -90)
                float skorDist = -90.0f;
                Vector2 skorPos = {
                    pos.x - (skorDist * s),
                    pos.y + (skorDist * c)
                };

                // BAHİS: Kartların Altında (Mesafe: +130)
                float bahisDist = 185.0f;
                Vector2 bahisPos = {
                    pos.x - (bahisDist * s),
                    pos.y + (bahisDist * c)
                };

                // BAKİYE: Bahisin Altında (Mesafe: +155)
                float bakiyeDist = 220.0f;
                Vector2 bakiyePos = {
                    pos.x - (bakiyeDist * s),
                    pos.y + (bakiyeDist * c)
                };

                // OYUNCU ADI: En Altta (Mesafe: +200)
                float isimDist = 255.0f;
                Vector2 isimPos = {
                    pos.x - (isimDist * s),
                    pos.y + (isimDist * c)
                };

                // --- YAZILARI ÇİZ ---
                YazıCizDondur(TextFormat("SKOR: %d", oyuncular[i].value), skorPos.x, skorPos.y, aci, 20, WHITE);
                YazıCizDondur(TextFormat("Bahis: %d", oyuncular[i].bahis), bahisPos.x, bahisPos.y, aci, 18, YELLOW);
                YazıCizDondur(TextFormat("Bakiye: %d", oyuncular[i].bakiye), bakiyePos.x, bakiyePos.y, aci, 18, GREEN);
                YazıCizDondur(TextFormat("Oyuncu %d", i+1), isimPos.x, isimPos.y, aci, 20, LIGHTGRAY);

                // SONUÇ YAZISI (KAZANDIN/KAYBETTİN) - Tam kartların ortasına
                if(mevcutDurum == STATE_SONUC) {
                     YazıCizDondur(oyuncular[i].sonuc, pos.x, pos.y, aci, 26, RED);
                }
            }
        }
        //Krupiyerin elini masaya çizer
        krupiyer_el_ciz(&krupiyer,cardSpriteSheet,(Vector2){700,80},kart_kapali_mi);
        if (mevcutDurum == STATE_OYUNCU_Ekle) {
            DrawRectangleRec(oyuncuekle, LIME);
            DrawText("Oyuncu Ekle",  5 + oyuncuekle.x, 15+ oyuncuekle.y, 14, BLACK);
            DrawRectangleRec(oyuncuTamam, LIME);
            DrawText("Oyun Başlasın",  5 + oyuncuTamam.x, 15+ oyuncuTamam.y, 14, BLACK);
            if (oyuncu_sayisi>5) {
                oyuncu_sayisi = 1;
            }
            DrawText(TextFormat("Oyuncu Sayisi %d ",oyuncu_sayisi),850.0f,300.0f,20.0f,WHITE);
        }
        else if (mevcutDurum == STATE_BAHİS) {
            //butonlar çizilir
            DrawRectangleRec(bahis10arttirbutton, LIME);
            DrawText("+10",  5 + bahis10arttirbutton.x, 15+ bahis10arttirbutton.y, 14, BLACK);
            DrawRectangleRec(bahis50arttirbutton, RED);
            DrawText("+50", bahis50arttirbutton.x + 5, bahis50arttirbutton.y + 15, 14, BLACK);
            DrawRectangleRec(bahis100arttirbutton, RED);
            DrawText("+100", bahis100arttirbutton.x+5, bahis100arttirbutton.y + 15, 14, BLACK);
            DrawRectangleRec(bahissifirlabutton, RED);
            DrawText("bahsi sifirla", 10.0f+bahissifirlabutton.x , bahissifirlabutton.y +15, 14, BLACK);
            DrawRectangleRec(bahiskoy, RED);
            DrawText("bahsi koy", bahiskoy.x + 10, bahiskoy.y + 15, 14, BLACK);
        }
        else if (mevcutDurum == STATE_OYUNCU_TURU) {
            Vector2 pos = koltuk_konumlari[siradaki_oyuncu];
            float aci = koltuk_acilari[siradaki_oyuncu];
            float rad = aci * DEG2RAD;

            // Vektörel hesaplama
            float s = sinf(rad);
            float c = cosf(rad);
            Vector2 ileri = { -s, c };
            Vector2 sag   = { c, s };

            // 1. Double (Üstte)
            float doubleDist = 100.0f;
            Vector2 doublePos = {
                pos.x + (ileri.x * doubleDist),
                pos.y + (ileri.y * doubleDist)
            };

            // 2. Hit/Stand (Altta)
            float hitStandDist = 150.0f;
            float ayrilik = 55.0f;
            Vector2 btnMerkez = {
                pos.x + (ileri.x * hitStandDist),
                pos.y + (ileri.y * hitStandDist)
            };
            Vector2 hitPos   = { btnMerkez.x - (sag.x * ayrilik), btnMerkez.y - (sag.y * ayrilik) };
            Vector2 standPos = { btnMerkez.x + (sag.x * ayrilik), btnMerkez.y + (sag.y * ayrilik) };

            // Rectangles
            Rectangle hitRect = { hitPos.x, hitPos.y, 100, 40 };
            Rectangle standRect = { standPos.x, standPos.y, 100, 40 };
            Rectangle doubleRect = { doublePos.x, doublePos.y, 100, 40 };
            Vector2 btnOrigin = { 50, 20 };

            // ÇİZİM
            DrawRectanglePro(hitRect, btnOrigin, aci, LIME);
            YazıCizDondur("HIT", hitPos.x, hitPos.y, aci, 20, BLACK);

            DrawRectanglePro(standRect, btnOrigin, aci, RED);
            YazıCizDondur("STAND", standPos.x, standPos.y, aci, 20, WHITE);

            if (oyuncular[siradaki_oyuncu].kart_sayi == 2) {
                DrawRectanglePro(doubleRect, btnOrigin, aci, BLUE);
                YazıCizDondur("2 KAT", doublePos.x, doublePos.y, aci, 20, WHITE);
            }
        } else if (mevcutDurum == STATE_SONUC) {
            DrawText(TextFormat("%s",oyun_sonucu),880,250,20,WHITE);
            DrawRectangleRec(tekrarOynaButton, BLUE);
            DrawText("TEKRAR OYNA", tekrarOynaButton.x + 40, tekrarOynaButton.y + 15, 20, WHITE);
        }
        EndDrawing();
    }
    UnloadTexture(cardSpriteSheet);
    UnloadSound(kartcekmesesi); // Yüklenen efektleri temizle
    UnloadMusicStream(arkaplan); // Yüklenen müziği temizle
    CloseAudioDevice();
    CloseWindow();
    return 0;
}