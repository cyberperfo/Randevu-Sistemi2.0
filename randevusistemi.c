#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- BÖLÜM 1: YAPILAR VE TANIMLAR ---
#define MAX_SALON 5
#define INF 9999

typedef struct
{
    int id;
    char isim[50];
    char tarih[11]; // "YYYY-MM-DD" formatında
    int baslangic;
    int bitis;
} Randevu;

typedef struct IntervalNode
{
    Randevu *veri;
    int max_bitis;
    struct IntervalNode *sol, *sag;
    int yukseklik;
} IntervalNode;

typedef struct StackNode
{
    int id;
    struct StackNode *sonraki;
} StackNode;

typedef struct KuyrukNode
{
    Randevu veri;
    struct KuyrukNode *sonraki;
} KuyrukNode;

typedef struct
{
    KuyrukNode *bas;
    KuyrukNode *son;
} Kuyruk;

// Küresel Bayrak
int sonIslemBasarili = 0;

// --- BÖLÜM 2: YARDIMCI FONKSİYONLAR ---

int maksimum(int a, int b) { return (a > b) ? a : b; }
int yukseklikAl(IntervalNode *n) { return (n == NULL) ? 0 : n->yukseklik; }

void maxBitisGuncelle(IntervalNode *n)
{
    if (n == NULL)
        return;
    int m = n->veri->bitis;
    if (n->sol != NULL && n->sol->max_bitis > m)
        m = n->sol->max_bitis;
    if (n->sag != NULL && n->sag->max_bitis > m)
        m = n->sag->max_bitis;
    n->max_bitis = m;
}

int dengeFaktoruAl(IntervalNode *n)
{
    return (n == NULL) ? 0 : yukseklikAl(n->sol) - yukseklikAl(n->sag);
}

