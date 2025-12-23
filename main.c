#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yapilar.h"

// --- YARDIMCI FONKSİYONLAR ---

int maksimum(int a, int b) { return (a > b) ? a : b; }
int yukseklikAl(IntervalNode *n) { return (n == NULL) ? 0 : n->yukseklik; }

void maxBitisGuncelle(IntervalNode *n) {
    if (n == NULL) return;
    int m = n->veri->bitis;
    if (n->sol != NULL && n->sol->max_bitis > m) m = n->sol->max_bitis;
    if (n->sag != NULL && n->sag->max_bitis > m) m = n->sag->max_bitis;
    n->max_bitis = m;
}

int dengeFaktoruAl(IntervalNode *n) {
    return (n == NULL) ? 0 : yukseklikAl(n->sol) - yukseklikAl(n->sag);
}

// --- ROTASYONLAR VE INTERVAL TREE MANTIĞI ---

IntervalNode* sagaDondur(IntervalNode *y) {
    IntervalNode *x = y->sol;
    IntervalNode *T2 = x->sag;
    x->sag = y;
    y->sol = T2;
    y->yukseklik = maksimum(yukseklikAl(y->sol), yukseklikAl(y->sag)) + 1;
    x->yukseklik = maksimum(yukseklikAl(x->sol), yukseklikAl(x->sag)) + 1;
    maxBitisGuncelle(y);
    maxBitisGuncelle(x);
    return x;
}

IntervalNode* solaDondur(IntervalNode *x) {
    IntervalNode *y = x->sag;
    IntervalNode *T2 = y->sol;
    y->sol = x;
    x->sag = T2;
    x->yukseklik = maksimum(yukseklikAl(x->sol), yukseklikAl(x->sag)) + 1;
    y->yukseklik = maksimum(yukseklikAl(y->sol), yukseklikAl(y->sag)) + 1;
    maxBitisGuncelle(x);
    maxBitisGuncelle(y);
    return y;
}

int cakismaVarMi(Randevu r1, Randevu r2) {
    return (r1.baslangic < r2.bitis && r2.baslangic < r1.bitis);
}

IntervalNode* dugumOlustur(Randevu r) {
    IntervalNode* yeni = (IntervalNode*)malloc(sizeof(IntervalNode));
    yeni->veri = (Randevu*)malloc(sizeof(Randevu));
    *(yeni->veri) = r;
    yeni->max_bitis = r.bitis;
    yeni->sol = yeni->sag = NULL;
    yeni->yukseklik = 1;
    return yeni;
}

IntervalNode* randevuEkle(IntervalNode* kok, Randevu r, int sessizMod) {
    if (kok == NULL) return dugumOlustur(r);

    if (cakismaVarMi(r, *(kok->veri))) {
        if (!sessizMod) printf("\aHATA: ID %d ile çakışma var! (%02d:%02d)\n", 
                               kok->veri->id, kok->veri->baslangic/60, kok->veri->baslangic%60);
        return kok;
    }

    if (r.baslangic < kok->veri->baslangic)
        kok->sol = randevuEkle(kok->sol, r, sessizMod);
    else
        kok->sag = randevuEkle(kok->sag, r, sessizMod);

    kok->yukseklik = 1 + maksimum(yukseklikAl(kok->sol), yukseklikAl(kok->sag));
    maxBitisGuncelle(kok);

    int denge = dengeFaktoruAl(kok);
    if (denge > 1 && r.baslangic < kok->sol->veri->baslangic) return sagaDondur(kok);
    if (denge < -1 && r.baslangic > kok->sag->veri->baslangic) return solaDondur(kok);
    if (denge > 1 && r.baslangic > kok->sol->veri->baslangic) {
        kok->sol = solaDondur(kok->sol);
        return sagaDondur(kok);
    }
    if (denge < -1 && r.baslangic < kok->sag->veri->baslangic) {
        kok->sag = sagaDondur(kok->sag);
        return solaDondur(kok);
    }
    return kok;
}

// --- DOSYA VE SIRALAMA (QUICKSORT) ---

void csvKaydet(Randevu r) {
    FILE *fp = fopen("randevular.csv", "a");
    if (fp != NULL) {
        fprintf(fp, "%d,%s,%d,%d\n", r.id, r.isim, r.baslangic, r.bitis);
        fclose(fp);
    }
}

void csvdenYukle(IntervalNode** kok) {
    FILE *fp = fopen("randevular.csv", "r");
    if (fp == NULL) return;
    Randevu r;
    while (fscanf(fp, "%d,%[^,],%d,%d\n", &r.id, r.isim, &r.baslangic, &r.bitis) != EOF) {
        *kok = randevuEkle(*kok, r, 1); // Yüklerken hata mesajı basmasın
    }
    fclose(fp);
}

