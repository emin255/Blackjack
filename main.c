#include "raylib.h"
#include "blackjack.h"

const float CARD_WIDTH = 84.0f;
const float CARD_HEIGHT = 120.0f;
const Rectangle hitButton = { 100, 600, 100, 50 };
const Rectangle standButton = { 220, 600, 100, 50 };
const Rectangle tekrarOynaButton = { 100, 600, 220, 50 };
const Rectangle bahis10arttirbutton = { 100, 600, 100, 50 };
const Rectangle bahis50arttirbutton = { 210, 600, 100, 50 };
const Rectangle bahis100arttirbutton = { 320, 600, 100, 50 };
const Rectangle bahissifirlabutton = { 430, 600, 100, 50 };
const Rectangle bahiskoy = { 540, 600, 100, 50 };
typedef enum{
    STATE_BAHİS,         // Bahis Zamanı
    STATE_KART_DAGIT,    // El başlıyor
    STATE_OYUNCU_TURU,   // Oyuncu Hit/Stand bekliyor
    STATE_KASA_TURU,     // Kasa oynuyor
    STATE_SONUC
}GAME_STATE;
Rectangle kart_degerini_al(struct kart* kart) {
    return (Rectangle){ (kart->konumx)*CARD_WIDTH, kart->konumy*CARD_HEIGHT, CARD_WIDTH, CARD_HEIGHT };
}
void yeni_el(struct oyuncu* oyuncu, struct oyuncu* krupiyer, struct kart* deste, int* kart_sayisi) {
    krupiyer->kart_sayi = 0;
    oyuncu->kart_sayi = 0;
    krupiyer->el[0] = deste[*kart_sayisi];
    (*kart_sayisi)++;
    oyuncu->el[0] = deste[*kart_sayisi];
    (*kart_sayisi)++;
    krupiyer->el[1] = deste[*kart_sayisi];
    (*kart_sayisi)++;
    oyuncu->el[1] = deste[*kart_sayisi];
    (*kart_sayisi)++;
    krupiyer->kart_sayi += 2;
    oyuncu->kart_sayi += 2;
}
void el_ciz(struct oyuncu* oyuncu,Texture2D spritesheet,Vector2 vector2,int gizle) {
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
void kartlarbittimi(int *kart_sayisi,struct kart* deste) {
    if (*kart_sayisi >42) {
        *kart_sayisi = 0;
        deste_olustur(deste);
        desteyi_karistir(deste);
    }
}

int main(void)
{
    int bahis = 0;
    char oyun_sonucu[30];
    double kasaCekmeZamani = 0.0;
    const double kasaBeklemeSuresi = 1.0;
    int kart_sayisi = 0;
    int kart_kapali_mi = 0;
    const int screenWidth = 1900;
    const int screenHeight = 1000;
    GAME_STATE mevcutDurum = STATE_BAHİS;
    int hesaplandi_mi = 0;
    struct kart deste[52];
    struct oyuncu krupiyer;
    struct oyuncu oyuncu;
    deste_olustur(deste);
    desteyi_karistir(deste);
    oyuncu.bakiye = 1000;
    oyuncu.value = 0;
    InitWindow(screenWidth, screenHeight, "Raylib - Blackjack");
    InitAudioDevice();
    oyuncu.kart_sayi = 0;
    krupiyer.kart_sayi = 0;

    SetTargetFPS(60);
    Texture2D cardSpriteSheet = LoadTexture("cards.png");
    Sound kartcekmesesi = LoadSound("ses.ogg");
    Music arkaplan = LoadMusicStream("arkaplan.ogg");
    PlayMusicStream(arkaplan);

    if (cardSpriteSheet.id == 0) {
        TraceLog(LOG_FATAL, "HATA: 'cards.png' yüklenemedi! .exe'nin yanına kopyaladığınızdan emin olun.");
        CloseWindow();
        return -1;
    }
    while (!WindowShouldClose())
    {
        UpdateMusicStream(arkaplan);
        Vector2 mousepos = GetMousePosition();
        switch (mevcutDurum) {
            case STATE_BAHİS:
                hesaplandi_mi = 0;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (CheckCollisionPointRec(mousepos, bahis10arttirbutton)) {
                        bahis += 10;
                    }
                    if (CheckCollisionPointRec(mousepos, bahis50arttirbutton)) {
                        bahis += 50;
                    }
                    if (CheckCollisionPointRec(mousepos, bahis100arttirbutton)) {
                        bahis += 100;
                    }
                    if (CheckCollisionPointRec(mousepos, bahiskoy)) {
                        if (bahis<=oyuncu.bakiye&&bahis>0) {
                            oyuncu.bakiye -= bahis;
                            mevcutDurum = STATE_KART_DAGIT;
                        }
                        else {
                            bahis = 0;
                        }
                    }
                    if (CheckCollisionPointRec(mousepos, bahissifirlabutton)) {
                        bahis = 0;
                    }
                }
                break;
            case STATE_KART_DAGIT:
                strcpy(oyun_sonucu,"      ");
                kartlarbittimi(&kart_sayisi, deste);
                kart_kapali_mi = 0;
                yeni_el(&oyuncu,&krupiyer,deste,&kart_sayisi);
                int deger = oyuncu_el_degeri(&oyuncu);
                oyuncu.value = deger;
                mevcutDurum = STATE_OYUNCU_TURU;
                break;
            case STATE_OYUNCU_TURU:
                if (oyuncu.value == 21) {
                    mevcutDurum = STATE_KASA_TURU;
                    kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                }
                if (oyuncu.value >21) {
                    mevcutDurum = STATE_SONUC;
                }
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mousepos, hitButton)&&oyuncu_el_degeri(&oyuncu)<21) {
                    kart_cek(&oyuncu,deste,&kart_sayisi);
                    PlaySound(kartcekmesesi);
                    oyuncu.value = oyuncu_el_degeri(&oyuncu);
                }
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mousepos, standButton)) {
                    mevcutDurum = STATE_KASA_TURU;
                    kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                }
                break;
            case STATE_KASA_TURU:
                kart_kapali_mi = 1;
                int kasa_durumu = oyuncu_el_degeri(&krupiyer);
                if(kasa_durumu<17) {
                    if (GetTime() > kasaCekmeZamani) {
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
            case STATE_SONUC:
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousepos, tekrarOynaButton)) {
                        mevcutDurum = STATE_BAHİS; // Tekrar başlat!
                        bahis = 0;
                        oyuncu.value = 0;
                        hesaplandi_mi = 0;
                    }
                }
                if (hesaplandi_mi == 0) {
                    int sonuc = kazanan(&oyuncu,&krupiyer);
                    if (sonuc==0) {
                        strcpy(oyun_sonucu,"kasa kazandi");
                    }
                    else if (sonuc==1) {
                        strcpy(oyun_sonucu,"oyuncu kazandi");
                        oyuncu.bakiye += bahis*2;
                    }
                    else if (sonuc==2) {
                        strcpy(oyun_sonucu,"oyuncu patladi");
                    }
                    else if (sonuc==3) {
                        strcpy(oyun_sonucu,"berabere");
                        oyuncu.bakiye += bahis;
                    }
                    else if (sonuc==4) {
                        strcpy(oyun_sonucu,"Kasa patladı");
                        oyuncu.bakiye += bahis*2;
                    }
                    hesaplandi_mi = 1;
                }
                break;
        }
        BeginDrawing();
        ClearBackground(DARKGREEN); // Casino masası rengi :)

        el_ciz(&oyuncu,cardSpriteSheet,(Vector2){100,400},1);
        el_ciz(&krupiyer,cardSpriteSheet,(Vector2){100,200},kart_kapali_mi);

        DrawText(TextFormat("Skor %d",oyuncu.value),100,550,20,WHITE);
        if (mevcutDurum == STATE_BAHİS) {
            DrawRectangleRec(bahis10arttirbutton, LIME);
            DrawText("+10", 35 + bahis10arttirbutton.x, 15 + bahis10arttirbutton.y, 20, BLACK);
            DrawRectangleRec(bahis50arttirbutton, RED);
            DrawText("+50", bahis50arttirbutton.x + 15 , bahis50arttirbutton.y + 15, 20, BLACK);
            DrawRectangleRec(bahis100arttirbutton, RED);
            DrawText("+100", bahis100arttirbutton.x+15, bahis100arttirbutton.y + 15, 20, BLACK);
            DrawRectangleRec(bahissifirlabutton, RED);
            DrawText("bahsi sifirla", bahissifirlabutton.x , bahissifirlabutton.y +15, 20, BLACK);
            DrawRectangleRec(bahiskoy, RED);
            DrawText("bahsi koy", bahiskoy.x , bahiskoy.y + 15, 20, BLACK);
            DrawText(TextFormat("BAHIS = %d",bahis),100.0f,800.0f,20.0f,WHITE);
            DrawText(TextFormat("BAKIYE = %d",oyuncu.bakiye),100.0f,840.0f,20.0f,WHITE);
        }
        else if (mevcutDurum == STATE_OYUNCU_TURU) {
            DrawRectangleRec(hitButton, LIME);
            DrawText("HIT", 35.0f + hitButton.x, 15.0f + hitButton.y, 20, BLACK);
            DrawRectangleRec(standButton, RED);
            DrawText("STAND", standButton.x + 25, standButton.y + 15, 20, BLACK);
            DrawText(TextFormat("BAHIS = %d",bahis),100.0f,800.0f,20.0f,WHITE);
            DrawText(TextFormat("BAKIYE = %d",oyuncu.bakiye),100.0f,840.0f,20.0f,WHITE);
        } else if (mevcutDurum == STATE_SONUC) {
            DrawText(TextFormat("%s",oyun_sonucu),100,360,20,WHITE);
            DrawRectangleRec(tekrarOynaButton, BLUE);
            DrawText("TEKRAR OYNA", tekrarOynaButton.x + 40, tekrarOynaButton.y + 15, 20, WHITE);
            DrawText(TextFormat("BAHIS = %d",bahis),100.0f,800.0f,20.0f,WHITE);
            DrawText(TextFormat("BAKIYE = %d",oyuncu.bakiye),100.0f,840.0f,20.0f,WHITE);
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


