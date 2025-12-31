#include "raylib.h"
#include "blackjack.h"
#include "string.h"
#include "math.h"
#define MAX_CARD_COUNT 364
#define MAX_SEATS 5
#define CARD_WIDTH 84.0f
#define CARD_HEIGHT 120.0f

const Rectangle tekrarOynaButton = { 850, 750, 220, 40 };
const Rectangle oyuncuTamam = { 880, 400, 100, 40  };

// Linear Interpolation (Yumusak Gecis)
float FloatLerp(float start, float end, float amount) {
    return start + amount * (end - start);
}

// Yazilari Donduren Fonksiyon
void YaziCizDondur(const char* text, float x,float y, float aci, int fontSize,Color color) {
    Font font = GetFontDefault();

    Vector2 textSize = MeasureTextEx(font,text,fontSize,1);

    Vector2 position = {x,y};

    Vector2 origin = {textSize.x/2, textSize.y/2};
    DrawTextPro(font,text,position,origin,aci,fontSize,1,color);
}

// Donmus butonlara basilip basilmadigini kontrol eder
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
    {425, 390},  // 0. Koltuk (En Sol)
    {650, 585},  // 1. Koltuk (Sol)
    {945, 650},  // 2. Koltuk (Orta)
    {1235, 580}, // 3. Koltuk (Sağ)
    {1465, 400}  // 4. Koltuk (En Sağ)
};

typedef enum{
    STATE_OYUNCU_Ekle,
    STATE_BAHIS,         // Bahis Zamani (S harfi duzeltildi)
    STATE_KART_DAGIT,    // El basliyor
    STATE_OYUNCU_TURU,   // Oyuncu Hit/Stand bekliyor
    STATE_KASA_TURU,     // Kasa oynuyor
    STATE_SONUC
}GAME_STATE;

Rectangle kart_degerini_al(const struct kart* kart) {//cardsheet icindeki kartlarin konumunu dondurur
    return (Rectangle){ (kart->konumx)*CARD_WIDTH, kart->konumy*CARD_HEIGHT, CARD_WIDTH, CARD_HEIGHT };
}

float koltuk_acilari[5] = { 54.0f, 27.0f, 0.0f, -27.0f, -54.0f };

// Yeni oyuncunun degerlerini ayarlar
void yenioyuncu(struct oyuncu* oyuncu) {
    oyuncu->bahis =0;
    oyuncu->splitValue = 0;
    oyuncu->sirasplittemi = 0;
    oyuncu->bakiye =1000;
    oyuncu->isDoubled = 0;
    oyuncu->value=0;
    oyuncu->kart_sayi=0;
    strcpy(oyuncu->sonuc,"   ");
    strcpy(oyuncu->splitsonuc,"   ");
    oyuncu->isSplitted = 0;
    oyuncu->splitkartsayi = 0;
}

// kartlarin bitip bitmedigini kontrol eder
void kartlarbittimi(int *kart_sayisi,struct kart* deste,Sound karistirma) {
    if (*kart_sayisi >340) {
        *kart_sayisi = 0;
        uzundesteyikaristir(deste);
        PlaySound(karistirma);
    }
}

// Yeni el baslatir kartlari dagitir
void yeni_el(struct oyuncu oyuncular[], struct oyuncu* krupiyer, struct kart* deste, int* kart_sayisi,Sound karistirma) {
    krupiyer->kart_sayi = 0;
    for (int i = 0;i<MAX_SEATS;i++) {
        if (oyuncular[i].isActive == 1) {
            kartlarbittimi(kart_sayisi,deste,karistirma);
            strcpy(oyuncular[i].splitsonuc,"   ");
            strcpy(oyuncular[i].sonuc,"   ");
            oyuncular[i].kart_sayi = 0;
            oyuncular[i].splitkartsayi = 0;
            oyuncular[i].isSplitted = 0;
            oyuncular[i].el[0] = deste[*kart_sayisi];
            (*kart_sayisi)++;
            oyuncular[i].el[1] = deste[*kart_sayisi];
            (*kart_sayisi)++;
            oyuncular[i].kart_sayi += 2;
            strcpy(oyuncular[i].sonuc,"    ");
            strcpy(oyuncular[i].splitsonuc,"    ");
        }
    }
    strcpy(krupiyer->sonuc,"    ");
    strcpy(krupiyer->splitsonuc,"    ");
    krupiyer->el[0] = deste[*kart_sayisi];
    (*kart_sayisi)++;
    krupiyer->el[1] = deste[*kart_sayisi];
    (*kart_sayisi)++;
    krupiyer->kart_sayi += 2;
}

// Kryupiyerin kartlarini ekrana cizer
void krupiyer_el_ciz(const struct oyuncu* oyuncu,Texture2D spritesheet,Vector2 vector2,int gizle) {
    for (int i = 0; i<oyuncu->kart_sayi;i++) {
        if (gizle == 0 && i == 0) {
            Rectangle kapali_kart = {13.0f*CARD_WIDTH,3.0f*CARD_HEIGHT,CARD_WIDTH,CARD_HEIGHT};
            Vector2 drawPos = { vector2.x + (i * (CARD_WIDTH + 10.0f)), vector2.y };
            DrawTextureRec(spritesheet, kapali_kart, drawPos, WHITE);
        }else {
            Rectangle sourcerec = kart_degerini_al(&oyuncu->el[i]);
            Vector2 drawPos = { vector2.x + (i * (CARD_WIDTH + 10.0f)), vector2.y };
            DrawTextureRec(spritesheet, sourcerec, drawPos, WHITE);
        }
    }
}

