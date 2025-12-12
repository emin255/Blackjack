

#ifndef BLACKJACK_BLACKJACK_H
#define BLACKJACK_BLACKJACK_H
struct kart {
    int value;
    float konumx;
    float konumy;
    char suit[5];
    char isim[3];
};
struct oyuncu {
    int bakiye;
    int value ;
    int kart_sayi;
    struct kart el[5];
};
int kazanan(struct oyuncu* oyuncu, struct oyuncu* krupiyer);
void deste_olustur(struct kart deste[52]);
int oyuncu_el_degeri(struct oyuncu* oyuncu);
void desteyi_karistir(struct kart deste[]);
void kart_cek(struct oyuncu* oyuncu, struct kart deste[],int* kart_sayisi);

#endif //BLACKJACK_BLACKJACK_H