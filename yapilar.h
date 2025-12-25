#ifndef YAPILAR_H
#define YAPILAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SALON 5
#define INF 99999

typedef struct Randevu {
    int id;
    char isim[50];
    int baslangic; 
    int bitis;     
} Randevu;

typedef struct IntervalNode {
    Randevu *veri;
    int max_bitis; 
    struct IntervalNode *sol, *sag;
    int yukseklik; 
} IntervalNode;

typedef struct StackNode {
    int id;
    struct StackNode *sonraki;
} StackNode;

typedef struct KuyrukNode {
    Randevu veri;
    struct KuyrukNode *sonraki;
} KuyrukNode;

typedef struct {
    KuyrukNode *bas, *son;
} Kuyruk;

// --- algoritmalar.c içerisindeki tüm fonksiyonların prototipleri ---
IntervalNode* randevuEkle(IntervalNode* kok, Randevu r, int sessiz);
IntervalNode* avlSil(IntervalNode* kok, int id);
void enKisaYolBul(int grafik[MAX_SALON][MAX_SALON], int baslangic);
void quicksort(Randevu dizi[], int d, int y);
int ikiliArama(Randevu dizi[], int d, int y, int id);
void seviyeGosterBFS(IntervalNode* kok);
int diziyeAktar(IntervalNode *kok, Randevu dizi[], int index);
void jsonDosyasiOlustur(IntervalNode *kok);
void csvdenYukle(IntervalNode** kok);
void csvKaydet(Randevu r);

// Stack ve Queue Prototipleri
void push(StackNode **ust, int id);
int pop(StackNode **ust);
void enqueue(Kuyruk *q, Randevu r);
Randevu dequeue(Kuyruk *q);

#endif