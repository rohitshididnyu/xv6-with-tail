#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAX_LINES 1024
#define MAX_LINE_LEN 512

void print_help(void) {
    printf("Usage: tail [OPTION]... [FILE]\n");
    printf("Print the last N lines (or from Nth line for +N) of FILE to standard output.\n\n");
    printf("Options:\n");
    printf("  -n N       Output the last N lines of the file\n");
    printf("  +N         Output lines starting from the Nth line (1-indexed)\n");
    printf("  --help     Show this help message\n\n");
    printf("Notes:\n");
    printf("  • For -n N, output is limited to the last %d lines (circular buffer)\n", MAX_LINES);
    printf("  • +N format directly skips the first N-1 lines without size restriction\n\n");
    printf("Examples:\n");
    printf("  tail file.txt          Show last 10 lines\n");
    printf("  tail -n 5 file.txt     Show last 5 lines\n");
    printf("  tail +5 file.txt       Show from 5th line onward\n");
}

void tail_from_plus_n(int fd, int start_line) {
    if (start_line < 1) start_line = 1;
    int c, line_count = 1;
    char ch;
    while ((c = read(fd, &ch, 1)) > 0) {
        if (line_count >= start_line) {
            write(1, &ch, 1);
        }
        if (ch == '\n') {
            line_count++;
        }
    }
}

void tail_last_n(int fd, int n) {
    if (n <= 0) return;
    char *lines[MAX_LINES];
    char buf[MAX_LINE_LEN];
    int head = 0, total = 0;
    int len = 0;
    char ch;

    for (int i = 0; i < MAX_LINES; i++) {
        lines[i] = 0;
    }

    while (read(fd, &ch, 1) > 0) {
        if (len < MAX_LINE_LEN - 1) {
            buf[len++] = ch;
        }
        if (ch == '\n') {
            buf[len] = '\0';
            if (lines[head]) free(lines[head]);
            lines[head] = malloc(len + 1);
            memmove(lines[head], buf, len + 1);
            head = (head + 1) % MAX_LINES;
            total++;
            len = 0;
        }
    }

    int start = (total > n) ? total - n : 0;
    int idx = (head + MAX_LINES - (total > n ? n : total)) % MAX_LINES;
    for (int i = start; i < total; i++) {
        if (lines[idx]) {
            printf("%s", lines[idx]);
            free(lines[idx]);
        }
        idx = (idx + 1) % MAX_LINES;
    }
}

int main(int argc, char *argv[]) {
    int fd, n = 10;
    int plus_mode = 0;
    char *filename = 0;

    if (argc < 2) {
        print_help();
        exit(1);
    }

    if (strcmp(argv[1], "--help") == 0) {
        print_help();
        exit(0);
    }

    if (argv[1][0] == '-' && argv[1][1] != 'n') {
        // Handle shorthand like -5
        if (argv[1][1] >= '0' && argv[1][1] <= '9') {
            printf("tail: option requires an argument -- n\n");
            exit(1);
        }
    }

    if (argv[1][0] == '+') {
        plus_mode = 1;
        n = atoi(argv[1] + 1);
        if (argc < 3) {
            printf("tail: missing file operand\n");
            exit(1);
        }
        filename = argv[2];
    } else if (strcmp(argv[1], "-n") == 0) {
        if (argc < 3) {
            printf("tail: option requires an argument -- n\n");
            exit(1);
        }
        n = atoi(argv[2]);
        if (argc < 4) {
            printf("tail: missing file operand\n");
            exit(1);
        }
        filename = argv[3];
    } else {
        // tail <file>
        filename = argv[1];
    }

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("tail: cannot open '%s'\n", filename);
        exit(1);
    }

    if (plus_mode) {
        tail_from_plus_n(fd, n);
    } else {
        tail_last_n(fd, n);
    }

    close(fd);
    exit(0);
}