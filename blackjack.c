#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "blackjack.h"
int kazanan(struct oyuncu* oyuncu,struct oyuncu* krupiyer) {
    // Krupiyer kazanırsa 0
    // Oyuncu kazanırsa 1
    // Oyuncu patlarsa 2
    // Krupiyer patlarsa 4
    // Denkse 3
    if (oyuncu_el_degeri(oyuncu)>21) {
        return 2;
    }
    if (oyuncu_el_degeri(krupiyer)>21) {
        return 4;
    }
    if (oyuncu_el_degeri(oyuncu)==oyuncu_el_degeri(krupiyer)) {
        return 3;
    }
    if (oyuncu_el_degeri(oyuncu)==21&&oyuncu_el_degeri(krupiyer)!=21&&oyuncu->kart_sayi == 2) {
        return 1;
    }
    if (oyuncu_el_degeri(oyuncu)<oyuncu_el_degeri(krupiyer)) {
        return 0;
    }
    if (oyuncu_el_degeri(oyuncu)>oyuncu_el_degeri(krupiyer)) {
        return 1;
    }
}
void deste_olustur(struct kart deste[52]){
    char *turler[] = {"kupa","karo","sinek","maca"};
    int hedef;
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 13; i++) {
            hedef = j*13+i;
            if (i==0) {
                deste[hedef].value=11;
                deste[hedef].konumx=0;
                deste[hedef].konumy=j;

            }else if(i<10){
                deste[hedef].value=i+1;
                deste[hedef].konumx=i;
                deste[hedef].konumy=j;

            }
            else {
                deste[hedef].value=10;
            }
            if (i==10) {
                deste[hedef].konumx=i;
                deste[hedef].konumy=j;
            }
            if (i==11) {
                deste[hedef].konumx=i;
                deste[hedef].konumy=j;
            }
            if (i==12) {
                deste[hedef].konumx=i;
                deste[hedef].konumy=j;
            }

            strcpy(deste[hedef].suit,turler[j]);
            if (i==0) {
                strcpy(deste[hedef].isim,"A");
            }else if (i<10) {
                sprintf(deste[hedef].isim,"%d",i+1);
            }else if (i==10) {
                strcpy(deste[hedef].isim,"J");
            }
            else if (i==11) {
                strcpy(deste[hedef].isim,"Q");
            }
            else{
                strcpy(deste[hedef].isim,"K");
            }
        }
    }
}
int oyuncu_el_degeri(struct oyuncu* oyuncu) {
    int el = 0;
    int as_sayisi = 0;
    for (int j = 0; j < oyuncu->kart_sayi; j++) {
        el += oyuncu->el[j].value;
        if (oyuncu->el[j].isim[0] == 'A') {
            as_sayisi++;
        }
    }
    while (el>21&&as_sayisi>0) {
        el = el - 10;
        as_sayisi--;
    }
    printf("Oyuncunun el değeri %d",el);
    return el;
}
void desteyi_karistir(struct kart deste[]) {
    srand(time(NULL));
    for (int j = 51; j > 0; j--) {
        int a = rand()%(j+1);

        struct kart gecici = deste[a];
        deste[a] = deste[j];
        deste[j] = gecici;
    }
}

void kart_cek(struct oyuncu* oyuncu, struct kart deste[],int* kart_sayisi) {
    oyuncu->el[oyuncu->kart_sayi] = deste[*kart_sayisi];
    (*kart_sayisi)++;
    oyuncu->kart_sayi++;

}