IntervalNode *sagaDondur(IntervalNode *y)
{
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

IntervalNode *solaDondur(IntervalNode *x)
{
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

int cakismaVarMi(Randevu r1, Randevu r2)
{
    // Farklı tarihlerdeki randevular çakışmaz
    if (strcmp(r1.tarih, r2.tarih) != 0)
        return 0;
    // Aynı tarihteyse saat kontrolü yap
    return (r1.baslangic < r2.bitis && r2.baslangic < r1.bitis);
}

// ID kontrolü fonksiyon prototipi
int idKontrol(IntervalNode *kok, int id);

IntervalNode *dugumOlustur(Randevu r)
{
    IntervalNode *yeni = (IntervalNode *)malloc(sizeof(IntervalNode));
    yeni->veri = (Randevu *)malloc(sizeof(Randevu));
    *(yeni->veri) = r;
    yeni->max_bitis = r.bitis;
    yeni->sol = yeni->sag = NULL;
    yeni->yukseklik = 1;
    sonIslemBasarili = 1;
    return yeni;
}

IntervalNode *randevuEkle(IntervalNode *kok, Randevu r, int sessiz)
{
    // Ters zaman kontrolü
    if (r.baslangic >= r.bitis)
    {
        if (!sessiz)
        {
            printf("\aHATA: Baslangic saati bitis saatinden sonra veya esit olamaz!\n");
            printf("Baslangic: %d dakika, Bitis: %d dakika\n", r.baslangic, r.bitis);
        }
        sonIslemBasarili = 0;
        return kok;
    }

    // Minimum süre kontrolü (15 dakika)
    if ((r.bitis - r.baslangic) < 15)
    {
        if (!sessiz)
        {
            printf("\aHATA: Randevu en az 15 dakika olmalı!\n");
            printf("Süre: %d dakika\n", (r.bitis - r.baslangic));
        }
        sonIslemBasarili = 0;
        return kok;
    }

    if (kok == NULL)
        return dugumOlustur(r);

    // Aynı ID kontrolü (tüm ağacı kontrol et)
    int idVarMi = idKontrol(kok, r.id);
    if (idVarMi)
    {
        if (!sessiz)
        {
            printf("\aHATA: ID %d zaten sistemde mevcut!\n", r.id);
        }
        sonIslemBasarili = 0;
        return kok;
    }

    // Aynı tarih kontrolü
    if (strcmp(r.tarih, kok->veri->tarih) == 0 && cakismaVarMi(r, *(kok->veri)))
    {
        if (!sessiz)
        {
            printf("\aHATA: ID %d ile saat cakismasi var! Kuyruga alindi.\n", kok->veri->id);
            printf("Mevcut: %02d:%02d-%02d:%02d, Yeni: %02d:%02d-%02d:%02d\n",
                   kok->veri->baslangic / 60, kok->veri->baslangic % 60,
                   kok->veri->bitis / 60, kok->veri->bitis % 60,
                   r.baslangic / 60, r.baslangic % 60,
                   r.bitis / 60, r.bitis % 60);
        }
        sonIslemBasarili = 0;
        return kok;
    }

    if (r.baslangic < kok->veri->baslangic)
    {
        kok->sol = randevuEkle(kok->sol, r, sessiz);
    }
    else
    {
        kok->sag = randevuEkle(kok->sag, r, sessiz);
    }

    kok->yukseklik = 1 + maksimum(yukseklikAl(kok->sol), yukseklikAl(kok->sag));
    maxBitisGuncelle(kok);
    int denge = dengeFaktoruAl(kok);

    // AVL dengesini sağla
    if (denge > 1 && r.baslangic < kok->sol->veri->baslangic)
        return sagaDondur(kok);
    if (denge < -1 && r.baslangic > kok->sag->veri->baslangic)
        return solaDondur(kok);
    if (denge > 1 && r.baslangic > kok->sol->veri->baslangic)
    {
        kok->sol = solaDondur(kok->sol);
        return sagaDondur(kok);
    }
    if (denge < -1 && r.baslangic < kok->sag->veri->baslangic)
    {
        kok->sag = sagaDondur(kok->sag);
        return solaDondur(kok);
    }
    return kok;
}

// ID kontrolü fonksiyonu
int idKontrol(IntervalNode *kok, int id)
{
    if (kok == NULL)
        return 0;
    if (kok->veri->id == id)
        return 1;
    if (id < kok->veri->id)
        return idKontrol(kok->sol, id);
    else
        return idKontrol(kok->sag, id);
}

IntervalNode *avlSil(IntervalNode *kok, int id)
{
    if (kok == NULL)
        return kok;
    if (id < kok->veri->id)
        kok->sol = avlSil(kok->sol, id);
    else if (id > kok->veri->id)
        kok->sag = avlSil(kok->sag, id);
    else
    {
        if ((kok->sol == NULL) || (kok->sag == NULL))
        {
            IntervalNode *temp = kok->sol ? kok->sol : kok->sag;
            if (temp == NULL)
            {
                temp = kok;
                kok = NULL;
            }
            else
                *kok = *temp;
            free(temp);
        }
        else
        {
            IntervalNode *akim = kok->sag;
            while (akim->sol != NULL)
                akim = akim->sol;
            *(kok->veri) = *(akim->veri);
            kok->sag = avlSil(kok->sag, akim->veri->id);
        }
    }
    if (kok == NULL)
        return kok;
    kok->yukseklik = 1 + maksimum(yukseklikAl(kok->sol), yukseklikAl(kok->sag));
    maxBitisGuncelle(kok);
    return kok;
}

void push(StackNode **ust, int id)
{
    StackNode *yeni = (StackNode *)malloc(sizeof(StackNode));
    yeni->id = id;
    yeni->sonraki = *ust;
    *ust = yeni;
}

int pop(StackNode **ust)
{
    if (*ust == NULL)
        return -1;
    StackNode *temp = *ust;
    int id = temp->id;
    *ust = (*ust)->sonraki;
    free(temp);
    return id;
}

void enqueue(Kuyruk *q, Randevu r)
{
    KuyrukNode *yeni = (KuyrukNode *)malloc(sizeof(KuyrukNode));
    yeni->veri = r;
    yeni->sonraki = NULL;
    if (q->son == NULL)
    {
        q->bas = q->son = yeni;
        return;
    }
    q->son->sonraki = yeni;
    q->son = yeni;
}

Randevu dequeue(Kuyruk *q)
{
    KuyrukNode *temp = q->bas;
    Randevu r = temp->veri;
    q->bas = q->bas->sonraki;
    if (q->bas == NULL)
        q->son = NULL;
    free(temp);
    return r;
}

void enKisaYolBul(int grafik[MAX_SALON][MAX_SALON], int baslangic)
{
    int mesafe[MAX_SALON], ziyaret[MAX_SALON];
    for (int i = 0; i < MAX_SALON; i++)
    {
        mesafe[i] = INF;
        ziyaret[i] = 0;
    }
    mesafe[baslangic] = 0;
    for (int s = 0; s < MAX_SALON - 1; s++)
    {
        int min = INF, u;
        for (int v = 0; v < MAX_SALON; v++)
            if (!ziyaret[v] && mesafe[v] <= min)
            {
                min = mesafe[v];
                u = v;
            }
        ziyaret[u] = 1;
        for (int v = 0; v < MAX_SALON; v++)
            if (!ziyaret[v] && grafik[u][v] && mesafe[u] + grafik[u][v] < mesafe[v])
                mesafe[v] = mesafe[u] + grafik[u][v];
    }
    printf("\n--- Salonlar Arasi Mesafe Analizi (Dijkstra) ---\n");
    for (int i = 0; i < MAX_SALON; i++)
        printf("Salon %d -> %d: %d dk\n", baslangic, i, mesafe[i]);
}

void quicksort(Randevu dizi[], int d, int y)
{
    if (d < y)
    {
        int pivot = dizi[y].id, i = d - 1;
        for (int j = d; j <= y - 1; j++)
        {
            if (dizi[j].id < pivot)
            {
                i++;
                Randevu t = dizi[i];
                dizi[i] = dizi[j];
                dizi[j] = t;
            }
        }
        Randevu t = dizi[i + 1];
        dizi[i + 1] = dizi[y];
        dizi[y] = t;
        quicksort(dizi, d, i);
        quicksort(dizi, i + 2, y);
    }
}

int ikiliArama(Randevu dizi[], int d, int y, int id)
{
    while (d <= y)
    {
        int o = d + (y - d) / 2;
        if (dizi[o].id == id)
            return o;
        if (dizi[o].id < id)
            d = o + 1;
        else
            y = o - 1;
    }
    return -1;
}

int diziyeAktar(IntervalNode *kok, Randevu dizi[], int index)
{
    if (kok == NULL)
        return index;
    index = diziyeAktar(kok->sol, dizi, index);
    dizi[index++] = *(kok->veri);
    return diziyeAktar(kok->sag, dizi, index);
}

void jsonDolas(IntervalNode *kok, FILE *fp, int *ilk)
{
    if (kok == NULL)
        return;
    jsonDolas(kok->sol, fp, ilk);
    if (!(*ilk))
        fprintf(fp, ",\n");
    fprintf(fp, "  {\"id\": %d, \"isim\": \"%s\", \"tarih\": \"%s\", \"baslangic\": %d, \"bitis\": %d}",
            kok->veri->id, kok->veri->isim, kok->veri->tarih, kok->veri->baslangic, kok->veri->bitis);
    *ilk = 0;
    jsonDolas(kok->sag, fp, ilk);
}

void jsonDosyasiOlustur(IntervalNode *kok)
{
    FILE *fp = fopen("data.json", "w");
    if (fp == NULL)
        return;
    fprintf(fp, "[\n");
    int ilk = 1;
    jsonDolas(kok, fp, &ilk);
    fprintf(fp, "\n]");
    fclose(fp);
}

void csvKaydet(Randevu r)
{
    FILE *fp = fopen("randevular.csv", "a");
    if (fp != NULL)
    {
        fprintf(fp, "%d,%s,%s,%d,%d\n", r.id, r.isim, r.tarih, r.baslangic, r.bitis);
        fclose(fp);
    }
}

// CSV dosyasını otomatik olarak güncelle (eski format -> yeni format)
void csvGuncelle()
{
    FILE *eski = fopen("randevular.csv", "r");
    if (eski == NULL)
        return;

    // İlk satırı oku ve formatı kontrol et
    char ilkSatir[256];
    if (fgets(ilkSatir, sizeof(ilkSatir), eski))
    {
        // Satırdaki virgül sayısını say
        int virgulSayisi = 0;
        for (int i = 0; ilkSatir[i]; i++)
        {
            if (ilkSatir[i] == ',')
                virgulSayisi++;
        }

        // Eski formatta 3 virgül var (id,isim,baslangic,bitis)
        // Yeni formatta 4 virgül olmalı (id,isim,tarih,baslangic,bitis)
        if (virgulSayisi == 3)
        {
            printf("CSV dosyasi eski formatta. Guncelleniyor...\n");
            fclose(eski);

            // Dosyayı yeniden aç ve tüm satırları oku
            eski = fopen("randevular.csv", "r");
            FILE *yeni = fopen("randevular_new.csv", "w");

            if (eski && yeni)
            {
                // Bugünün tarihini al
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                char tarih[11];
                sprintf(tarih, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

                char line[256];
                while (fgets(line, sizeof(line), eski))
                {
                    int id, baslangic, bitis;
                    char isim[50];

                    if (sscanf(line, "%d,%[^,],%d,%d", &id, isim, &baslangic, &bitis) == 4)
                    {
                        // Eski verileri bugünün tarihine ata
                        fprintf(yeni, "%d,%s,%s,%d,%d\n", id, isim, tarih, baslangic, bitis);
                    }
                }

                fclose(eski);
                fclose(yeni);

                // Yeni dosyayı eski dosyanın üzerine kopyala
                remove("randevular.csv");
                rename("randevular_new.csv", "randevular.csv");

                printf("CSV dosyasi guncellendi! Tum randevular bugunun tarihine atandi: %s\n", tarih);
            }
            return;
        }
        fclose(eski);
    }
}

void csvdenYukle(IntervalNode **kok)
{
    // Önce CSV dosyasını güncelle
    csvGuncelle();

    FILE *fp = fopen("randevular.csv", "r");
    if (fp == NULL)
        return;

    Randevu r;
    // Yeni format: id,isim,tarih,baslangic,bitis
    while (fscanf(fp, "%d,%[^,],%10[^,],%d,%d\n", &r.id, r.isim, r.tarih, &r.baslangic, &r.bitis) != EOF)
    {
        *kok = randevuEkle(*kok, r, 1);
    }
    fclose(fp);
}

void seviyeGosterBFS(IntervalNode *kok)
{
    if (kok == NULL)
    {
        printf("Agac su an bos.\n");
        return;
    }
    printf("\n--- Agac Hiyerarsisi (BFS Katmanlari) ---\n");
    IntervalNode *kuyruk[100];
    int bas = 0, son = 0;
    kuyruk[son++] = kok;
    while (bas < son)
    {
        IntervalNode *akim = kuyruk[bas++];
        printf("[ID: %d | Isim: %s | Tarih: %s]", akim->veri->id, akim->veri->isim, akim->veri->tarih);
        if (akim->sol != NULL)
        {
            printf(" -> Sol: %d", akim->sol->veri->id);
            kuyruk[son++] = akim->sol;
        }
        if (akim->sag != NULL)
        {
            printf(" | Sag: %d", akim->sag->veri->id);
            kuyruk[son++] = akim->sag;
        }
        printf("\n");
    }
}

void bufferTemizle()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

// --- BÖLÜM 3: ANA DÖNGÜ ---
int main()
{
    IntervalNode *kok = NULL;
    StackNode *undoStack = NULL;
    Kuyruk bekleme = {NULL, NULL};
    int secim;
    int grafik[MAX_SALON][MAX_SALON] = {
        {0, 10, 0, 30, 0}, {10, 0, 50, 0, 0}, {0, 50, 0, 20, 10}, {30, 0, 20, 0, 60}, {0, 0, 10, 60, 0}};

    // CSV'yi otomatik güncelle ve yükle
    csvdenYukle(&kok);
    jsonDosyasiOlustur(kok);

    do
    {
        printf("\n=== IMU SALON YONETIM MOTORU v5.0 ===\n");
        printf("1. Yeni Randevu Ekle\n2. Geri Al (Undo)\n3. Bekleme Listesi\n4. Bekleyeni Isle\n5. Agac Yapisi (BFS)\n6. Mesafe Analizi (Dijkstra)\n7. ID Sirala (Quicksort)\n8. ID Ara (Binary Search)\n9. JSON Guncelle\n10. Cikis\nSecim: ");
        if (scanf("%d", &secim) != 1)
        {
            bufferTemizle();
            continue;
        }

        switch (secim)
        {
        case 1:
        {
            Randevu y;
            int s1, d1, s2, d2;
            char tarih[11];
            printf("ID: ");
            scanf("%d", &y.id);
            printf("Isim: ");
            scanf(" %[^\n]s", y.isim);

            // Bugünün tarihini otomatik göster
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char bugun[11];
            sprintf(bugun, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

            printf("Tarih (YYYY-AA-GG) [%s]: ", bugun);
            scanf("%10s", tarih);

            // Kullanıcı enter'a basarsa bugünün tarihini kullan
            if (strlen(tarih) == 0)
            {
                strcpy(y.tarih, bugun);
            }
            else
            {
                strcpy(y.tarih, tarih);
            }

            printf("Saat (SS:DD SS:DD): ");
            scanf("%d:%d %d:%d", &s1, &d1, &s2, &d2);
            y.baslangic = s1 * 60 + d1;
            y.bitis = s2 * 60 + d2;
            sonIslemBasarili = 0;
            kok = randevuEkle(kok, y, 0);
            if (sonIslemBasarili)
            {
                csvKaydet(y);
                push(&undoStack, y.id);
                jsonDosyasiOlustur(kok);
                printf("✓ Randevu basariyla eklendi!\n");
            }
            else
            {
                enqueue(&bekleme, y);
                printf("✗ Randevu eklenemedi! Bekleme listesine alindi.\n");
            }
            break;
        }
        case 2:
        {
            int id = pop(&undoStack);
            if (id != -1)
            {
                kok = avlSil(kok, id);
                jsonDosyasiOlustur(kok);
                printf("✓ ID %d silindi.\n", id);
            }
            else
            {
                printf("ℹ Geri alinacak islem yok.\n");
            }
            break;
        }
        case 3:
        {
            KuyrukNode *t = bekleme.bas;
            printf("\n--- Bekleme Listesi ---\n");
            if (!t)
                printf("ℹ Liste bos.\n");
            while (t)
            {
                printf("ID:%d | %s | Tarih: %s\n", t->veri.id, t->veri.isim, t->veri.tarih);
                t = t->sonraki;
            }
            break;
        }
        case 4:
        {
            if (bekleme.bas)
            {
                Randevu r = dequeue(&bekleme);
                printf("%s yeniden deneniyor...\n", r.isim);
                sonIslemBasarili = 0;
                kok = randevuEkle(kok, r, 0);
                if (sonIslemBasarili)
                {
                    push(&undoStack, r.id);
                    jsonDosyasiOlustur(kok);
                    printf("✓ Randevu eklendi!\n");
                }
                else
                {
                    enqueue(&bekleme, r);
                    printf("✗ Hala cakisma var! Geri bekleme listesine alindi.\n");
                }
            }
            else
                printf("ℹ Bekleyen randevu yok.\n");
            break;
        }
        case 5:
            seviyeGosterBFS(kok);
            break;
        case 6:
            enKisaYolBul(grafik, 0);
            break;
        case 7:
        {
            Randevu liste[100];
            int adet = diziyeAktar(kok, liste, 0);
            if (adet > 0)
            {
                quicksort(liste, 0, adet - 1);
                for (int i = 0; i < adet; i++)
                    printf("ID:%d | %s | Tarih: %s\n", liste[i].id, liste[i].isim, liste[i].tarih);
            }
            else
                printf("ℹ Sistemde veri yok.\n");
            break;
        }
        case 8:
        {
            Randevu liste[100];
            int adet = diziyeAktar(kok, liste, 0);
            if (adet > 0)
            {
                quicksort(liste, 0, adet - 1);
                int ara;
                printf("Aranacak ID: ");
                scanf("%d", &ara);
                int snc = ikiliArama(liste, 0, adet - 1, ara);
                if (snc != -1)
                    printf("✓ Bulundu: %s | Tarih: %s\n", liste[snc].isim, liste[snc].tarih);
                else
                    printf("✗ ID %d bulunamadi.\n", ara);
            }
            break;
        }
        case 9:
            jsonDosyasiOlustur(kok);
            printf("✓ JSON senkronize edildi.\n");
            break;
        case 10:
            printf("Sistem kapatiliyor...\n");
            break;
        }
    } while (secim != 10);
    return 0;
}