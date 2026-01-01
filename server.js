const express = require('express');
const { spawn } = require('child_process');
const path = require('path');
const app = express();

app.use(express.json());
app.use(express.static('.'));

app.post('/ekle', (req, res) => {
    const { id, isim, tarih, baslangic, bitis } = req.body;
    
    // Sunucu tarafında temel kontroller
    if (!id || !isim || !tarih || !baslangic || !bitis) {
        return res.json({ 
            success: false, 
            error: 'Eksik veri! Tüm alanları doldurun.' 
        });
    }
    
    // Tarih formatı kontrolü
    const tarihRegex = /^\d{4}-\d{2}-\d{2}$/;
    if (!tarihRegex.test(tarih)) {
        return res.json({ 
            success: false, 
            error: 'Geçersiz tarih formatı! (YYYY-AA-GG)' 
        });
    }
    
    // Saat kontrolü
    const [basSa, basDk] = baslangic.split(':').map(Number);
    const [bitSa, bitDk] = bitis.split(':').map(Number);
    
    if (isNaN(basSa) || isNaN(basDk) || isNaN(bitSa) || isNaN(bitDk) ||
        basSa > 23 || basDk > 59 || bitSa > 23 || bitDk > 59) {
        return res.json({ 
            success: false, 
            error: 'Geçersiz saat formatı!' 
        });
    }
    
    const basDakika = basSa * 60 + basDk;
    const bitDakika = bitSa * 60 + bitDk;
    
    if (basDakika >= bitDakika) {
        return res.json({ 
            success: false, 
            error: 'Başlangıç saati bitiş saatinden önce olmalı!' 
        });
    }
    
    if ((bitDakika - basDakika) < 15) {
        return res.json({ 
            success: false, 
            error: 'Randevu en az 15 dakika olmalı!' 
        });
    }
    
    const child = spawn(path.join(__dirname, 'randevusistemi.exe'));
    
    let output = '';
    let errorOutput = '';
    
    // C programının çıktısını yakala
    child.stdout.on('data', (data) => {
        output += data.toString();
    });
    
    child.stderr.on('data', (data) => {
        errorOutput += data.toString();
    });
    
    const input = `1\n${id}\n${isim}\n${tarih}\n${basSa}:${basDk} ${bitSa}:${bitDk}\n10\n`;
    
    console.log('C programına gönderilen input:', input);
    
    child.stdin.write(input);
    child.stdin.end();
    
    child.on('close', (code) => {
        console.log('C programı çıktısı:', output);
        console.log('C programı hata çıktısı:', errorOutput);
        
        // C programından gelen hata mesajlarını kontrol et
        if (output.includes('HATA:') || output.includes('cakisma') || 
            output.includes('esit olamaz') || output.includes('zaten mevcut')) {
            
            let hataMesaji = 'Randevu eklenemedi!';
            
            if (output.includes('zaten mevcut')) {
                hataMesaji = 'Bu ID zaten kullanılıyor!';
            } else if (output.includes('cakisma')) {
                hataMesaji = 'Bu saatte başka randevu var!';
            } else if (output.includes('esit olamaz')) {
                hataMesaji = 'Başlangıç saati bitiş saatinden önce olmalı!';
            }
            
            return res.json({ 
                success: false, 
                error: hataMesaji,
                cOutput: output 
            });
        }
        
        res.json({ 
            success: true,
            message: 'Randevu başarıyla eklendi!' 
        });
    });
});

app.post('/undo', (req, res) => {
    const child = spawn(path.join(__dirname, 'randevusistemi.exe'));
    
    let output = '';
    
    child.stdout.on('data', (data) => {
        output += data.toString();
    });
    
    child.stdin.write("2\n10\n");
    child.stdin.end();
    
    child.on('close', () => {
        res.json({ 
            success: true,
            message: 'Son işlem geri alındı.' 
        });
    });
});

app.listen(3000, () => console.log('Sistem http://localhost:3000 adresinde aktif.'));