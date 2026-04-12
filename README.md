# Laporan Resmi Praktikum Sistem Operasi
## Modul 2 - Proses dan Daemon

| Nama | Marvelino Davas |
|------|-----------------|
| NRP | 5027241085 |
| Kelas | IT |

---

## Soal 1 - Kasbon Warga Kampung Durian Runtuh

### Deskripsi
Program `kasir_muthu.c` mengamankan data buku hutang milik Uncle Muthu secara otomatis menggunakan Sequential Process. Program menggunakan kombinasi `fork()`, `execlp()`, dan `waitpid()` untuk menjalankan empat proses child secara berurutan.

### Cara Kerja

Program membuat empat child process secara berurutan. Setiap child harus selesai sebelum child berikutnya dijalankan menggunakan `waitpid()`.

**Child 1** — Membuat folder `brankas_kedai`:

    execlp("mkdir", "mkdir", "brankas_kedai", NULL);

**Child 2** — Menyalin `buku_hutang.csv` ke dalam `brankas_kedai`:

    execlp("cp", "cp", "buku_hutang.csv", "brankas_kedai/", NULL);

**Child 3** — Mencari data pelanggan berstatus `"Belum Lunas"` dan menyimpannya ke `daftar_penunggak.txt`:

    execlp("bash", "bash", "-c", "grep \"Belum Lunas\" brankas_kedai/buku_hutang.csv > brankas_kedai/daftar_penunggak.txt", NULL);

Menggunakan `bash -c` karena operator `>` adalah fitur shell, bukan fitur `grep` langsung.

**Child 4** — Mengompres `brankas_kedai` menjadi `rahasia_muthu.zip`:

    execlp("zip", "zip", "-r", "rahasia_muthu.zip", "brankas_kedai/", NULL);

**Error Handling** — Setiap setelah `waitpid()`, program mengecek status keberhasilan child. Jika gagal, program mencetak pesan error dan berhenti:

    [ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.

Jika seluruh proses berhasil, program mencetak:

    [INFO] Fuhh, selamat! Buku hutang dan daftar penagihan berhasil diamankan.

### Cara Menjalankan

    gcc kasir_muthu.c -o kasir_muthu
    ./kasir_muthu

### Struktur File yang Dihasilkan

    soal_1/
    ├── buku_hutang.csv
    ├── kasir_muthu.c
    ├── rahasia_muthu.zip
    └── brankas_kedai/
        ├── buku_hutang.csv
        └── daftar_penunggak.txt

---

## Soal 2 - The world never stops, even when you feel tired

### Deskripsi
Program `contract_daemon.c` adalah sebuah daemon yang berjalan di background dan memantau keberadaan serta integritas file `contract.txt`. Daemon menulis log ke `work.log` secara berkala selama berjalan.

### Cara Kerja

**Inisialisasi Daemon** — Program mengikuti langkah standar pembuatan daemon:
1. `fork()` — parent exit, child lanjut di background
2. `setsid()` — melepas diri dari terminal
3. `chdir()` — pindah ke direktori kerja
4. Menutup `stdin`, `stdout`, `stderr`

**1. Logging berkala** — Setiap 5 detik, daemon menulis ke `work.log` dengan salah satu status acak:

    still working… [awake]
    still working… [drifting]
    still working… [numbness]

**2. Pembuatan contract.txt** — Saat daemon pertama kali dijalankan, dibuat file `contract.txt` berisi:

    "A promise to keep going, even when unseen."

    created at: YYYY-MM-DD HH:MM:SS

**3. Restore jika dihapus** — Daemon mengecek keberadaan `contract.txt` setiap 1 detik. Jika file dihapus, daemon membuat ulang file tersebut dengan baris kedua:

    restored at: <timestamp>

**4. Deteksi perubahan isi** — Jika isi `contract.txt` diubah, daemon menulis `contract violated.` ke `work.log` dan merestore isi file ke kondisi semula.

**5. Pesan saat dihentikan** — Saat daemon menerima sinyal `SIGTERM`, daemon menulis pesan terakhir ke `work.log`:

    We really weren't meant to be together

### Cara Menjalankan

    gcc contract_daemon.c -o contract_daemon
    ./contract_daemon

### Cara Menghentikan

    ps aux | grep contract_daemon
    kill <PID>

---

## Soal 3 - One letter for destiny

### Deskripsi
Program `angel.c` adalah daemon yang berjalan di background dengan nama proses `maya`. Program memiliki tiga mode operasi yang dijalankan melalui argumen command line.

### Cara Kerja

**Inisialisasi Daemon** — Sama seperti soal 2, mengikuti langkah standar pembuatan daemon. Nama proses diubah menjadi `maya` menggunakan dua cara:

    prctl(PR_SET_NAME, "maya", 0, 0, 0);  // ubah nama di kernel
    strcpy(argv[0], "maya");               // ubah tampilan di ps aux

**1. Fitur secret** — Dijalankan otomatis di dalam daemon setiap 10 detik. Menulis satu kalimat acak ke `LoveLetter.txt` dari daftar berikut:
- aku akan fokus pada diriku sendiri
- aku mencintaimu dari sekarang hingga selamanya
- aku akan menjauh darimu, hingga takdir mempertemukan kita di versi kita yang terbaik.
- kalau aku dilahirkan kembali, aku tetap akan terus menyayangimu

**2. Fitur surprise** — Dijalankan otomatis setelah secret. Mengenkripsi isi `LoveLetter.txt` menggunakan Base64.

**3. Fitur decrypt** — Dipanggil secara eksplisit via `./angel -decrypt`. Mendekripsi isi `LoveLetter.txt` kembali ke bentuk aslinya.

**4. Fitur kill** — Dipanggil via `./angel -kill`. Membaca PID daemon dari `/tmp/angel.pid` lalu mengirim `SIGTERM`.

**5. Logging** — Semua aktivitas dicatat ke `ethereal.log` dengan format:

    [dd:mm:yyyy]-[hh:mm:ss]_nama-proses_STATUS

Status yang digunakan: `RUNNING`, `SUCCESS`, `ERROR`.

### Cara Menjalankan

    gcc angel.c -o angel
    ./angel          # tampilkan usage
    ./angel -daemon  # jalankan daemon (nama proses: maya)
    ./angel -decrypt # decrypt LoveLetter.txt
    ./angel -kill    # hentikan daemon

### Struktur File yang Dihasilkan

    soal_3/
    ├── angel.c
    ├── LoveLetter.txt
    └── ethereal.log