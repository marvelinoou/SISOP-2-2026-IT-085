#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <signal.h>


#define LOVE_LETTER "LoveLetter.txt"
#define ETHEREAL_LOG "ethereal.log"
#define PID_FILE "/tmp/angel.pid"

const char *sentences[] = {
    "aku akan fokus pada diriku sendiri",
    "aku mencintaimu dari sekarang hingga selamanya",
    "aku akan menjauh darimu, hingga takdir mempertemukan kita di versi kita yang terbaik.",
    "kalau aku dilahirkan kembali, aku tetap akan terus menyayangimu"
};

static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void write_log(const char *process, const char *status) {
    FILE *f = fopen(ETHEREAL_LOG, "a");
    if (!f) return;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(f, "[%02d:%02d:%04d]-[%02d:%02d:%02d]_%s_%s\n",
        t->tm_mday, t->tm_mon+1, t->tm_year+1900,
        t->tm_hour, t->tm_min, t->tm_sec,
        process, status);
    fclose(f);
}

void base64_encode(const char *input, int len, char *output) {
    int i = 0, j = 0;
    while (i < len) {
        unsigned int a = i < len ? (unsigned char)input[i++] : 0;
        unsigned int b = i < len ? (unsigned char)input[i++] : 0;
        unsigned int c = i < len ? (unsigned char)input[i++] : 0;
        unsigned int triple = (a << 16) | (b << 8) | c;
        output[j++] = b64chars[(triple >> 18) & 0x3F];
        output[j++] = b64chars[(triple >> 12) & 0x3F];
        output[j++] = b64chars[(triple >> 6) & 0x3F];
        output[j++] = b64chars[triple & 0x3F];
    }
    int mod = len % 3;
    if (mod == 1) { output[j-2] = '='; output[j-1] = '='; }
    else if (mod == 2) { output[j-1] = '='; }
    output[j] = '\0';
}

void base64_decode(const char *input, char *output) {
    unsigned char dtable[256];
    memset(dtable, 0x80, 256);
    for (int i = 0; i < 64; i++) dtable[(unsigned char)b64chars[i]] = i;
    dtable['='] = 0;
    int len = strlen(input), j = 0;
    for (int i = 0; i < len; i += 4) {
        unsigned int a = dtable[(unsigned char)input[i]];
        unsigned int b = dtable[(unsigned char)input[i+1]];
        unsigned int c = dtable[(unsigned char)input[i+2]];
        unsigned int d = dtable[(unsigned char)input[i+3]];
        output[j++] = (a << 2) | (b >> 4);
        if (input[i+2] != '=') output[j++] = (b << 4) | (c >> 2);
        if (input[i+3] != '=') output[j++] = (c << 6) | d;
    }
    output[j] = '\0';
}

void do_secret() {
    write_log("secret", "RUNNING");
    srand(time(NULL));
    int idx = rand() % 4;
    FILE *f = fopen(LOVE_LETTER, "w");
    if (!f) { write_log("secret", "ERROR"); return; }
    fprintf(f, "%s", sentences[idx]);
    fclose(f);
    write_log("secret", "SUCCESS");
}

void do_surprise() {
    write_log("surprise", "RUNNING");
    FILE *f = fopen(LOVE_LETTER, "r");
    if (!f) { write_log("surprise", "ERROR"); return; }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);
    char *content = malloc(fsize + 1);
    fread(content, 1, fsize, f);
    content[fsize] = '\0';
    fclose(f);

    char *encoded = malloc(fsize * 2 + 4);
    base64_encode(content, fsize, encoded);
    free(content);

    f = fopen(LOVE_LETTER, "w");
    if (!f) { free(encoded); write_log("surprise", "ERROR"); return; }
    fprintf(f, "%s", encoded);
    fclose(f);
    free(encoded);
    write_log("surprise", "SUCCESS");
}

void do_decrypt() {
    write_log("decrypt", "RUNNING");
    FILE *f = fopen(LOVE_LETTER, "r");
    if (!f) {
        write_log("decrypt", "ERROR");
        printf("Error: LoveLetter.txt tidak ditemukan.\n");
        return;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);
    char *content = malloc(fsize + 1);
    fread(content, 1, fsize, f);
    content[fsize] = '\0';
    fclose(f);

    char *decoded = malloc(fsize + 1);
    base64_decode(content, decoded);
    free(content);

    f = fopen(LOVE_LETTER, "w");
    if (!f) { free(decoded); write_log("decrypt", "ERROR"); return; }
    fprintf(f, "%s", decoded);
    fclose(f);
    free(decoded);
    write_log("decrypt", "SUCCESS");
    printf("LoveLetter.txt berhasil didekripsi.\n");
}

void do_kill() {
    write_log("kill", "RUNNING");
    FILE *pf = fopen(PID_FILE, "r");
    if (!pf) {
        write_log("kill", "ERROR");
        printf("Error: daemon belum berjalan.\n");
        return;
    }
    pid_t pid;
    fscanf(pf, "%d", &pid);
    fclose(pf);

    if (kill(pid, SIGTERM) == 0) {
        remove(PID_FILE);
        write_log("kill", "SUCCESS");
        printf("Daemon berhasil dihentikan.\n");
    } else {
        write_log("kill", "ERROR");
        printf("Error: gagal menghentikan daemon.\n");
    }
}

void run_daemon(int argc, char *argv[]) {
    pid_t pid, sid;

    // Fork dari parent
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);  // parent exit

    umask(0);

    sid = setsid();
    if (sid < 0) exit(EXIT_FAILURE);

    //if (chdir() < 0) exit(EXIT_FAILURE);

    // Simpan PID
    FILE *pf = fopen(PID_FILE, "w");
    if (pf) { fprintf(pf, "%d\n", getpid()); fclose(pf); }

    // Ubah nama proses jadi "maya"
    prctl(PR_SET_NAME, "maya", 0, 0, 0);

    char *m = argv[argc - 1] + strlen(argv[argc - 1]);
    memset(argv[0], 0, m - argv[0]);
    strcpy(argv[0], "maya");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Loop utama daemon
    while (1) {
        do_secret();
        do_surprise();
        sleep(10);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Penggunaan:\n");
        printf("  ./angel -daemon  : jalankan sebagai daemon (nama proses: maya)\n");
        printf("  ./angel -decrypt : decrypt LoveLetter.txt\n");
        printf("  ./angel -kill    : kill proses\n");
        return 0;
    }

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    if (strcmp(argv[1], "-daemon") == 0) {
        run_daemon(argc, argv);
    } else if (strcmp(argv[1], "-decrypt") == 0) {
        do_decrypt();
    } else if (strcmp(argv[1], "-kill") == 0) {
        do_kill();
    } else {
        printf("Penggunaan:\n");
        printf("  ./angel -daemon  : jalankan sebagai daemon (nama proses: maya)\n");
        printf("  ./angel -decrypt : decrypt LoveLetter.txt\n");
        printf("  ./angel -kill    : kill proses\n");
    }

    return 0;
}