# SISOP-2-2026-IT-085

Pengerjaan Praktikum Sistem Operasi Modul 2

- [Soal 1 - Kasbon Warga Kampung Durian Runtuh](#soal-1---kasbon-warga-kampung-durian-runtuh)
- [Soal 2 - The world never stops, even when you feel tired](#soal-2---the-world-never-stops-even-when-you-feel-tired)
- [Soal 3 - One letter for destiny](#soal-3---one-letter-for-destiny)

---

| Nama | NRP |
| --- | --- |
| Marvelino Davas | 5027241085 |

---

## Soal 1 - Kasbon Warga Kampung Durian Runtuh

### Penjelasan

Program `kasir_muthu.c` dibuat untuk membantu Uncle Muthu mengamankan data buku hutang yang terancam virus. Program menggunakan Sequential Process dengan kombinasi `fork()`, `execlp()`, dan `waitpid()` untuk menjalankan empat child process secara berurutan.

**Child 1** membuat folder `brankas_kedai` menggunakan command `mkdir`:

```c
execlp("mkdir", "mkdir", "brankas_kedai", NULL);
```

**Child 2** menyalin `buku_hutang.csv` ke dalam `brankas_kedai` menggunakan command `cp`:

```c
execlp("cp", "cp", "buku_hutang.csv", "brankas_kedai/", NULL);
```

**Child 3** mencari semua data pelanggan berstatus `"Belum Lunas"` lalu menyimpannya ke `daftar_penunggak.txt`. Menggunakan `bash -c` karena operator `>` adalah fitur shell, bukan fitur `grep` secara langsung:

```c
execlp("bash", "bash", "-c", "grep \"Belum Lunas\" brankas_kedai/buku_hutang.csv > brankas_kedai/daftar_penunggak.txt", NULL);
```

**Child 4** mengompres folder `brankas_kedai` menjadi `rahasia_muthu.zip` menggunakan command `zip -r`:

```c
execlp("zip", "zip", "-r", "rahasia_muthu.zip", "brankas_kedai/", NULL);
```

Setiap child harus selesai sebelum child berikutnya dijalankan. Setelah setiap `waitpid()`, program mengecek status keberhasilan menggunakan `WIFEXITED` dan `WEXITSTATUS`. Jika ada child yang gagal, program langsung berhenti dan mencetak pesan error. Jika seluruh proses berhasil, program mencetak pesan sukses.

### Cara Penggunaan

```bash
gcc kasir_muthu.c -o kasir_muthu
./kasir_muthu
```

### Kendala

Tidak ada kendala.

---

## Soal 2 - The world never stops, even when you feel tired

### Penjelasan

Program `contract_daemon.c` adalah daemon yang berjalan di background untuk memantau keberadaan dan integritas file `contract.txt`, serta menulis log ke `work.log` secara berkala.

**Inisialisasi daemon** mengikuti langkah standar dari template modul: `fork()` untuk memisahkan dari parent, `setsid()` untuk melepas diri dari terminal, `chdir()` untuk pindah ke direktori kerja, lalu menutup `stdin`, `stdout`, dan `stderr`.

**Logging berkala** — Setiap 5 detik daemon menulis ke `work.log` dengan salah satu dari tiga status acak: `[awake]`, `[drifting]`, atau `[numbness]`. Contoh output:

```
still working… [awake]
```

**Pembuatan contract.txt** — Saat pertama kali berjalan, daemon membuat file `contract.txt` berisi kalimat kontrak dan timestamp `created at`.

**Restore jika dihapus** — Daemon mengecek keberadaan file setiap 1 detik menggunakan `fopen()`. Jika file tidak ditemukan, daemon langsung membuat ulang file tersebut dengan label `restored at` beserta timestamp.

**Deteksi perubahan isi** — Jika baris pertama `contract.txt` tidak sesuai dengan kalimat aslinya, daemon mencatat `contract violated.` ke `work.log` lalu merestore isi file.

**Pesan saat dihentikan** — Daemon menangkap sinyal `SIGTERM` menggunakan `signal()`. Ketika sinyal diterima, loop berhenti dan daemon menulis pesan terakhir ke `work.log`:

```
We really weren't meant to be together
```

### Cara Penggunaan

```bash
gcc contract_daemon.c -o contract_daemon
./contract_daemon
```

Untuk menghentikan daemon:

```bash
ps aux | grep contract_daemon
kill <PID>
```

### Kendala

Tidak ada kendala.

---

## Soal 3 - One letter for destiny

### Penjelasan

Program `angel.c` adalah daemon yang berjalan di background dengan nama proses `maya`. Program menyediakan tiga mode operasi yang dijalankan melalui argumen command line.

**Inisialisasi daemon** mengikuti template modul. Nama proses diubah menjadi `maya` menggunakan dua pendekatan: `prctl(PR_SET_NAME, "maya", ...)` untuk mengubah nama di kernel, dan manipulasi `argv[0]` langsung untuk mengubah tampilan di kolom CMD pada `ps aux`. PID daemon disimpan ke `/tmp/angel.pid` untuk keperluan fitur kill.

**Fitur secret** berjalan otomatis di dalam daemon setiap 10 detik. Fitur ini memilih satu kalimat secara acak dari empat kalimat yang tersedia lalu menulisnya ke `LoveLetter.txt`.

**Fitur surprise** berjalan otomatis setelah secret selesai. Fitur ini membaca isi `LoveLetter.txt`, mengenkripsinya menggunakan algoritma Base64 yang diimplementasikan manual, lalu menuliskan hasil enkripsi kembali ke file yang sama.

**Fitur decrypt** dipanggil secara eksplisit via `./angel -decrypt`. Fitur ini membaca isi `LoveLetter.txt` yang sudah terenkripsi, mendekodenya kembali ke bentuk asli menggunakan tabel Base64, lalu menyimpan hasilnya ke file yang sama.

**Fitur kill** dipanggil via `./angel -kill`. Fitur ini membaca PID dari `/tmp/angel.pid` lalu mengirim sinyal `SIGTERM` ke daemon. Jika file PID tidak ditemukan, program menampilkan pesan error bahwa daemon belum berjalan.

**Logging** — Seluruh aktivitas dicatat ke `ethereal.log` dengan format:

```
[dd:mm:yyyy]-[hh:mm:ss]_nama-proses_STATUS
```

Status yang digunakan adalah `RUNNING` saat proses mulai, `SUCCESS` saat berhasil, dan `ERROR` saat terjadi kegagalan.

### Cara Penggunaan

```bash
gcc angel.c -o angel
./angel           # tampilkan usage
./angel -daemon   # jalankan daemon (nama proses: maya)
./angel -decrypt  # decrypt LoveLetter.txt
./angel -kill     # hentikan daemon
```

### Kendala

Tidak ada kendala.