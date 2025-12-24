#include "yapilar.h"

// --- 1. YARDIMCI AVL VE INTERVAL TREE FONKSİYONLARI ---
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

// --- 2. AVL ROTASYONLARI (Zorunlu Şart: Dengeli Ağaç) ---
IntervalNode* sagaDondur(IntervalNode *y) {
    IntervalNode *x = y->sol;
    IntervalNode *T2 = x->sag;
    x->sag = y; y->sol = T2;
    y->yukseklik = maksimum(yukseklikAl(y->sol), yukseklikAl(y->sag)) + 1;
    x->yukseklik = maksimum(yukseklikAl(x->sol), yukseklikAl(x->sag)) + 1;
    maxBitisGuncelle(y); maxBitisGuncelle(x);
    return x;
}

IntervalNode* solaDondur(IntervalNode *x) {
    IntervalNode *y = x->sag;
    IntervalNode *T2 = y->sol;
    y->sol = x; x->sag = T2;
    x->yukseklik = maksimum(yukseklikAl(x->sol), yukseklikAl(x->sag)) + 1;
    y->yukseklik = maksimum(yukseklikAl(y->sol), yukseklikAl(y->sag)) + 1;
    maxBitisGuncelle(x); maxBitisGuncelle(y);
    return y;
}



// --- 3. STACK (UNDO) VE QUEUE (WAITING LIST) (Zorunlu Şart: Gerçek Senaryo) ---
void push(StackNode **ust, int id) {
    StackNode *yeni = (StackNode*)malloc(sizeof(StackNode));
    yeni->silinecekID = id; yeni->sonraki = *ust; *ust = yeni;
}

void enqueue(Kuyruk *q, Randevu r) {
    KuyrukNode *yeni = (KuyrukNode*)malloc(sizeof(KuyrukNode));
    yeni->veri = r; yeni->sonraki = NULL;
    if (q->son == NULL) { q->bas = q->son = yeni; return; }
    q->son->sonraki = yeni; q->son = yeni;
}



// --- 4. DIJKSTRA ALGORİTMASI (Zorunlu Şart: Kısa Yol) ---
void salonUlasimDijkstra(int grafik[MAX_SALON][MAX_SALON], int baslangic) {
    int mesafe[MAX_SALON], ziyaret[MAX_SALON];
    for (int i = 0; i < MAX_SALON; i++) { mesafe[i] = INF; ziyaret[i] = 0; }
    mesafe[baslangic] = 0;
    for (int s = 0; s < MAX_SALON - 1; s++) {
        int min = INF, u;
        for (int v = 0; v < MAX_SALON; v++)
            if (!ziyaret[v] && mesafe[v] <= min) { min = mesafe[v]; u = v; }
        ziyaret[u] = 1;
        for (int v = 0; v < MAX_SALON; v++)
            if (!ziyaret[v] && grafik[u][v] && mesafe[u] + grafik[u][v] < mesafe[v])
                mesafe[v] = mesafe[u] + grafik[u][v];
    }
    printf("\n--- Salon 0'dan En Kisa Mesafeler (Dijkstra) ---\n");
    for (int i = 0; i < MAX_SALON; i++) printf("Salon %d: %d dk\n", i, mesafe[i]);
}



// --- 5. INTERVAL TREE VE ÇAKIŞMA KONTROLÜ ---
int sonIslemBasarili = 0;
int cakismaVarMi(Randevu r1, Randevu r2) { return (r1.baslangic < r2.bitis && r2.baslangic < r1.bitis); }

IntervalNode* dugumOlustur(Randevu r) {
    IntervalNode* yeni = (IntervalNode*)malloc(sizeof(IntervalNode));
    yeni->veri = (Randevu*)malloc(sizeof(Randevu));
    *(yeni->veri) = r; yeni->max_bitis = r.bitis;
    yeni->sol = yeni->sag = NULL; yeni->yukseklik = 1;
    sonIslemBasarili = 1; return yeni;
}

IntervalNode* randevuEkle(IntervalNode* kok, Randevu r, int sessizMod) {
    if (kok == NULL) return dugumOlustur(r);
    if (cakismaVarMi(r, *(kok->veri))) {
        if (!sessizMod) printf("\aHATA: ID %d ile cakisma! Kuyruga eklendi.\n", kok->veri->id);
        sonIslemBasarili = 0; return kok;
    }
    if (r.baslangic < kok->veri->baslangic) kok->sol = randevuEkle(kok->sol, r, sessizMod);
    else kok->sag = randevuEkle(kok->sag, r, sessizMod);
    kok->yukseklik = 1 + maksimum(yukseklikAl(kok->sol), yukseklikAl(kok->sag));
    maxBitisGuncelle(kok);
    int denge = dengeFaktoruAl(kok);
    if (denge > 1 && r.baslangic < kok->sol->veri->baslangic) return sagaDondur(kok);
    if (denge < -1 && r.baslangic > kok->sag->veri->baslangic) return solaDondur(kok);
    if (denge > 1 && r.baslangic > kok->sol->veri->baslangic) { kok->sol = solaDondur(kok->sol); return sagaDondur(kok); }
    if (denge < -1 && r.baslangic < kok->sag->veri->baslangic) { kok->sag = sagaDondur(kok->sag); return solaDondur(kok); }
    return kok;
}