// Oyuncularin ellerini oturduklari konuma gore cizer
void oyuncu_el_ciz(struct oyuncu* oyuncu, Texture2D spritesheet, Vector2 pos, float aci,bool hareket_var_mi) {
    float radyan = aci * DEG2RAD; // Dereceyi Radyana cevir
    float split_araligi = 80;
    if (oyuncu->isSplitted == 1) {
        for (int i = 0; i < oyuncu->splitkartsayi; i++) {
            float kart_araligi = 30.0f;
            Rectangle sourcerec = kart_degerini_al(&oyuncu->splitEl[i]);

            float shiftX = ((i * kart_araligi)+(split_araligi)) * cosf(radyan);
            float shiftY = ((i * kart_araligi)+(split_araligi)) * sinf(radyan);

            Vector2 hedefKonum = {
                pos.x + shiftX,
                pos.y + shiftY
            };

            struct kart *aktifKart = &oyuncu->splitEl[i];
            Vector2 gorselPos = {aktifKart->mevcutKonumx,aktifKart->mevcutkonumy};

            if (i>0 && oyuncu->el[i-1].vardimmi == 0) {
                continue;
            }
            if (aktifKart->vardimmi == 0) {
                if (hareket_var_mi == true) {
                    continue;
                }
                hareket_var_mi = true;
                aktifKart->mevcutKonumx = FloatLerp(gorselPos.x, hedefKonum.x, 0.09f);
                aktifKart->mevcutkonumy= FloatLerp(gorselPos.y, hedefKonum.y, 0.09f);

                if (fabsf(gorselPos.x - hedefKonum.x) < 1.0f &&
                    fabsf(gorselPos.y - hedefKonum.y) < 1.0f) {

                    gorselPos = hedefKonum;
                    aktifKart->vardimmi = 1;
                    }
            } else {
                gorselPos = hedefKonum;
            }
            Rectangle destRec = {
                gorselPos.x,
                gorselPos.y,
                CARD_WIDTH,
                CARD_HEIGHT
            };

            Vector2 origin = { CARD_WIDTH / 2.0f, CARD_HEIGHT / 2.0f };
            DrawTexturePro(spritesheet, sourcerec, destRec, origin, aci, WHITE);

            for (int i = 0; i < oyuncu->kart_sayi; i++) {
                Rectangle sourcerec1 = kart_degerini_al(&oyuncu->el[i]);

                float shiftX1 = ((i * kart_araligi)-(split_araligi)) * cosf(radyan);
                float shiftY1 = ((i * kart_araligi)-(split_araligi)) * sinf(radyan);

                Vector2 hedefKonum1 = {
                    pos.x + shiftX1,
                    pos.y + shiftY1
                };

                struct kart *aktifKart1 = &oyuncu->el[i];
                Vector2 gorselPos1 = {aktifKart1->mevcutKonumx,aktifKart1->mevcutkonumy};

                if (i>0 && oyuncu->el[i-1].vardimmi == 0) {
                    continue;
                }
                if (aktifKart1->vardimmi == 0) {
                    if (hareket_var_mi == true) {
                        continue;
                    }
                    hareket_var_mi = true;
                    aktifKart1->mevcutKonumx = FloatLerp(gorselPos1.x, hedefKonum1.x, 0.09f);
                    aktifKart1->mevcutkonumy= FloatLerp(gorselPos1.y, hedefKonum1.y, 0.09f);

                    if (fabsf(gorselPos1.x - hedefKonum1.x) < 1.0f &&
                        fabsf(gorselPos1.y - hedefKonum1.y) < 1.0f) {

                        gorselPos1 = hedefKonum1;
                        aktifKart1->vardimmi = 1;
                        }
                } else {
                    gorselPos1 = hedefKonum1;
                }
                Rectangle destRec1 = {
                    gorselPos1.x,
                    gorselPos1.y,
                    CARD_WIDTH,
                    CARD_HEIGHT
                };

                Vector2 origin1 = { CARD_WIDTH / 2.0f, CARD_HEIGHT / 2.0f };
                DrawTexturePro(spritesheet, sourcerec1, destRec1, origin1, aci, WHITE);
            }
        }
    }else {
        for (int i = 0; i < oyuncu->kart_sayi; i++) {
            float kart_araligi = 30.0f;
            Rectangle sourcerec = kart_degerini_al(&oyuncu->el[i]);

            float shiftX = (i * kart_araligi) * cosf(radyan);
            float shiftY = (i * kart_araligi) * sinf(radyan);

            Vector2 hedefKonum = {
                pos.x + shiftX,
                pos.y + shiftY
            };

            struct kart *aktifKart = &oyuncu->el[i];
            Vector2 gorselPos = {aktifKart->mevcutKonumx,aktifKart->mevcutkonumy};

            if (i>0 && oyuncu->el[i-1].vardimmi == 0) {
                continue;
            }
            if (aktifKart->vardimmi == 0) {
                if (hareket_var_mi == true) {
                    continue;
                }
                hareket_var_mi = true;
                aktifKart->mevcutKonumx = FloatLerp(gorselPos.x, hedefKonum.x, 0.09f);
                aktifKart->mevcutkonumy= FloatLerp(gorselPos.y, hedefKonum.y, 0.09f);

                if (fabsf(gorselPos.x - hedefKonum.x) < 1.0f &&
                    fabsf(gorselPos.y - hedefKonum.y) < 1.0f) {

                    gorselPos = hedefKonum;
                    aktifKart->vardimmi = 1;
                    }
            } else {
                gorselPos = hedefKonum;
            }
            Rectangle destRec = {
                gorselPos.x,
                gorselPos.y,
                CARD_WIDTH,
                CARD_HEIGHT
            };

            Vector2 origin = { CARD_WIDTH / 2.0f, CARD_HEIGHT / 2.0f };
            DrawTexturePro(spritesheet, sourcerec, destRec, origin, aci, WHITE);
        }
    }

}

