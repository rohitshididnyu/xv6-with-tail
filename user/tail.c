#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_LINE 128
#define MAX_LINES 1024

char buf[MAX_LINES][MAX_LINE];

void print_help() {
  printf("Usage: tail [-n N | -N | +N] [file]\n");
  printf("Print the last N lines of a file, or start from line +N.\n");
  printf("Examples:\n");
  printf("  tail file.txt           Show last 10 lines of file.txt\n");
  printf("  tail -n 5 file.txt      Show last 5 lines\n");
  printf("  tail -5 file.txt        Shorthand for last 5 lines\n");
  printf("  tail +3 file.txt        Show from line 3 to end\n");
  printf("  tail -n +3 file.txt     Same as above\n");
  printf("  tail                    Read from stdin\n");
}

int is_number(const char *s) {
  if (*s == '\0') return 0;
  for (int i = 0; s[i]; i++) {
    if (s[i] < '0' || s[i] > '9') return 0;
  }
  return 1;
}

int main(int argc, char *argv[]) {
  int fd = 0, n = 10, total = 0;
  int start_from_line = -1; // -1 means last N lines mode, else start from this line
  char line[MAX_LINE];
  int line_len = 0, c;

  if (argc >= 2 && strcmp(argv[1], "--help") == 0) {
    print_help();
    exit(0);
  }

  // Argument parsing
  if (argc == 1) {
    fd = 0;
  }
  else if (argc == 2) {
    // tail <filename> OR tail -N OR tail +N (stdin)
    if (argv[1][0] == '-' && is_number(argv[1] + 1)) {
      n = atoi(argv[1] + 1);
      fd = 0;
    } else if (argv[1][0] == '+' && is_number(argv[1] + 1)) {
      start_from_line = atoi(argv[1] + 1);
      fd = 0;
    } else {
      fd = open(argv[1], 0);
      if (fd < 0) {
        fprintf(2, "tail: cannot open %s\n", argv[1]);
        exit(1);
      }
    }
  }
  else if (argc == 3) {
    if (strcmp(argv[1], "-n") == 0) {
      if (argv[2][0] == '+') {
        if (!is_number(argv[2] + 1)) {
          fprintf(2, "tail: invalid number: %s\n", argv[2]);
          exit(1);
        }
        start_from_line = atoi(argv[2] + 1);
      } else {
        if (!is_number(argv[2])) {
          fprintf(2, "tail: invalid number: %s\n", argv[2]);
          exit(1);
        }
        n = atoi(argv[2]);
      }
      fd = 0;
    }
    else if (argv[1][0] == '-' && is_number(argv[1] + 1)) {
      n = atoi(argv[1] + 1);
      fd = open(argv[2], 0);
      if (fd < 0) {
        fprintf(2, "tail: cannot open %s\n", argv[2]);
        exit(1);
      }
    }
    else if (argv[1][0] == '+' && is_number(argv[1] + 1)) {
      start_from_line = atoi(argv[1] + 1);
      fd = open(argv[2], 0);
      if (fd < 0) {
        fprintf(2, "tail: cannot open %s\n", argv[2]);
        exit(1);
      }
    }
    else {
      fprintf(2, "Usage: tail [-n N | -N | +N] [file]\n");
      exit(1);
    }
  }
  else if (argc == 4 && strcmp(argv[1], "-n") == 0) {
    if (argv[2][0] == '+') {
      if (!is_number(argv[2] + 1)) {
        fprintf(2, "tail: invalid number: %s\n", argv[2]);
        exit(1);
      }
      start_from_line = atoi(argv[2] + 1);
    } else {
      if (!is_number(argv[2])) {
        fprintf(2, "tail: invalid number: %s\n", argv[2]);
        exit(1);
      }
      n = atoi(argv[2]);
    }
    fd = open(argv[3], 0);
    if (fd < 0) {
      fprintf(2, "tail: cannot open %s\n", argv[3]);
      exit(1);
    }
  }
  else {
    fprintf(2, "Usage: tail [-n N | -N | +N] [file]\n");
    exit(1);
  }

  if (n < 0) {
    fprintf(2, "tail: number of lines cannot be negative: %d\n", n);
    exit(1);
  }
  if (start_from_line == 0) start_from_line = 1;

  // Read all lines into circular buffer
  while (read(fd, &c, 1) == 1) {
    if (c == '\n' || line_len == MAX_LINE - 1) {
      line[line_len] = '\0';
      for (int j = 0; j < MAX_LINE; j++) {
        buf[total % MAX_LINES][j] = line[j];
        if (line[j] == '\0') break;
      }
      buf[total % MAX_LINES][MAX_LINE - 1] = '\0';
      total++;
      line_len = 0;
    } else {
      line[line_len++] = c;
    }
  }
  if (line_len > 0) {
    line[line_len] = '\0';
    for (int j = 0; j < MAX_LINE; j++) {
      buf[total % MAX_LINES][j] = line[j];
      if (line[j] == '\0') break;
    }
    buf[total % MAX_LINES][MAX_LINE - 1] = '\0';
    total++;
  }

  if (start_from_line != -1) {
    int start = start_from_line - 1;
    if (start < 0) start = 0;
    for (int i = start; i < total; i++) {
      printf("%s\n", buf[i % MAX_LINES]);
    }
  } else {
    int start = total > n ? total - n : 0;
    for (int i = start; i < total; i++) {
      printf("%s\n", buf[i % MAX_LINES]);
    }
  }

  if (fd > 0) close(fd);
  exit(0);
}