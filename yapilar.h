#ifndef YAPILAR_H
#define YAPILAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Her bir randevuyu temsil eden yapı
typedef struct Randevu {
    int id;
    char isim[50];
    int baslangic; // Dakika cinsinden (Örn: 09:00 -> 540)
    int bitis;     // Dakika cinsinden
} Randevu;

// Interval Tree (Aralık Ağacı) düğümü
typedef struct IntervalNode {
    Randevu *veri;
    int max_bitis; // Alt ağaçtaki en büyük bitiş zamanı (Çakışma kontrolü için kritik)
    struct IntervalNode *sol, *sag;
    int yukseklik; // AVL (Dengeli Ağaç) özelliği için
} IntervalNode;

// Bekleme listesi için Kuyruk (Queue) yapısı
typedef struct KuyrukNode {
    Randevu *veri;
    struct KuyrukNode *sonraki;
} KuyrukNode;
#endif