#include "yapilar.h"

// Küresel Bayrak: Kayıt işlemlerini tetiklemek için
int sonIslemBasarili = 0;

// Hatalı girişte sonsuz döngüyü önleyen tampon temizleyici
void bufferTemizle() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// --- ANA DÖNGÜ VE MENÜ YÖNETİMİ ---
int main() {
    IntervalNode* kok = NULL; 
    StackNode* undoStack = NULL; 
    Kuyruk bekleme = {NULL, NULL};
    int secim;
    
    // Salonlar arası mesafe matrisi (Dijkstra için)
    int grafik[MAX_SALON][MAX_SALON] = {
        {0, 10, 0, 30, 0}, {10, 0, 50, 0, 0}, {0, 50, 0, 20, 10}, {30, 0, 20, 0, 60}, {0, 0, 10, 60, 0}
    };

    // Açılışta kalıcı verileri yükle
    csvdenYukle(&kok); 
    jsonDosyasiOlustur(kok);

    do {
        printf("\n=== IMU SALON YONETIM MOTORU v5.0 ===\n");
        printf("1. Yeni Randevu Ekle (AVL & Stack Push)\n");
        printf("2. Geri Al (Undo - Stack Pop & AVL Delete)\n");
        printf("3. Bekleme Listesini Gor (Queue Gosterim)\n");
        printf("4. Bekleyeni Isle (Queue Dequeue & Re-Insert)\n");
        printf("5. Agac Yapisi (BFS - Seviye Listeleme)\n");
        printf("6. Salonlar Arasi Mesafe (Dijkstra)\n");
        printf("7. Randevulari ID'ye Gore Sirala (Quicksort)\n");
        printf("8. ID ile Randevu Ara (Binary Search)\n");
        printf("9. JSON Verisini Guncelle (Manuel Sync)\n");
        printf("10. Cikis\n");
        printf("Seciminiz: ");

        // Seçim Format Kontrolü
        if (scanf("%d", &secim) != 1) {
            printf("\aHata: Lutfen sadece rakam girerek tekrar deneyiniz.\n");
            bufferTemizle();
            continue;
        }

        switch(secim) {
            case 1: { // ATOMİK EKLEME
                Randevu y; int s1, d1, s2, d2;
                printf("ID (Sayisal): ");
                while(scanf("%d", &y.id) != 1) {
                    printf("Gecersiz ID! Tekrar deneyiniz: ");
                    bufferTemizle();
                }
                printf("Isim: "); scanf(" %[^\n]s", y.isim);

                while (1) {
                    printf("Saat Formati (SS:DD SS:DD): ");
                    if (scanf("%d:%d %d:%d", &s1, &d1, &s2, &d2) == 4) {
                        if (s1>=0 && s1<24 && d1>=0 && d1<60 && s2>=0 && s2<24 && d2>=0 && d2<60) {
                            y.baslangic = s1 * 60 + d1; y.bitis = s2 * 60 + d2;
                            if (y.bitis > y.baslangic) break;
                            else printf("Hata: Bitis saati baslangictan sonra olmali!\n");
                        } else printf("Hata: Gecersiz saat araligi!\n");
                    } else printf("Format yanlis (Orn: 09:00 10:30)! Tekrar deneyiniz.\n");
                    bufferTemizle();
                }

                sonIslemBasarili = 0;
                kok = randevuEkle(kok, y, 0);
                if (sonIslemBasarili) {
                    csvKaydet(y); push(&undoStack, y.id); jsonDosyasiOlustur(kok);
                    printf("Basariyla eklendi!\n");
                } else {
                    enqueue(&bekleme, y);
                    printf("Cakisiyor! Bekleme listesine alindi.\n");
                }
                break;
            }

            case 2: { // ATOMİK UNDO (Stack Pop)
                int id = pop(&undoStack);
                if (id != -1) {
                    kok = avlSil(kok, id); 
                    jsonDosyasiOlustur(kok);
                    printf("ID %d geri alindi ve agactan silindi.\n", id);
                } else printf("Geri alinacak islem yok.\n");
                break;
            }

            case 3: { // QUEUE DISPLAY
                KuyrukNode *t = bekleme.bas;
                printf("\n--- Bekleme Listesi (Queue) ---\n");
                if(!t) printf("Liste bos.\n");
                while(t) { 
                    printf("ID:%d | %s | Saat: %02d:%02d\n", 
                           t->veri.id, t->veri.isim, t->veri.baslangic/60, t->veri.baslangic%60); 
                    t = t->sonraki; 
                }
                break;
            }

            case 4: { // TASK QUEUE DEQUEUE (Gorev Kuyrugu)
                if (bekleme.bas != NULL) {
                    Randevu r = dequeue(&bekleme);
                    printf("%s yeniden deneniyor...\n", r.isim);
                    
                    sonIslemBasarili = 0;
                    kok = randevuEkle(kok, r, 0);
                    
                    if (sonIslemBasarili) { 
                        push(&undoStack, r.id); 
                        jsonDosyasiOlustur(kok);
                        printf("Basariyla eklendi!\n");
                    } else {                        enqueue(&bekleme, r);
                        printf("Hala cakisiyor, kuyruga geri atildi.\n");
                    }
                } else printf("Bekleyen randevu yok.\n");
                break;
            }

            case 5: seviyeGosterBFS(kok); break; // BFS

            case 6: enKisaYolBul(grafik, 0); break; // DIJKSTRA

            case 7: { // QUICKSORT
                Randevu liste[100]; int adet = diziyeAktar(kok, liste, 0);
                if (adet > 0) {
                    quicksort(liste, 0, adet - 1);
                    printf("\n--- ID Degerine Gore Sirali Randevular ---\n");
                    for(int i=0; i<adet; i++) printf("ID:%d | %s\n", liste[i].id, liste[i].isim);
                } else printf("Sistemde veri yok.\n");
                break;
            }

            case 8: { // BINARY SEARCH
                Randevu liste[100]; int adet = diziyeAktar(kok, liste, 0);
                if (adet > 0) {
                    quicksort(liste, 0, adet - 1);
                    int ara; printf("Aranacak ID: "); scanf("%d", &ara);
                    int snc = ikiliArama(liste, 0, adet - 1, ara);
                    if(snc != -1) printf("Bulundu: %s\n", liste[snc].isim);
                    else printf("ID %d bulunamadi!\n", ara);
                }
                break;
            }

            case 9: jsonDosyasiOlustur(kok); printf("JSON senkronize edildi.\n"); break;

            case 10: printf("Sistem kapatiliyor...\n"); break;

            default: printf("Gecersiz secim! Lutfen 1-10 arasi bir rakam giriniz.\n");
        }
    } while (secim != 10);

    return 0;
}