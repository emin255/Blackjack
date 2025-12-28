#ifndef BLACKJACK_BLACKJACK_H
#define BLACKJACK_BLACKJACK_H
struct kart {
    float mevcutKonumx;
    float mevcutkonumy;
    int vardimmi;
    int value;
    float konumx;
    float konumy;
    char suit[5];
    char isim[3];
};
struct oyuncu {
    int sirasplittemi;
    int splitValue;
    char splitsonuc[30];
    int isSplitted;
    char sonuc[30];
    int isActive;
    int bahis;
    int bakiye;
    int isDoubled;
    int value ;
    int kart_sayi;
    struct kart el[5];
    struct kart splitEl[5];
    int splitkartsayi;
};
void kazanan(struct oyuncu* oyuncu, struct oyuncu* krupiyer);
void deste_olustur(struct kart deste[52]);
void uzundesteyikaristir(struct kart deste[]);
int oyuncu_el_degeri(struct kart el[],int kartsayisi);
void desteyi_karistir(struct kart deste[]);
void kart_cek(int *oyuncu_kart_sayi,struct kart el[], struct kart deste[],int* kart_sayisi);

#endif //BLACKJACK_BLACKJACK_H