#include "raylib.h"
#include "blackjack.h"

const float CARD_WIDTH = 84.0f;
const float CARD_HEIGHT = 120.0f;
const Rectangle hitButton = { 100, 600, 100, 50 };
const Rectangle standButton = { 220, 600, 100, 50 };
const Rectangle tekrarOynaButton = { 100, 600, 220, 50 };
typedef enum{
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
    if (*kart_sayisi >52) {
        *kart_sayisi = 0;
        deste_olustur(deste);
        desteyi_karistir(deste);
    }
}
int main(void)
{
    char oyun_sonucu[30];
    double kasaCekmeZamani = 0.0;
    const double kasaBeklemeSuresi = 1.0;
    int kart_sayisi = 0;
    int kart_kapali_mi = 0;
    const int screenWidth = 1900;
    const int screenHeight = 1000;
    GAME_STATE mevcutDurum = STATE_KART_DAGIT;

    struct kart deste[52];
    struct oyuncu krupiyer;
    struct oyuncu oyuncu;
    deste_olustur(deste);
    desteyi_karistir(deste);

    InitWindow(screenWidth, screenHeight, "Raylib - Blackjack");
    InitAudioDevice();

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
            case STATE_KART_DAGIT:
                if (kart_sayisi >52) {
                    kartlarbittimi(&kart_sayisi, deste);
                    kart_sayisi = 0;
                    deste_olustur(deste);
                    desteyi_karistir(deste);
                }
                kart_kapali_mi = 0;
                yeni_el(&oyuncu,&krupiyer,deste,&kart_sayisi);
                int deger = oyuncu_el_degeri(&oyuncu);
                oyuncu.value = deger;
                mevcutDurum = STATE_OYUNCU_TURU;
                break;
            case STATE_OYUNCU_TURU:
                if (deger == 21) {
                    mevcutDurum = STATE_KASA_TURU;
                    kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                    break;
                }
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mousepos, hitButton)&&oyuncu_el_degeri(&oyuncu)<21) {
                    kartlarbittimi(&kart_sayisi, deste);
                    kart_cek(&oyuncu,deste,&kart_sayisi);
                    PlaySound(kartcekmesesi);
                    oyuncu.value = oyuncu_el_degeri(&oyuncu);
                    if (oyuncu.value >21) {
                        mevcutDurum = STATE_SONUC;
                    }
                }
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(mousepos, standButton)) {
                    kartlarbittimi(&kart_sayisi, deste);
                    mevcutDurum = STATE_KASA_TURU;
                    kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                }
                break;
            case STATE_KASA_TURU:
                kart_kapali_mi = 1;
                int kasa_durumu = oyuncu_el_degeri(&krupiyer);

                if(kasa_durumu<17&&oyuncu_el_degeri(&oyuncu)<=21) {
                    if (GetTime() > kasaCekmeZamani) {
                        kartlarbittimi(&kart_sayisi, deste);
                        PlaySound(kartcekmesesi);
                        kart_cek(&krupiyer,deste,&kart_sayisi);
                        kasa_durumu = oyuncu_el_degeri(&krupiyer);
                        kasaCekmeZamani = GetTime() + kasaBeklemeSuresi;
                    }
                }
                if (kasa_durumu>17&&oyuncu_el_degeri(&oyuncu)<=21) {
                    mevcutDurum = STATE_SONUC;
                    break;
                }
            case STATE_SONUC:
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousepos, tekrarOynaButton)) {
                        mevcutDurum = STATE_KART_DAGIT; // Tekrar başlat!
                    }
                }
                break;
        }
        BeginDrawing();
        ClearBackground(DARKGREEN); // Casino masası rengi :)

        el_ciz(&oyuncu,cardSpriteSheet,(Vector2){100,400},1);
        el_ciz(&krupiyer,cardSpriteSheet,(Vector2){100,200},kart_kapali_mi);

        DrawText(TextFormat("Skor %d",oyuncu.value),100,550,20,WHITE);

        if (mevcutDurum == STATE_OYUNCU_TURU) {
            DrawRectangleRec(hitButton, LIME);
            DrawText("HIT", 35.0f + hitButton.x, 15.0f + hitButton.y, 20, BLACK);
            DrawRectangleRec(standButton, RED);
            DrawText("STAND", standButton.x + 25, standButton.y + 15, 20, BLACK);
        } else if (mevcutDurum == STATE_SONUC) {
            // (Kazananı buraya yaz)
            if (kazanan(&oyuncu,&krupiyer)==0) {
                strcpy(oyun_sonucu,"kasa kazandi");
            }
            else if (kazanan(&oyuncu,&krupiyer)==1) {
                strcpy(oyun_sonucu,"oyuncu kazandi");
            }
            else if (kazanan(&oyuncu,&krupiyer)==2) {
                strcpy(oyun_sonucu,"oyuncu patladi");
            }
            else if (kazanan(&oyuncu,&krupiyer)==3) {
                strcpy(oyun_sonucu,"berabere");
            }
            else if (kazanan(&oyuncu,&krupiyer)==4) {
                strcpy(oyun_sonucu,"Kasa patladı");
            }
            DrawText(TextFormat("%s",oyun_sonucu),100,360,20,WHITE);
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
