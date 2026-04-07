#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define CONTRACT_FILE "contract.txt"
#define WORK_LOG "work.log"

const char *statuses[] = {"[awake]", "[drifting]", "[numbness]"};
volatile sig_atomic_t running = 1;

void write_log(const char *msg) {
    FILE *f = fopen(WORK_LOG, "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

void create_contract(const char *label) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    FILE *f = fopen(CONTRACT_FILE, "w");
    if (f) {
        fprintf(f, "\"A promise to keep going, even when unseen.\"\n\n%s: %s\n", label, timestamp);
        fclose(f);
    }
}

void handle_sigterm(int sig) {
    running = 0;
}

int main() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    pid_t pid, sid;
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    sid = setsid();
    if (sid < 0) exit(EXIT_FAILURE);
    if (chdir(cwd) < 0) exit(EXIT_FAILURE);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    signal(SIGTERM, handle_sigterm);
    srand(time(NULL));

    // Buat contract.txt saat daemon start
    create_contract("created at");

    int counter = 0;

    while (running) {
        sleep(1);
        counter++;

        // Cek apakah contract.txt ada
        FILE *f = fopen(CONTRACT_FILE, "r");
        if (!f) {
            // File dihapus, buat ulang
            create_contract("restored at");
        } else {
            // Cek apakah isi berubah
            char first_line[256];
            fgets(first_line, sizeof(first_line), f);
            fclose(f);

            if (strncmp(first_line, "\"A promise to keep going, even when unseen.\"", 44) != 0) {
                write_log("contract violated.");
                create_contract("restored at");
            }
        }

        // Tulis log setiap 5 detik
        if (counter % 5 == 0) {
            int idx = rand() % 3;
            char msg[64];
            snprintf(msg, sizeof(msg), "still working\xe2\x80\xa6 %s", statuses[idx]);
            write_log(msg);
        }
    }

    write_log("We really weren't meant to be together");
    return 0;
}