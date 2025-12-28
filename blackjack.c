#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "blackjack.h"
void kazanan(struct oyuncu* oyuncu,struct oyuncu* krupiyer) {
    // Krupiyer kazanırsa 0
    // Oyuncu kazanırsa 1
    // Oyuncu patlarsa 2
    // Krupiyer patlarsa 4
    // Denkse 3
    int krupiyervalue = oyuncu_el_degeri(krupiyer->el,krupiyer->kart_sayi);
    int oyuncuvalue = oyuncu_el_degeri(oyuncu->el,oyuncu->kart_sayi);
    int oyuncusplitValue = oyuncu_el_degeri(krupiyer->splitEl,oyuncu->splitkartsayi);
    if (oyuncu->isSplitted==1) {
        if (oyuncusplitValue>21) {
            strcpy(oyuncu->splitsonuc,"oyuncu patladi");
        }
        else if (krupiyervalue>21) {
            strcpy(oyuncu->splitsonuc,"Kasa patladi");
            oyuncu->bakiye += oyuncu->bahis*2;
        }
        else if (oyuncusplitValue == 21&&krupiyervalue!=21&&oyuncu->splitkartsayi == 2) {
            strcpy(oyuncu->splitsonuc,"oyuncu kazandi");
            oyuncu->bakiye += oyuncu->bahis*2;
        }
        else if (oyuncusplitValue<krupiyervalue) {
            strcpy(oyuncu->splitsonuc,"kasa kazandi");
        }
        else if (oyuncusplitValue>krupiyervalue) {
            strcpy(oyuncu->splitsonuc,"oyuncu kazandi");
            oyuncu->bakiye += oyuncu->bahis*2;
        }else if (oyuncusplitValue==krupiyervalue&&krupiyervalue<21&&oyuncuvalue<21) {
            strcpy(oyuncu->splitsonuc,"berabere");
            oyuncu->bakiye += oyuncu->bahis;
        }
    }
    if (oyuncu_el_degeri(oyuncu->el,oyuncu->kart_sayi)>21) {
        strcpy(oyuncu->sonuc,"oyuncu patladi");
    }
    else if (oyuncu_el_degeri(krupiyer->el,oyuncu->kart_sayi)>21) {
        strcpy(oyuncu->sonuc,"Kasa patladi"); // duzeltildi
        oyuncu->bakiye += oyuncu->bahis*2;
    }
    else if (oyuncu_el_degeri(oyuncu->el,oyuncu->kart_sayi)==21&&oyuncu_el_degeri(krupiyer->el,krupiyer->kart_sayi)!=21&&oyuncu->kart_sayi == 2) {
        strcpy(oyuncu->sonuc,"oyuncu kazandi");
        oyuncu->bakiye += oyuncu->bahis*2;
    }
    else if (oyuncu_el_degeri(oyuncu->el,oyuncu->kart_sayi)<oyuncu_el_degeri(krupiyer->el,krupiyer->kart_sayi)) {
        strcpy(oyuncu->sonuc,"kasa kazandi");
    }
    else if (oyuncu_el_degeri(oyuncu->el,oyuncu->kart_sayi)>oyuncu_el_degeri(krupiyer->el,krupiyer->kart_sayi)) {
        strcpy(oyuncu->sonuc,"oyuncu kazandi");
        oyuncu->bakiye += oyuncu->bahis*2;
    }else {
        strcpy(oyuncu->sonuc,"berabere");
        oyuncu->bakiye += oyuncu->bahis;
    }


}
void deste_olustur(struct kart deste[52]){
    char *turler[] = {"kupa","karo","sinek","maca"};
    int hedef;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 13; x++) {
            hedef = y*13+x;
            deste[hedef].konumx = x;
            deste[hedef].konumy = y;
        }
    }
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 13; i++) {
            hedef = j*13+i;
            deste[hedef].mevcutKonumx = 600;
            deste[hedef].mevcutkonumy = 150;
            deste[hedef].vardimmi = 0;
            if (i==0) {
                deste[hedef].value=11;
            }else if(i<10){
                deste[hedef].value=i+1;
            }
            else {
                deste[hedef].value=10;
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
int oyuncu_el_degeri(struct kart el[],int kartsayisi) {
    int deger = 0;
    int as_sayisi = 0;
    for (int j = 0; j < kartsayisi; j++) {
        deger += el[j].value;
        if (el[j].isim[0] == 'A') {
            as_sayisi++;
        }
    }
    while (deger>21&&as_sayisi>0) {
        deger = deger - 10;
        as_sayisi--;
    }
    return deger;
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

void kart_cek(int *oyuncu_kart_sayi,struct kart el[], struct kart deste[],int* kart_sayisi) {
    el[*oyuncu_kart_sayi] = deste[*kart_sayisi];
    (*kart_sayisi)++;
    (*oyuncu_kart_sayi)++;
}
void uzundesteyikaristir(struct kart deste[])
{
    srand(time(NULL));
    for (int j = 363; j > 0; j--) {
        int a = rand()%(j+1);

        struct kart gecici = deste[a];
        deste[a] = deste[j];
        deste[j] = gecici;
    }
}