// --- 6. DOSYA İŞLEMLERİ (JSON/CSV) ---
void jsonDolas(IntervalNode *kok, FILE *fp, int *ilk) {
    if (kok == NULL) return;
    jsonDolas(kok->sol, fp, ilk);
    if (!(*ilk)) fprintf(fp, ",\n");
    fprintf(fp, "  {\"id\": %d, \"isim\": \"%s\", \"baslangic\": %d, \"bitis\": %d}", 
            kok->veri->id, kok->veri->isim, kok->veri->baslangic, kok->veri->bitis);
    *ilk = 0; jsonDolas(kok->sag, fp, ilk);
}

void jsonDosyasiOlustur(IntervalNode *kok) {
    FILE *fp = fopen("data.json", "w"); if (fp == NULL) return;
    fprintf(fp, "[\n"); int ilk = 1; jsonDolas(kok, fp, &ilk); fprintf(fp, "\n]"); fclose(fp);
}

void csvKaydet(Randevu r) {
    FILE *fp = fopen("randevular.csv", "a");
    if (fp != NULL) { fprintf(fp, "%d,%s,%d,%d\n", r.id, r.isim, r.baslangic, r.bitis); fclose(fp); }
}

void csvdenYukle(IntervalNode** kok) {
    FILE *fp = fopen("randevular.csv", "r"); if (fp == NULL) return;
    Randevu r; while (fscanf(fp, "%d,%[^,],%d,%d\n", &r.id, r.isim, &r.baslangic, &r.bitis) != EOF) { *kok = randevuEkle(*kok, r, 1); }
    fclose(fp);
}

// --- 7. SIRALAMA VE ARAMA (Zorunlu Şart: Quicksort & Binary Search) ---
void quicksort(Randevu dizi[], int d, int y) {
    if (d < y) {
        int pivot = dizi[y].id, i = d - 1;
        for (int j = d; j <= y - 1; j++) {
            if (dizi[j].id < pivot) { i++; Randevu t = dizi[i]; dizi[i] = dizi[j]; dizi[j] = t; }
        }
        Randevu t = dizi[i+1]; dizi[i+1] = dizi[y]; dizi[y] = t;
        quicksort(dizi, d, i); quicksort(dizi, i + 2, y);
    }
}

int binarySearchID(Randevu dizi[], int d, int y, int id) {
    while (d <= y) {
        int o = d + (y - d) / 2;
        if (dizi[o].id == id) return o;
        if (dizi[o].id < id) d = o + 1; else y = o - 1;
    }
    return -1;
}

int diziyeAktar(IntervalNode *kok, Randevu dizi[], int index) {
    if (kok == NULL) return index;
    index = diziyeAktar(kok->sol, dizi, index);
    dizi[index++] = *(kok->veri);
    return diziyeAktar(kok->sag, dizi, index);
}

// --- 8. ANA MENÜ ---
int main() {
    IntervalNode* kok = NULL; StackNode* undoStack = NULL; Kuyruk bekleme = {NULL, NULL};
    int secim, grafik[MAX_SALON][MAX_SALON] = {{0,10,0,30,0},{10,0,50,0,0},{0,50,0,20,10},{30,0,20,0,60},{0,0,10,60,0}};
    csvdenYukle(&kok); jsonDosyasiOlustur(kok);

    do {
        printf("\n=== IMU SALON MOTORU ===\n1. Randevu Ekle\n2. Bekleme Listesi (Queue)\n3. Salon Mesafeleri (Dijkstra)\n4. Sirali Liste & Arama\n5. Cikis\nSecim: ");
        scanf("%d", &secim);

        if (secim == 1) {
            Randevu y; int s1,d1,s2,d2;
            printf("ID: "); scanf("%d", &y.id); printf("Isim: "); scanf(" %[^\n]s", y.isim);
            printf("Bas (SS:DD): "); scanf("%d:%d", &s1, &d1); printf("Bit (SS:DD): "); scanf("%d:%d", &s2, &d2);
            y.baslangic = s1*60+d1; y.bitis = s2*60+d2;
            kok = randevuEkle(kok, y, 0);
            if (sonIslemBasarili) { csvKaydet(y); push(&undoStack, y.id); jsonDosyasiOlustur(kok); printf("Basarili!\n"); }
            else enqueue(&bekleme, y);
        } 
        else if (secim == 2) {
            KuyrukNode *t = bekleme.bas; printf("\n--- Bekleme Listesi ---\n");
            while(t) { printf("ID:%d | %s\n", t->veri.id, t->veri.isim); t = t->sonraki; }
        } 
        else if (secim == 3) salonUlasimDijkstra(grafik, 0);
        else if (secim == 4) {
            Randevu liste[100]; int adet = diziyeAktar(kok, liste, 0);
            if (adet > 0) {
                quicksort(liste, 0, adet - 1);
                for(int i=0; i<adet; i++) printf("ID:%d | %s | %02d:%02d\n", liste[i].id, liste[i].isim, liste[i].baslangic/60, liste[i].baslangic%60);
                int ara; printf("Aranacak ID: "); scanf("%d", &ara);
                int snc = binarySearchID(liste, 0, adet - 1, ara);
                if(snc != -1) printf("Bulundu: %s\n", liste[snc].isim); else printf("Yok!\n");
            }
        }
    } while (secim != 5);
    return 0;
}