void quicksort(Randevu dizi[], int dusuk, int yuksek) {
    if (dusuk < yuksek) {
        int pivot = dizi[yuksek].id;
        int i = dusuk - 1;
        for (int j = dusuk; j <= yuksek - 1; j++) {
            if (dizi[j].id < pivot) {
                i++;
                Randevu temp = dizi[i]; dizi[i] = dizi[j]; dizi[j] = temp;
            }
        }
        Randevu temp = dizi[i+1]; dizi[i+1] = dizi[yuksek]; dizi[yuksek] = temp;
        int pi = i + 1;
        quicksort(dizi, dusuk, pi - 1);
        quicksort(dizi, pi + 1, yuksek);
    }
}
// ID'ye göre Binary Search (İkili Arama) Algoritması
int binarySearchID(Randevu dizi[], int dusuk, int yuksek, int arananID) {
    while (dusuk <= yuksek) {
        int orta = dusuk + (yuksek - dusuk) / 2;

        // ID bulundu mu?
        if (dizi[orta].id == arananID)
            return orta;

        // ID daha büyükse sağ tarafa bak
        if (dizi[orta].id < arananID)
            dusuk = orta + 1;
        // ID daha küçükse sol tarafa bak
        else
            yuksek = orta - 1;
    }
    return -1; // Bulunamadı
}
int diziyeAktar(IntervalNode *kok, Randevu dizi[], int index) {
    if (kok == NULL) return index;
    index = diziyeAktar(kok->sol, dizi, index);
    dizi[index++] = *(kok->veri);
    index = diziyeAktar(kok->sag, dizi, index);
    return index;
}
void jsonKaydet(IntervalNode *kok, FILE *fp, int *ilk) {
    if (kok == NULL) return;
    jsonKaydet(kok->sol, fp, ilk);
    
    if (!(*ilk)) fprintf(fp, ",\n");
    fprintf(fp, "  {\"id\": %d, \"isim\": \"%s\", \"baslangic\": %d, \"bitis\": %d}", 
            kok->veri->id, kok->veri->isim, kok->veri->baslangic, kok->veri->bitis);
    *ilk = 0;
    
    jsonKaydet(kok->sag, fp, ilk);
}

void jsonDosyasiOlustur(IntervalNode *kok) {
    FILE *fp = fopen("data.json", "w");
    if (fp == NULL) return;
    fprintf(fp, "[\n");
    int ilk = 1;
    jsonKaydet(kok, fp, &ilk);
    fprintf(fp, "\n]");
    fclose(fp);
}
// --- ANA MENÜ ---

int main() {
    IntervalNode* kok = NULL;
    int secim;
    csvdenYukle(&kok);

    do {
        printf("\n=== MEDENIYET RANDEVU SISTEMI ===\n");
        printf("1. Yeni Randevu Ekle\n");
        printf("2. Sirali Randevulari Listele (Quicksort)\n");
        printf("3. Cikis\n");
        printf("Seciminiz: ");
        scanf("%d", &secim);

        if (secim == 1) {
            Randevu yeni;
            int s1, d1, s2, d2;
            printf("ID: "); scanf("%d", &yeni.id);
            printf("Isim: "); scanf(" %[^\n]s", yeni.isim);
            printf("Baslangic (SS:DD): "); scanf("%d:%d", &s1, &d1);
            printf("Bitis (SS:DD): "); scanf("%d:%d", &s2, &d2);
            
            yeni.baslangic = s1 * 60 + d1;
            yeni.bitis = s2 * 60 + d2;

            IntervalNode* yeni_kok = randevuEkle(kok, yeni, 0);
            if (yeni_kok != kok || kok == NULL) {
                kok = yeni_kok;
                csvKaydet(yeni);
                printf("Basariyla kaydedildi!\n");
            }
        } 
        else if (secim == 2) {
            Randevu liste[200];
            int adet = diziyeAktar(kok, liste, 0);
            if (adet == 0) printf("Kayitli randevu yok.\n");
            else {
                quicksort(liste, 0, adet - 1);
                printf("\n--- GUNLUK PROGRAM ---\n");
                for(int i=0; i<adet; i++) {
                    printf("[%02d:%02d - %02d:%02d] ID:%d | %s\n", 
                           liste[i].baslangic/60, liste[i].baslangic%60,
                           liste[i].bitis/60, liste[i].bitis%60,
                           liste[i].id, liste[i].isim);
                }
            }
        }
        else if (secim == 3) {
            int aranan;
            Randevu liste[200];
            int adet = diziyeAktar(kok, liste, 0);
            
            // Arama öncesi ID'ye göre sıralama şart!
            quicksort(liste, 0, adet - 1); 

            printf("Aranacak Randevu ID: ");
            scanf("%d", &aranan);

            int sonuc = binarySearchID(liste, 0, adet - 1, aranan);

            if (sonuc != -1) {
                printf("\n--- RANDEVU BULUNDU ---\n");
                printf("ID: %d | Isim: %s\n", liste[sonuc].id, liste[sonuc].isim);
                printf("Saat: %02d:%02d - %02d:%02d\n", 
                        liste[sonuc].baslangic/60, liste[sonuc].baslangic%60,
                        liste[sonuc].bitis/60, liste[sonuc].bitis%60);
            } else {
                printf("Hata: %d ID'li bir randevu kaydi bulunamadi.\n", aranan);
            }
        }
    } while (secim != 3);

    return 0;
}