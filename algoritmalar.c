#include "yapilar.h"

// Küresel Bayrak: İşlem durumunu main.c ile paylaşmak için
extern int sonIslemBasarili;

// --- 1. YARDIMCI AVL FONKSİYONLARI ---
int maksimum(int a, int b) { return (a > b) ? a : b; }

int yukseklikAl(IntervalNode *n) { return (n == NULL) ? 0 : n->yukseklik; }

// Interval Tree gereksinimi: Düğümün altındaki en büyük bitiş zamanını günceller
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

// --- 2. AVL ROTASYONLARI ---
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

// --- 3. INTERVAL TREE & ÇAKIŞMA MANTIĞI ---
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
    sonIslemBasarili = 1; 
    return yeni;
}

IntervalNode* randevuEkle(IntervalNode* kok, Randevu r, int sessiz) {
    if (kok == NULL) return dugumOlustur(r);

    // Çakışma Kontrolü
    if (cakismaVarMi(r, *(kok->veri))) {
        if (!sessiz) printf("\aHATA: ID %d ile cakisma! Kuyruga alindi.\n", kok->veri->id);
        sonIslemBasarili = 0; 
        return kok;
    }

    if (r.baslangic < kok->veri->baslangic) 
        kok->sol = randevuEkle(kok->sol, r, sessiz);
    else 
        kok->sag = randevuEkle(kok->sag, r, sessiz);

    // Yükseklik ve Max Bitiş Güncelleme
    kok->yukseklik = 1 + maksimum(yukseklikAl(kok->sol), yukseklikAl(kok->sag));
    maxBitisGuncelle(kok);

    // AVL Dengeleme
    int denge = dengeFaktoruAl(kok);
    if (denge > 1 && r.baslangic < kok->sol->veri->baslangic) return sagaDondur(kok);
    if (denge < -1 && r.baslangic > kok->sag->veri->baslangic) return solaDondur(kok);
    if (denge > 1 && r.baslangic > kok->sol->veri->baslangic) { 
        kok->sol = solaDondur(kok->sol); return sagaDondur(kok); 
    }
    if (denge < -1 && r.baslangic < kok->sag->veri->baslangic) { 
        kok->sag = sagaDondur(kok->sag); return solaDondur(kok); 
    }
    return kok;
}

// --- 4. AVL SİLME (UNDO İŞLEMİ İÇİN) ---
IntervalNode* enKucukDugum(IntervalNode* n) {
    IntervalNode* akim = n;
    while (akim->sol != NULL) akim = akim->sol;
    return akim;
}

IntervalNode* avlSil(IntervalNode* kok, int id) {
    if (kok == NULL) return kok;
    if (id < kok->veri->id) kok->sol = avlSil(kok->sol, id);
    else if (id > kok->veri->id) kok->sag = avlSil(kok->sag, id);
    else {
        if ((kok->sol == NULL) || (kok->sag == NULL)) {
            IntervalNode *temp = kok->sol ? kok->sol : kok->sag;
            if (temp == NULL) { temp = kok; kok = NULL; }
            else *kok = *temp;
            free(temp);
        } else {
            IntervalNode* temp = enKucukDugum(kok->sag);
            *(kok->veri) = *(temp->veri);
            kok->sag = avlSil(kok->sag, temp->veri->id);
        }
    }
    if (kok == NULL) return kok;
    kok->yukseklik = 1 + maksimum(yukseklikAl(kok->sol), yukseklikAl(kok->sag));
    maxBitisGuncelle(kok);
    int denge = dengeFaktoruAl(kok);
    if (denge > 1 && dengeFaktoruAl(kok->sol) >= 0) return sagaDondur(kok);
    if (denge > 1 && dengeFaktoruAl(kok->sol) < 0) { 
        kok->sol = solaDondur(kok->sol); return sagaDondur(kok); 
    }
    if (denge < -1 && dengeFaktoruAl(kok->sag) <= 0) return solaDondur(kok);
    if (denge < -1 && dengeFaktoruAl(kok->sag) > 0) { 
        kok->sag = sagaDondur(kok->sag); return solaDondur(kok); 
    }
    return kok;
}

