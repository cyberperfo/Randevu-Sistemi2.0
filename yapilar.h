#ifndef YAPILAR_H
#define YAPILAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SALON 5
#define INF 99999

// Her bir randevuyu temsil eden yapı
typedef struct Randevu {
    int id;
    char isim[50];
    int baslangic; 
    int bitis;     
} Randevu;

// AVL + Interval Tree düğümü [Zorunlu Şart 1]
typedef struct IntervalNode {
    Randevu *veri;
    int max_bitis; 
    struct IntervalNode *sol, *sag;
    int yukseklik; 
} IntervalNode;

// Geri Al (Undo) işlemi için Yığın (Stack) yapısı [Zorunlu Şart 4]
typedef struct StackNode {
    int silinecekID;
    struct StackNode *sonraki;
} StackNode;

// Bekleme listesi için Kuyruk (Queue) yapısı [Zorunlu Şart 4]
typedef struct KuyrukNode {
    Randevu veri;
    struct KuyrukNode *sonraki;
} KuyrukNode;

typedef struct {
    KuyrukNode *bas, *son;
} Kuyruk;

#endif