// Ana Fonksiyon
int main(void)
{
    Rectangle kapali_kart = {13.0f*CARD_WIDTH,3.0f*CARD_HEIGHT,CARD_WIDTH,CARD_HEIGHT};
    int sonuc;
    int siradaki_oyuncu = 0;
    struct oyuncu oyuncular[5];
    double kasaCekmeZamani = 0.0;
    int kart_sayisi = 0;
    int kart_kapali_mi = 0;
    const int screenWidth = 1900;
    const int screenHeight = 1000;
    GAME_STATE mevcutDurum = STATE_OYUNCU_Ekle;
    int hesaplandi_mi = 0;
    struct kart desteler[7][52];
    struct kart uzundeste[364];
    struct oyuncu krupiyer;
    for (int i = 0; i < 7; i++) {
        deste_olustur(desteler[i]);
        desteyi_karistir(desteler[i]);
    }
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 52; j++) {
            uzundeste[i*52+j] = desteler[i][j];
        }
    }
    uzundesteyikaristir(uzundeste);
    InitWindow(screenWidth, screenHeight, "Raylib - Blackjack");
    InitAudioDevice();
    krupiyer.kart_sayi = 0;
    for(int i=0; i<5; i++) {
        yenioyuncu(&oyuncular[i]);
        oyuncular[i].isActive = 0; // Basta hepsi pasif
    }
    SetTargetFPS(60);
    // Gerekli dosyalari raylib'e tanitir
    Texture2D cardSpriteSheet = LoadTexture("cards.png");
    Texture2D masa = LoadTexture("masa.png");
    Sound kartcekmesesi = LoadSound("ses.ogg");
    Sound karistirmasesi = LoadSound("karistirma.ogg");
    Music arkaplan = LoadMusicStream("arkaplan.ogg");
    PlayMusicStream(arkaplan);
    DrawTexture(masa,0,0,WHITE);

    if (cardSpriteSheet.id == 0) {
        TraceLog(LOG_FATAL, "HATA: 'cards.png' yuklenemedi! .exe'nin yanina kopyaladiginizdan emin olun.");
        CloseWindow();
        return -1;
    }
    while (!WindowShouldClose())
    {
        const double kasaBeklemeSuresi = 1.0;
        UpdateMusicStream(arkaplan);
        Vector2 mousepos = GetMousePosition();
        // State Machine
        switch (mevcutDurum) {
            case STATE_OYUNCU_Ekle: // Oyuncu ekleme
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

                    // 1. BOŞ KOLTUKLARA TIKLANDI MI KONTROL ET
                    for (int i = 0; i < MAX_SEATS; i++) {
                        // Eğer koltuk boşsa
                        if (oyuncular[i].isActive == 0) {
                            Vector2 koltukPos = koltuk_konumlari[i];
                            // Koltuğun üzerinde mi? (Yarıçapı 40 olan bir daire varsaydık)
                            if (CheckCollisionPointCircle(mousepos, koltukPos, 40)) {
                                oyuncular[i].isActive = 1;
                                yenioyuncu(&oyuncular[i]); // Puanlarını sıfırla
                                PlaySound(kartcekmesesi);  // Efekt olsun
                            }
                        }
                        // Eğer koltuk doluysa (Çıkarmak isterse diye opsiyonel)
                        else {
                            Vector2 koltukPos = koltuk_konumlari[i];
                            // Dolu koltuğa tıklarsa kaldırsın mı? (İstersen burayı sil)
                            if (CheckCollisionPointCircle(mousepos, koltukPos, 40)) {
                                oyuncular[i].isActive = 0;
                            }
                        }
                    }

                    // 2. OYUNU BAŞLAT BUTONU
                    if (CheckCollisionPointRec(mousepos, oyuncuTamam)) {
                        // En az 1 oyuncu var mı kontrol et
                        int aktifOyuncuVarMi = 0;
                        for(int k=0; k<5; k++) {
                            if(oyuncular[k].isActive) aktifOyuncuVarMi = 1;
                        }

                        if (aktifOyuncuVarMi) {
                            mevcutDurum = STATE_BAHIS;
                            siradaki_oyuncu = 0;
                            // İlk aktif oyuncuyu bul
                            while(siradaki_oyuncu < 5 && oyuncular[siradaki_oyuncu].isActive == 0) {
                                siradaki_oyuncu++;
                            }
                        }
                    }
                }
                break;
            case STATE_BAHIS:
                hesaplandi_mi = 0;
                struct oyuncu *suanki_oyuncu = &oyuncular[siradaki_oyuncu];

                Vector2 pos = koltuk_konumlari[siradaki_oyuncu];
                float aci = koltuk_acilari[siradaki_oyuncu];
                float rad = aci * DEG2RAD;

                // Vektorel hesaplama
                float s = sinf(rad);
                float c = cosf(rad);
                Vector2 ileri = { -s, c };
                Vector2 sag   = { c, s };

                float kucukayrilik = 50.0f;
                float bahiskoyayrilik = 80.0f;
                float bahissifirlaayrilik = 95.0f;
                float uzaklik = 150.0f;
                Vector2 btnMerkez = {
                    pos.x + (ileri.x*uzaklik),
                    pos.y + (ileri.y*uzaklik)
                };

                // Normal Bahis Butonları
                Vector2 bahis100pos = {btnMerkez.x, btnMerkez.y};
                Vector2 bahis50pos = {btnMerkez.x-(sag.x*kucukayrilik), btnMerkez.y-(sag.y*kucukayrilik)};
                Vector2 bahis10pos = {btnMerkez.x-2*(sag.x*kucukayrilik), btnMerkez.y-2*(sag.y*kucukayrilik)};
                Vector2 bahiskoypos = {btnMerkez.x+(sag.x*bahiskoyayrilik), btnMerkez.y+(sag.y*bahiskoyayrilik)};
                Vector2 bahissifirlapos = {btnMerkez.x+2*(sag.x*bahissifirlaayrilik), btnMerkez.y+2*(sag.y*bahissifirlaayrilik)};

                Rectangle bahis10Rect = {bahis10pos.x,bahis10pos.y,40,40};
                Rectangle bahis50Rect = {bahis50pos.x,bahis50pos.y,40,40};
                Rectangle bahis100Rect = {bahis100pos.x,bahis100pos.y,40,40};
                Rectangle bahiskoyRect = {bahiskoypos.x,bahiskoypos.y,100,40};
                Rectangle bahissifirlaRect = {bahissifirlapos.x,bahissifirlapos.y,100,40};

                // --- YENİ EKLENEN: İFLAS BUTONLARI ---
                Vector2 borcAlPos = {btnMerkez.x - (sag.x * 60), btnMerkez.y - (sag.y * 60)};
                Vector2 kalkPos = {btnMerkez.x + (sag.x * 60), btnMerkez.y + (sag.y * 60)};
                Rectangle borcAlRect = {borcAlPos.x, borcAlPos.y, 100, 40};
                Rectangle kalkRect = {kalkPos.x, kalkPos.y, 100, 40};

                if (suanki_oyuncu->isActive == 1) {

                    // DURUM 1: Bakiyesi VARSA normal bahis oynasın
                    if (suanki_oyuncu->bakiye > 0) {
                        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                            if (CheckCollisionRotatedRec(mousepos,bahis10Rect,aci)) suanki_oyuncu->bahis += 10;
                            if (CheckCollisionRotatedRec(mousepos,bahis50Rect,aci)) suanki_oyuncu->bahis += 50;
                            if (CheckCollisionRotatedRec(mousepos,bahis100Rect,aci)) suanki_oyuncu->bahis += 100;

                            if (CheckCollisionRotatedRec(mousepos,bahissifirlaRect,aci)) suanki_oyuncu->bahis = 0;

                            if (CheckCollisionRotatedRec(mousepos,bahiskoyRect,aci)) {
                                if (suanki_oyuncu->bahis != 0 && suanki_oyuncu->bahis <= suanki_oyuncu->bakiye) {
                                    suanki_oyuncu->bakiye -= suanki_oyuncu->bahis;
                                    // Sonraki oyuncuya geç
                                    do { siradaki_oyuncu++; }
                                    while (siradaki_oyuncu < 5 && !oyuncular[siradaki_oyuncu].isActive);

                                    if (siradaki_oyuncu >= 5) mevcutDurum = STATE_KART_DAGIT;
                                }
                            }
                        }
                    }
                    // DURUM 2: Bakiyesi BİTMİŞSE (0 ise) Seçenek Sun
                    else {
                        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                            // Borç Al Butonu
                            if (CheckCollisionRotatedRec(mousepos, borcAlRect, aci)) {
                                suanki_oyuncu->bakiye = 1000; // 1000 TL borç ver
                            }
                            // Masadan Kalk Butonu
                            if (CheckCollisionRotatedRec(mousepos, kalkRect, aci)) {
                                suanki_oyuncu->isActive = 0; // Oyuncuyu sil

                                // Hemen bir sonrakine geç
                                do { siradaki_oyuncu++; }
                                while (siradaki_oyuncu < 5 && !oyuncular[siradaki_oyuncu].isActive);

                                if (siradaki_oyuncu >= 5) mevcutDurum = STATE_KART_DAGIT;
                            }
                        }
                    }
                }
                // Eğer koltuk zaten boşsa atla (Eski kodundaki gibi)
                else {
                     do { siradaki_oyuncu++; }
                     while (siradaki_oyuncu < 5 && !oyuncular[siradaki_oyuncu].isActive);
                     if (siradaki_oyuncu >= 5) mevcutDurum = STATE_KART_DAGIT;
                }
                break;

            case STATE_KART_DAGIT:// kart dagitma
                yeni_el(oyuncular, &krupiyer, uzundeste, &kart_sayisi,karistirmasesi);
                kartlarbittimi(&kart_sayisi, uzundeste,karistirmasesi);
                // Degerleri hesapla (Herkes icin)
                for(int i=0; i<5; i++) {
                    if(oyuncular[i].isActive) oyuncular[i].value = oyuncu_el_degeri(oyuncular[i].el,oyuncular[i].kart_sayi);
                }

                // Sirayi tekrar basa al
                siradaki_oyuncu = 0;
                while(!oyuncular[siradaki_oyuncu].isActive && siradaki_oyuncu < 5) siradaki_oyuncu++;

                mevcutDurum = STATE_OYUNCU_TURU;
                break;
            case STATE_OYUNCU_TURU: {
                kart_kapali_mi = 0;

                // Eğer tüm oyuncular bittiyse Kasa'ya geç
                if(siradaki_oyuncu >= MAX_SEATS) {
                    mevcutDurum = STATE_KASA_TURU;
                    kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                    break;
                }

                struct oyuncu *aktif_oyuncu = &oyuncular[siradaki_oyuncu];

                // Eğer oyuncu pasifse (boş koltuksa) direk geç
                if(aktif_oyuncu->isActive == 0) {
                    siradaki_oyuncu++;
                    break;
                }

                Vector2 pos2 = koltuk_konumlari[siradaki_oyuncu];
                float aci2 = koltuk_acilari[siradaki_oyuncu];
                float rad2 = aci2 * DEG2RAD;
                Vector2 doublePos,hitPos,standPos;
                // Vektorler
                float s2 = sinf(rad2);
                float c2 = cosf(rad2);
                Vector2 ileri2 = { -s2, c2 };
                Vector2 sag2   = { c2, s2 };
                float doubleDist = 110.0f;
                Vector2 splitPos ={
                    pos2.x + (ileri2.x * doubleDist) - ((sag2.x) * 100), // Hafif sola
                    pos2.y + (ileri2.y * doubleDist) - (sag2.y * 100)
                };
                // --- BUTON KONUMLANDIRMA ---
                // --- AKILLI KOORDİNAT SİSTEMİ (2x2 Düzen) ---
                // 1. Önce "Yanal Kayma" (Split sonrası el takibi için)
                float yanalKayma = 0.0f;

                if (aktif_oyuncu->isSplitted == 1) {
                    if (aktif_oyuncu->sirasplittemi == 0) {
                        yanalKayma = -100.0f; // 1. El için sola kay
                    } else {
                        yanalKayma = 100.0f;  // 2. El için sağa kay
                    }
                }

                // 2. İki Farklı Merkez Belirle (Üst Sıra ve Alt Sıra)

                // ALT SIRA (Hit ve Stand) - Koltuktan daha uzak
                float altSiraUzaklik = 160.0f;
                Vector2 altMerkez = {
                    pos2.x + (ileri2.x * altSiraUzaklik) + (sag2.x * yanalKayma),
                    pos2.y + (ileri2.y * altSiraUzaklik) + (sag2.y * yanalKayma)
                };

                // ÜST SIRA (Split ve Double) - Koltuğa daha yakın
                float ustSiraUzaklik = 110.0f;
                Vector2 ustMerkez = {
                    pos2.x + (ileri2.x * ustSiraUzaklik) + (sag2.x * yanalKayma),
                    pos2.y + (ileri2.y * ustSiraUzaklik) + (sag2.y * yanalKayma)
                };

                // 3. Butonları Sağa/Sola Aç (Ayrılık Miktarı)
                float butonAraligi = 60.0f;

                // --- ALT SIRA BUTONLARI ---
                // HIT (Sol Alt)
                Vector2 hitPos1 = {
                    altMerkez.x - (sag2.x * butonAraligi),
                    altMerkez.y - (sag2.y * butonAraligi)
                };

                // STAND (Sağ Alt)
                Vector2 standPos1 = {
                    altMerkez.x + (sag2.x * butonAraligi),
                    altMerkez.y + (sag2.y * butonAraligi)
                };

                // --- ÜST SIRA BUTONLARI ---
                // SPLIT (Sol Üst)
                Vector2 splitPos1 = {
                    ustMerkez.x - (sag2.x * butonAraligi),
                    ustMerkez.y - (sag2.y * butonAraligi)
                };

                // DOUBLE (Sağ Üst)
                Vector2 doublePos1 = {
                    ustMerkez.x + (sag2.x * butonAraligi),
                    ustMerkez.y + (sag2.y * butonAraligi)
                };

                // Rectangle Tanımları
                Rectangle hitRect = { hitPos1.x, hitPos1.y, 100, 40 };
                Rectangle standRect = { standPos1.x, standPos1.y, 100, 40 };
                Rectangle doubleRect = { doublePos1.x, doublePos1.y, 100, 40 };
                Rectangle splitRect = { splitPos1.x, splitPos1.y, 100, 40 };

                // --- HANGİ ELİ OYNUYORUZ? ---
                // Puanı ve kart sayısını şu an oynanan ele göre belirle
                int *suanki_puan;
                int *suanki_kartsayi;
                struct kart *suanki_el;

                if (aktif_oyuncu->isSplitted == 1 && aktif_oyuncu->sirasplittemi == 1) {
                    // İkinci (Split) eli oynuyor
                    suanki_puan = &aktif_oyuncu->splitValue;
                    suanki_kartsayi = &aktif_oyuncu->splitkartsayi;
                    suanki_el = aktif_oyuncu->splitEl;
                } else {
                    // İlk (Ana) eli oynuyor
                    suanki_puan = &aktif_oyuncu->value;
                    suanki_kartsayi = &aktif_oyuncu->kart_sayi;
                    suanki_el = aktif_oyuncu->el;
                }

                // --- OTOMATİK KONTROLLER (21 Üstü Patlama) ---
                if (*suanki_puan >= 21) {
                    // Eğer split yapmışsa ve ilk eldeyse -> İKİNCİ ELE GEÇ
                    if (aktif_oyuncu->isSplitted == 1 && aktif_oyuncu->sirasplittemi == 0) {
                        aktif_oyuncu->sirasplittemi = 1;
                    }
                    // Yoksa -> SONRAKİ OYUNCUYA GEÇ
                    else {
                        do { siradaki_oyuncu++; }
                        while (siradaki_oyuncu < MAX_SEATS && !oyuncular[siradaki_oyuncu].isActive);

                        if (siradaki_oyuncu >= MAX_SEATS) {
                            mevcutDurum = STATE_KASA_TURU;
                            kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                        }
                    }
                }

                // --- TIKLAMA MANTIĞI ---
                else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    // 1. SPLIT BUTONU (Sadece ilk turda, kartlar eşitse ve split yapılmadıysa)
                    if (aktif_oyuncu->isSplitted == 0 && aktif_oyuncu->kart_sayi == 2 &&
                        aktif_oyuncu->el[0].konumx == aktif_oyuncu->el[1].konumx&&
                                        aktif_oyuncu->bakiye>=aktif_oyuncu->bahis) {
                        if (CheckCollisionRotatedRec(mousepos, splitRect, aci2)) {
                            // Split İşlemleri
                            aktif_oyuncu->isSplitted = 1;
                            aktif_oyuncu->sirasplittemi = 0; // Önce ilk el oynanacak
                            aktif_oyuncu->bakiye -= aktif_oyuncu->bahis;
                            // 2. Kartı Split eline taşı
                            aktif_oyuncu->splitEl[0] = aktif_oyuncu->el[1];

                            // Sayıları eşitle (Her elde 1 kart var şuan)
                            aktif_oyuncu->kart_sayi = 1;
                            aktif_oyuncu->splitkartsayi = 1;

                            // İki ele de birer kart dağıt (Blackjack kuralı)
                            kart_cek(&aktif_oyuncu->kart_sayi, aktif_oyuncu->el, uzundeste, &kart_sayisi);
                            kart_cek(&aktif_oyuncu->splitkartsayi, aktif_oyuncu->splitEl, uzundeste, &kart_sayisi);

                            PlaySound(karistirmasesi); // Ses efekti
                            // Değerleri güncelle
                            aktif_oyuncu->value = oyuncu_el_degeri(aktif_oyuncu->el,aktif_oyuncu->kart_sayi);
                            aktif_oyuncu->splitValue = oyuncu_el_degeri(aktif_oyuncu->splitEl,aktif_oyuncu->splitkartsayi);
                        }
                    }

                    // 2. HIT BUTONU
                    if(CheckCollisionRotatedRec(mousepos, hitRect, aci2)) {
                        kartlarbittimi(&kart_sayisi, uzundeste, karistirmasesi);

                        // Şu an hangi el oynanıyorsa ona kart çek
                        kart_cek(suanki_kartsayi, suanki_el, uzundeste, &kart_sayisi);
                        PlaySound(kartcekmesesi);

                        // Değeri güncelle
                        if (aktif_oyuncu->isSplitted == 1 && aktif_oyuncu->sirasplittemi == 1) {
                             // Split puanı hesaplama (Yukarıdaki kopya yöntemiyle veya struct yapına göre)
                             aktif_oyuncu->splitValue = oyuncu_el_degeri(aktif_oyuncu->splitEl,aktif_oyuncu->splitkartsayi);
                        } else {
                             aktif_oyuncu->value = oyuncu_el_degeri(aktif_oyuncu->el,aktif_oyuncu->kart_sayi);
                        }
                    }

                    // 3. STAND BUTONU
                    if(CheckCollisionRotatedRec(mousepos, standRect, aci2)) {
                        // Eğer Split var ve 1. el oynanıyorsa -> 2. ELE GEÇ
                        if (aktif_oyuncu->isSplitted == 1 && aktif_oyuncu->sirasplittemi == 0) {
                            aktif_oyuncu->sirasplittemi = 1;
                        }
                        // Yoksa -> SONRAKİ OYUNCU
                        else {
                            do { siradaki_oyuncu++; }
                            while (siradaki_oyuncu < MAX_SEATS && !oyuncular[siradaki_oyuncu].isActive);

                            if (siradaki_oyuncu >= MAX_SEATS) {
                                mevcutDurum = STATE_KASA_TURU;
                                kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                            }
                        }
                    }

                    // 4. DOUBLE BUTONU (Sadece 2 kart varken)
                    if(*suanki_kartsayi == 2 && aktif_oyuncu->bakiye >= aktif_oyuncu->bahis) {
                        if(CheckCollisionRotatedRec(mousepos, doubleRect, aci2)) {
                            // Bahsi düş
                            aktif_oyuncu->bakiye -= aktif_oyuncu->bahis;
                            // (Split bahsi ayrı tutulmalı ama şimdilik basitleştirdim)
                            aktif_oyuncu->bahis *= 2;

                            // Tek kart çek
                            kart_cek(suanki_kartsayi, suanki_el, uzundeste, &kart_sayisi);
                            PlaySound(kartcekmesesi);

                            // Puanı güncelle
                            if (aktif_oyuncu->isSplitted == 1 && aktif_oyuncu->sirasplittemi == 1) {
                                *suanki_puan = oyuncu_el_degeri(aktif_oyuncu->el,aktif_oyuncu->kart_sayi);
                            } else {
                                aktif_oyuncu->value = oyuncu_el_degeri(aktif_oyuncu->el,aktif_oyuncu->kart_sayi);
                            }

                            // Double'dan sonra mecburi Stand yapılır
                            if (aktif_oyuncu->isSplitted == 1 && aktif_oyuncu->sirasplittemi == 0) {
                                aktif_oyuncu->sirasplittemi = 1;
                            } else {
                                do { siradaki_oyuncu++; }
                                while (siradaki_oyuncu < MAX_SEATS && !oyuncular[siradaki_oyuncu].isActive);
                                if (siradaki_oyuncu >= MAX_SEATS) {
                                    mevcutDurum = STATE_KASA_TURU;
                                    kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                                }
                            }
                        }
                    }
                }

                // --- BUTON ÇİZİMLERİ ---

                // HIT (Yeşil)
                DrawRectanglePro(hitRect, altMerkez, aci2, LIME);
                YaziCizDondur("HIT", hitPos.x, hitPos.y, aci2, 20, BLACK);

                // STAND (Kırmızı)
                DrawRectanglePro(standRect,altMerkez, aci2, RED);
                YaziCizDondur("STAND", standPos.x, standPos.y, aci2, 20, WHITE);

                // DOUBLE (Mavi - Şartlı)
                if (*suanki_kartsayi == 2) {
                    DrawRectanglePro(doubleRect, ustMerkez, aci2, BLUE);
                    YaziCizDondur("DOUBLE", doublePos.x, doublePos.y, aci2, 20, WHITE);
                }

                // SPLIT (Gri - Şartlı)
                if (aktif_oyuncu->isSplitted == 0 && aktif_oyuncu->kart_sayi == 2 &&
                    aktif_oyuncu->el[0].konumx == aktif_oyuncu->el[1].konumx) {
                    DrawRectanglePro(splitRect, ustMerkez, aci2, LIGHTGRAY);
                    YaziCizDondur("SPLIT", splitPos.x, splitPos.y, aci2, 20, BLACK);
                }

                // HANGİ EL OYNANIYOR GÖSTERGESİ (Ok işareti gibi)
                if(aktif_oyuncu->isSplitted) {
                    const char* elText = (aktif_oyuncu->sirasplittemi == 0) ? "<< EL 1" : "EL 2 >>";
                    YaziCizDondur(elText, pos2.x, pos2.y - 100, aci2, 25, YELLOW);
                }

                break;
            }

             // Kasa Sirasi 17nin altina kart ceker
            case STATE_KASA_TURU:
                kart_kapali_mi = 1;
                int kasa_durumu = oyuncu_el_degeri(krupiyer.el,krupiyer.kart_sayi);
                if(kasa_durumu<17) {
                    if (GetTime() > kasaCekmeZamani) {
                        kartlarbittimi(&kart_sayisi, uzundeste,karistirmasesi);
                        PlaySound(kartcekmesesi);
                        kart_cek(&krupiyer.kart_sayi,krupiyer.el,uzundeste,&kart_sayisi);
                        kasa_durumu = oyuncu_el_degeri(krupiyer.el,krupiyer.kart_sayi);
                        kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                    }
                }
                if (kasa_durumu>=17) {
                    mevcutDurum = STATE_SONUC;
                }
                break;

            // Oyuncularin Sonuclarini hesaplar ve bakiyeleri ayarlar
            case STATE_SONUC:
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousepos, tekrarOynaButton)) {
                        mevcutDurum = STATE_OYUNCU_Ekle; // Tekrar baslat!
                        for (int i = 0;i<MAX_SEATS;i++) {
                            oyuncular[i].bahis = 0;
                            oyuncular[i].value = 0;
                            oyuncular[i].isDoubled=0;
                            strcpy(oyuncular[i].sonuc,"   ");
                            strcpy(oyuncular[i].splitsonuc,"   ");
                        }
                        krupiyer.kart_sayi = 0;
                        krupiyer.value = 0;
                    }
                }
                // sonuclarin 1 kere hesaplanmasini saglar ki bakiyeler ucmasin
                if (hesaplandi_mi == 0) {
                    for (int i = 0;i<MAX_SEATS;i++) {
                        if (oyuncular[i].isActive == 1) {
                            kazanan(&oyuncular[i],&krupiyer);
                        }
                    }
                    hesaplandi_mi = 1;
                }
                break;
        }
        BeginDrawing();// Cizim Baslangici
        ClearBackground(DARKGREEN);
        DrawTexture(masa,-450,-270,WHITE);// Arkaya masa Goruntusu ekler
        DrawText(TextFormat("Kalan Kart: %d", MAX_CARD_COUNT - kart_sayisi), 20, 20, 20, RED);
        DrawText(TextFormat("Kart Sayac: %d", kart_sayisi), 20, 50, 20, RED);
        for (int i = 0 ;i<MAX_CARD_COUNT-kart_sayisi;i++) {
            Vector2 drawPos = { 200+(i * 1), 100};
            DrawTextureRec(cardSpriteSheet, kapali_kart, drawPos, WHITE);
        }
        for (int i = 0;i<MAX_SEATS;i++) {
            // sirdaki oyuncu aktif ise islem yapar
            if (oyuncular[i].isActive == 1) {
                bool global_animasyon_kilit = false;
                float aci = koltuk_acilari[i];
                Vector2 pos = koltuk_konumlari[i];
                float rad = aci * DEG2RAD;
                float s = sinf(rad);
                float c = cosf(rad);
                Vector2 sag   = { c, s };
                // 1. Kartlari Ciz
                if (mevcutDurum == STATE_BAHIS||mevcutDurum==STATE_OYUNCU_Ekle) {

                }else {
                    oyuncu_el_ciz(&oyuncular[i], cardSpriteSheet, pos, aci,global_animasyon_kilit);
                }
                float skorDist = -90.0f;
                Vector2 skorPos = {
                    pos.x - (skorDist * s),
                    pos.y + (skorDist * c)
                };

                // BAHIS: Kartlarin Altinda (Mesafe: +130)
                float bahisDist = 185.0f;
                Vector2 bahisPos = {
                    pos.x - (bahisDist * s),
                    pos.y + (bahisDist * c)
                };

                // BAKIYE: Bahisin Altinda (Mesafe: +155)
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
                // --- YAZILARI CIZ ---
                if (oyuncular[i].isSplitted) {
                    YaziCizDondur(TextFormat("SKOR: %d", oyuncular[i].splitValue), skorPos.x+(sag.x*100), skorPos.y+(sag.y*100), aci, 20, WHITE);
                    YaziCizDondur(TextFormat("SKOR: %d", oyuncular[i].value), skorPos.x-(sag.x*100), skorPos.y-(sag.y*100), aci, 20, WHITE);
                }else {
                    YaziCizDondur(TextFormat("SKOR: %d", oyuncular[i].value), skorPos.x, skorPos.y, aci, 20, WHITE);
                }
                YaziCizDondur(TextFormat("Bahis: %d", oyuncular[i].bahis), bahisPos.x, bahisPos.y, aci, 18, YELLOW);
                YaziCizDondur(TextFormat("Bakiye: %d", oyuncular[i].bakiye), bakiyePos.x, bakiyePos.y, aci, 18, GREEN);
                YaziCizDondur(TextFormat("Oyuncu %d", i+1), isimPos.x, isimPos.y, aci, 20, LIGHTGRAY);

                // SONUC YAZISI (KAZANDIN/KAYBETTIN) - Tam kartlarin ortasina
                if(mevcutDurum == STATE_SONUC) {

                    if (oyuncular[i].isSplitted==1) {
                        YaziCizDondur(oyuncular[i].splitsonuc, pos.x+(sag.x*100), pos.y+(sag.y*100), aci, 20, RED);
                        YaziCizDondur(oyuncular[i].sonuc, pos.x-(sag.x*100), pos.y-(sag.y*100), aci, 20, RED);
                    }
                    else {
                        YaziCizDondur(oyuncular[i].sonuc, pos.x, pos.y, aci, 20, RED);
                    }
                }
            }
        }
        // Krupiyerin elini masaya cizer
        krupiyer_el_ciz(&krupiyer,cardSpriteSheet,(Vector2){700,80},kart_kapali_mi);
        if (mevcutDurum == STATE_OYUNCU_Ekle) {
            // Tüm koltukları gez
            for (int i = 0; i < MAX_SEATS; i++) {
                Vector2 pos = koltuk_konumlari[i];
                float aci = koltuk_acilari[i];

                // Eğer koltuk DOLU ise
                if (oyuncular[i].isActive == 1) {
                    // Oyuncuyu çiz (Mevcut fonksiyonunu kullanabilirsin veya basit bir daire)
                    DrawCircleV(pos, 40, MAROON);
                    DrawText("IPTAL", pos.x - 20, pos.y - 5, 10, WHITE);

                    // Oyuncu ismini yaz
                    YaziCizDondur(TextFormat("Oyuncu %d", i+1), pos.x, pos.y + 50, aci, 20, WHITE);
                }
                // Eğer koltuk BOŞ ise
                else {
                    // Yeşil bir daire ve artı işareti çiz
                    DrawCircleV(pos, 40, DARKGREEN); // Buton zemini

                    // Artı (+) İşareti
                    DrawRectangle(pos.x - 5, pos.y - 20, 10, 40, WHITE); // Dikey
                    DrawRectangle(pos.x - 20, pos.y - 5, 40, 10, WHITE); // Yatay

                    YaziCizDondur("OTUR", pos.x, pos.y + 50, aci, 20, LIGHTGRAY);
                }
            }

            // Başlat Butonu
            DrawRectangleRec(oyuncuTamam, BLUE);
            DrawText("BASLAT",  15 + oyuncuTamam.x, 12+ oyuncuTamam.y, 20, WHITE);

            // Bilgilendirme
            DrawText("Masaya oturmak icin koltuklara tiklayin.", 700, 300, 20, WHITE);
        }
        else if (mevcutDurum == STATE_BAHIS) {
            Vector2 pos = koltuk_konumlari[siradaki_oyuncu];
            float aci = koltuk_acilari[siradaki_oyuncu];
            float rad = aci * DEG2RAD;
            // Vektorel hesaplama
            float s = sinf(rad);
            float c = cosf(rad);
            Vector2 ileri = { -s, c };
            Vector2 sag   = { c, s };

            // DEGISKEN ADLARI DUZELTILDI
            float kucukayrilik = 50.0f;
            float bahiskoyayrilik = 80.0f;
            float bahissifirlaayrilik = 95.0f;
            float uzaklik = 150.0f;
            Vector2 btnMerkez = {
                pos.x + (ileri.x*uzaklik),
                pos.y + (ileri.y*uzaklik)
            };

            Vector2 kucukorigin = {20,20};
            Vector2 buyukorigin = {50,20};
            Vector2 bahis100pos = {btnMerkez.x, btnMerkez.y};
            Vector2 bahis50pos = {btnMerkez.x-(sag.x*kucukayrilik), btnMerkez.y-(sag.y*kucukayrilik)};
            Vector2 bahis10pos = {btnMerkez.x-2*(sag.x*kucukayrilik), btnMerkez.y-2*(sag.y*kucukayrilik)};
            Vector2 bahiskoypos = {btnMerkez.x+(sag.x*bahiskoyayrilik), btnMerkez.y+(sag.y*bahiskoyayrilik)};
            Vector2 bahissifirlapos = {btnMerkez.x+2*(sag.x*bahissifirlaayrilik), btnMerkez.y+2*(sag.y*bahissifirlaayrilik)};

            Vector2 borcAlPos = {btnMerkez.x - (sag.x * 60), btnMerkez.y - (sag.y * 60)};
            Vector2 kalkPos = {btnMerkez.x + (sag.x * 60), btnMerkez.y + (sag.y * 60)};
            Rectangle borcAlRect = {borcAlPos.x, borcAlPos.y, 100, 40};
            Rectangle kalkRect = {kalkPos.x, kalkPos.y, 100, 40};

            Rectangle bahis10Rect = {bahis10pos.x,bahis10pos.y,40,40};
            Rectangle bahis50Rect = {bahis50pos.x,bahis50pos.y,40,40};
            Rectangle bahis100Rect = {bahis100pos.x,bahis100pos.y,40,40};
            Rectangle bahiskoyRect = {bahiskoypos.x,bahiskoypos.y,100,40};
            Rectangle bahissifirlaRect = {bahissifirlapos.x,bahissifirlapos.y,100,40};

            struct oyuncu *cizilenOyuncu = &oyuncular[siradaki_oyuncu];

            if (cizilenOyuncu->bakiye > 0) {
                // PARA VARSA NORMAL BAHİS BUTONLARI
                DrawRectanglePro(bahis10Rect,kucukorigin,aci,RED);
                DrawRectanglePro(bahis50Rect,kucukorigin,aci,RED);
                DrawRectanglePro(bahis100Rect,kucukorigin,aci,RED);
                DrawRectanglePro(bahiskoyRect,buyukorigin,aci,GREEN);
                DrawRectanglePro(bahissifirlaRect,buyukorigin,aci,BLUE);

                YaziCizDondur("+10",bahis10Rect.x,bahis10Rect.y, aci, 14,WHITE);
                YaziCizDondur("+50",bahis50Rect.x,bahis50Rect.y, aci, 14,WHITE);
                YaziCizDondur("+100",bahis100Rect.x,bahis100Rect.y, aci, 14,WHITE);
                YaziCizDondur("Bahsi Koy",bahiskoyRect.x,bahiskoyRect.y, aci, 14,WHITE);
                YaziCizDondur("Sifirla",bahissifirlaRect.x,bahissifirlaRect.y, aci, 14,WHITE);
            }
            else {
                // PARA YOKSA İFLAS BUTONLARI
                DrawRectanglePro(borcAlRect, buyukorigin, aci, GOLD);
                DrawRectanglePro(kalkRect, buyukorigin, aci, MAROON);

                YaziCizDondur("Borc Al", borcAlRect.x, borcAlRect.y, aci, 14, BLACK);
                YaziCizDondur("Kalk", kalkRect.x, kalkRect.y, aci, 14, WHITE);

                // Uyarı yazısı
                YaziCizDondur("BAKIYE YETERSİZ!", btnMerkez.x, btnMerkez.y - 50, aci, 20, RED);
            }
        }
        else if (mevcutDurum == STATE_OYUNCU_TURU) {
            Vector2 pos = koltuk_konumlari[siradaki_oyuncu];
            float aci = koltuk_acilari[siradaki_oyuncu];
            float rad = aci * DEG2RAD;

            // Vektorel hesaplama
            float s = sinf(rad);
            float c = cosf(rad);
            Vector2 ileri = { -s, c };
            Vector2 sag   = { c, s };
            struct oyuncu* aktif_oyuncu = &oyuncular[siradaki_oyuncu];

            float yanalKayma = 0.0f;

                if (aktif_oyuncu->isSplitted == 1) {
                    if (aktif_oyuncu->sirasplittemi == 0) {
                        yanalKayma = -100.0f;
                    } else {
                        yanalKayma = 100.0f;
                    }
                }

                // 2. İki Farklı Merkez Belirle (ileri ve sag kullanılıyor)

                // ALT SIRA (Hit ve Stand)
                float altSiraUzaklik = 160.0f;
                Vector2 altMerkez = {
                    pos.x + (ileri.x * altSiraUzaklik) + (sag.x * yanalKayma),
                    pos.y + (ileri.y * altSiraUzaklik) + (sag.y * yanalKayma)
                };

                // ÜST SIRA (Split ve Double)
                float ustSiraUzaklik = 110.0f;
                Vector2 ustMerkez = {
                    pos.x + (ileri.x * ustSiraUzaklik) + (sag.x * yanalKayma),
                    pos.y + (ileri.y * ustSiraUzaklik) + (sag.y * yanalKayma)
                };

                float butonAraligi = 60.0f;

                // --- ALT SIRA BUTONLARI ---
                // HIT (Sol Alt)
                Vector2 hitPos = {
                    altMerkez.x - (sag.x * butonAraligi),
                    altMerkez.y - (sag.y * butonAraligi)
                };

                // STAND (Sağ Alt)
                Vector2 standPos = {
                    altMerkez.x + (sag.x * butonAraligi),
                    altMerkez.y + (sag.y * butonAraligi)
                };

                // --- ÜST SIRA BUTONLARI ---
                // SPLIT (Sol Üst)
                Vector2 splitPos = {
                    ustMerkez.x - (sag.x * butonAraligi),
                    ustMerkez.y - (sag.y * butonAraligi)
                };

                // DOUBLE (Sağ Üst)
                Vector2 doublePos = {
                    ustMerkez.x + (sag.x * butonAraligi),
                    ustMerkez.y + (sag.y * butonAraligi)
                };

                // Rectangle Tanımları
                Rectangle hitRect = { hitPos.x, hitPos.y, 100, 40 };
                Rectangle standRect = { standPos.x, standPos.y, 100, 40 };
                Rectangle doubleRect = { doublePos.x, doublePos.y, 100, 40 };
                Rectangle splitRect = { splitPos.x, splitPos.y, 100, 40 };
                Vector2 btnOrigin = { 50, 20 };
            // CIZIM

            DrawRectanglePro(hitRect, btnOrigin, aci, LIME);
            YaziCizDondur("HIT", hitPos.x, hitPos.y, aci, 20, BLACK);

            DrawRectanglePro(standRect, btnOrigin, aci, RED);
            YaziCizDondur("STAND", standPos.x, standPos.y, aci, 20, WHITE);

            if (oyuncular[siradaki_oyuncu].isSplitted == 0&&oyuncular[siradaki_oyuncu].el[0].konumx==oyuncular[siradaki_oyuncu].el[1].konumx) {
                DrawRectanglePro(splitRect, btnOrigin, aci,WHITE);
                YaziCizDondur("SPLIT", splitPos.x, splitPos.y, aci, 20, BLACK);
            }
            if (oyuncular[siradaki_oyuncu].kart_sayi == 2) {
                DrawRectanglePro(doubleRect, btnOrigin, aci, BLUE);
                YaziCizDondur("DOUBLE", doublePos.x, doublePos.y, aci, 20, WHITE);
            }
        } else if (mevcutDurum == STATE_SONUC) {
            char oyun_sonucu[30];
            DrawText(TextFormat("%s",oyun_sonucu),880,250,20,WHITE);
            DrawRectangleRec(tekrarOynaButton, BLUE);
            DrawText("TEKRAR OYNA", tekrarOynaButton.x + 40, tekrarOynaButton.y + 15, 20, WHITE);
        }
        EndDrawing();
    }
    UnloadTexture(cardSpriteSheet);
    UnloadSound(karistirmasesi);
    UnloadSound(kartcekmesesi); // Yuklenen efektleri temizle
    UnloadMusicStream(arkaplan); // Yuklenen muzigi temizle
    CloseAudioDevice();
    CloseWindow();
    return 0;
}