// --- 5. STACK VE QUEUE OPERASYONLARI ---
void push(StackNode **ust, int id) {
    StackNode *yeni = (StackNode*)malloc(sizeof(StackNode));
    yeni->id = id; yeni->sonraki = *ust; *ust = yeni;
}

int pop(StackNode **ust) {
    if (*ust == NULL) return -1;
    StackNode *temp = *ust; int id = temp->id;
    *ust = (*ust)->sonraki; free(temp); return id;
}

void enqueue(Kuyruk *q, Randevu r) {
    KuyrukNode *yeni = (KuyrukNode*)malloc(sizeof(KuyrukNode));
    yeni->veri = r; yeni->sonraki = NULL;
    if (q->son == NULL) { q->bas = q->son = yeni; return; }
    q->son->sonraki = yeni; q->son = yeni;
}

Randevu dequeue(Kuyruk *q) {
    KuyrukNode *temp = q->bas; Randevu r = temp->veri;
    q->bas = q->bas->sonraki; if (q->bas == NULL) q->son = NULL;
    free(temp); return r;
}

// --- 6. DIJKSTRA (KISA YOL ANALİZİ) ---
void enKisaYolBul(int grafik[MAX_SALON][MAX_SALON], int baslangic) {
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
    printf("\n--- Salonlar Arasi Mesafe Analizi (Dijkstra) ---\n");
    for (int i = 0; i < MAX_SALON; i++) printf("Salon %d -> %d: %d dk\n", baslangic, i, mesafe[i]);
}

// --- 7. SIRALAMA VE ARAMA ---
void quicksort(Randevu dizi[], int d, int y) {
    if (d < y) {
        int pivot = dizi[y].id, i = d - 1;
        for (int j = d; j <= y - 1; j++) {
            if (dizi[j].id < pivot) { 
                i++; Randevu t = dizi[i]; dizi[i] = dizi[j]; dizi[j] = t; 
            }
        }
        Randevu t = dizi[i+1]; dizi[i+1] = dizi[y]; dizi[y] = t;
        quicksort(dizi, d, i); quicksort(dizi, i + 2, y);
    }
}

int ikiliArama(Randevu dizi[], int d, int y, int id) {
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

// --- 8. DOSYA VE JSON İŞLEMLERİ ---
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
    Randevu r; 
    while (fscanf(fp, "%d,%[^,],%d,%d\n", &r.id, r.isim, &r.baslangic, &r.bitis) != EOF) { 
        *kok = randevuEkle(*kok, r, 1); 
    }
    fclose(fp);
}

// --- 9. BFS (AGAC YAPISI GOSTERIMI) ---
void seviyeGosterBFS(IntervalNode* kok) {
    if (kok == NULL) {
        printf("Agac su an bos.\n");
        return;
    }
    printf("\n--- Agac Hiyerarsisi (BFS Katmanlari) ---\n");

    IntervalNode* kuyruk[100]; // Geçici node kuyruğu
    int bas = 0, son = 0;

    kuyruk[son++] = kok;

    while (bas < son) {
        IntervalNode* akim = kuyruk[bas++];
        
        printf("[ID: %d | Isim: %s]", akim->veri->id, akim->veri->isim);
        
        if (akim->sol != NULL) {
            printf(" -> Sol: %d", akim->sol->veri->id);
            kuyruk[son++] = akim->sol;
        } else printf(" -> Sol: -");

        if (akim->sag != NULL) {
            printf(" | Sag: %d", akim->sag->veri->id);
            kuyruk[son++] = akim->sag;
        } else printf(" | Sag: -");
        
        printf("\n");
    }
}