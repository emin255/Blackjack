#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct kart {
    int value;
    char suit[5];
    char isim[3];
};
struct oyuncu {
    int value;
    int kart_sayisi;
    struct kart el[5];
};
void deste_olustur(struct kart deste[52]) {
    char *turler[] = {"maca","kupa","sinek","karo"};
    int hedef;
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 13; i++) {
            hedef = j*13+i;
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
void oyuncu_el(struct oyuncu oyuncu){
    for (int j = 0; j < oyuncu.kart_sayisi; j++) {
        printf("%s %s ",oyuncu.el[j].suit,oyuncu.el[j].isim);
    }
}
int oyuncu_el_degeri(struct oyuncu oyuncu) {
    int el = 0;
    int as_sayisi = 0;
    for (int j = 0; j < oyuncu.kart_sayisi; j++) {
        el += oyuncu.el[j].value;
        if (oyuncu.el[j].isim[0] == 'A') {
            as_sayisi++;
        }
    }
    while (el>21&&as_sayisi>0) {
        el = el - 10;
    }
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
/*int main(void) {
    setlocale(LC_ALL, "Turkish");
    int oyun_devam;
    char devam;
    printf("Blackjack uygulamasina hosgeldiniz \n");
    printf("Oyundan cikmak icin q tusuna, baslamak icin ise p tusuna basiniz \n");
    scanf("%c",&devam);
    if (devam == 'p') {
        oyun_devam = 1;
    }
    else if (devam == 'q') {
        oyun_devam = 0;
        printf("Blackjack oyunundan cikiliyor");
    }
    struct kart deste[52];
    deste_olustur(deste);
    desteyi_karistir(deste);
    int kart_sayısı=0;
    struct oyuncu krupiyer;
    struct oyuncu oyuncu;
    while (oyun_devam==1) {
        krupiyer.kart_sayisi = 0;
        oyuncu.kart_sayisi = 0;
        krupiyer.el[0] = deste[kart_sayısı];
        kart_sayısı++;
        oyuncu.el[0] = deste[kart_sayısı];
        kart_sayısı++;
        krupiyer.el[1] = deste[kart_sayısı];
        kart_sayısı++;
        oyuncu.el[1] = deste[kart_sayısı];
        kart_sayısı++;
        krupiyer.kart_sayisi+= 2;
        oyuncu.kart_sayisi += 2;
        printf("Krupiyer \n");
        printf("%s %s Kapali kart",krupiyer.el[0].suit,krupiyer.el[0].isim);
        printf("\n");
        printf("%s %s %s %s  Kartlarin toplam degeri %d \n",oyuncu.el[0].isim,oyuncu.el[0].suit,oyuncu.el[1].isim,oyuncu.el[1].suit, oyuncu_el_degeri(oyuncu));
        while (oyuncu_el_degeri(oyuncu)<21) {
            int el_durumu;
            printf("\nKart cekmek mi istersiniz yoksa kalmak mi(cekmek icin 1 kalmak icin 0 yaziniz)\n");
            scanf("%d",&el_durumu);
            if (el_durumu==1) {
                oyuncu.el[oyuncu.kart_sayisi] = deste[kart_sayısı];
                kart_sayısı++;
                oyuncu.kart_sayisi++;
                oyuncu_el(oyuncu);
                printf("  =  ");
                printf("%d",oyuncu_el_degeri(oyuncu));
                printf("\n");
            }
            else if (el_durumu==0) {
                break;
                printf("\n");
            }
        }
        if (oyuncu_el_degeri(oyuncu)>21) {
            printf("Krupiyerin eli  ");
            oyuncu_el(krupiyer);
            printf("  =  ");
            printf("%d \n",oyuncu_el_degeri(krupiyer));
            printf("Krupiyer kazandi \n");
        }else if (oyuncu_el_degeri(oyuncu)==21&& oyuncu_el_degeri(krupiyer)!=21){
            printf("Krupiyerin eli  ");
            oyuncu_el(krupiyer);
            printf("  =  ");
            printf("%d",oyuncu_el_degeri(krupiyer));
            printf("\n");
            printf("Oyuncu kazandı");
        }else {
            int a=0;
            while (oyuncu_el_degeri(krupiyer)<17) {
                printf("\nKrupiyerin eli  ");
                oyuncu_el(krupiyer);
                printf("  =  ");
                printf("%d \n",oyuncu_el_degeri(krupiyer));
                printf("\nKrupiyer Kart cekiyor\n");
                krupiyer.el[krupiyer.kart_sayisi] = deste[kart_sayısı];
                kart_sayısı++;
                krupiyer.kart_sayisi++;
                oyuncu_el(krupiyer);
                printf("  =  ");
                printf("%d",oyuncu_el_degeri(krupiyer));
                printf("\n");
                a++;
            }
            if (oyuncu_el_degeri(krupiyer)>21&&oyuncu_el_degeri(oyuncu)<21) {
                printf("\nKrupiyerin eli  ");
                oyuncu_el(krupiyer);
                printf("  =  ");
                printf("%d \n",oyuncu_el_degeri(krupiyer));
                printf("Oyuncun kazandı");

            }
            else if (oyuncu_el_degeri(krupiyer)<oyuncu_el_degeri(oyuncu)) {
                printf("\nKrupiyerin eli  ");
                oyuncu_el(krupiyer);
                printf("  =  ");
                printf("%d \n",oyuncu_el_degeri(krupiyer));
                printf("Oyuncu kazandi\n");
            }else if (oyuncu_el_degeri(krupiyer)>oyuncu_el_degeri(oyuncu)) {
                printf("\nKrupiyerin eli  ");
                oyuncu_el(krupiyer);
                printf("  =  ");
                printf("%d \n",oyuncu_el_degeri(krupiyer));
                printf("Krupiyer kazandi\n");
            }else if (oyuncu_el_degeri(krupiyer)==oyuncu_el_degeri(oyuncu)) {
                printf("\nKrupiyerin eli  ");
                oyuncu_el(krupiyer);
                printf("  =  ");
                printf("%d \n",oyuncu_el_degeri(krupiyer));
                printf("Krupiyerle oyuncu degerleri esit BERABERE\n");
            }
        }
        char oyun_devamı;
        printf("Blackjack oynamaya devam etmek istiyor musunuz?(devam icin p, kapatmak icin q)\n");
        scanf(" %c",&oyun_devamı);
        if (oyun_devamı == 'q') {
            oyun_devam = 0;
        }else {
            printf(" Blackjack oyunu devam ediyor \n");
        }
    }
    printf("Blackjack oyunundan cikiliyor... \n");
    return 0;*/
