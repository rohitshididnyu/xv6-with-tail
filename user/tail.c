#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_LINE 128
#define MAX_LINES 1024

char buf[MAX_LINES][MAX_LINE];

void print_help() {
  printf("Usage: tail [-n N] [file]\n");
  printf("Print the last N lines of a file or from standard input.\n");
  printf("Examples:\n");
  printf("  tail file.txt           Show last 10 lines of file.txt\n");
  printf("  tail -n 5 file.txt      Show last 5 lines\n");
  printf("  tail -n 0 file.txt      Show nothing\n");
  printf("  tail                    Read from stdin\n");
}

int main(int argc, char *argv[]) {
  int fd = 0, n = 10, total = 0;
  char line[MAX_LINE];
  int line_len = 0, c;
  int from_pipe = 1;

  // Handle help
  if (argc >= 2 && strcmp(argv[1], "--help") == 0) {
    print_help();
    exit(0);
  }

  // Argument Parsing
  if (argc == 1) {
    // tail
    fd = 0;
  } else if (argc == 2) {
    // tail file
    from_pipe = 0;
    fd = open(argv[1], 0);
    if (fd < 0) {
      fprintf(2, "tail: cannot open %s\n", argv[1]);
      exit(1);
    }
  } else if (argc == 3 && strcmp(argv[1], "-n") == 0) {
    // tail -n N
    n = atoi(argv[2]);
    if (n < 0) {
      fprintf(2, "tail: invalid line count\n");
      exit(1);
    }
    fd = 0;
  } else if (argc == 4 && strcmp(argv[1], "-n") == 0) {
    // tail -n N file
    n = atoi(argv[2]);
    if (n < 0) {
      fprintf(2, "tail: invalid line count\n");
      exit(1);
    }
    from_pipe = 0;
    fd = open(argv[3], 0);
    if (fd < 0) {
      fprintf(2, "tail: cannot open %s\n", argv[3]);
      exit(1);
    }
  } else {
    fprintf(2, "Usage: tail [-n N] [file]\n");
    exit(1);
  }

  // Read input one char at a time
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

  // Handle input that ends without newline
  if (line_len > 0) {
    line[line_len] = '\0';
    for (int j = 0; j < MAX_LINE; j++) {
      buf[total % MAX_LINES][j] = line[j];
      if (line[j] == '\0') break;
    }
    buf[total % MAX_LINES][MAX_LINE - 1] = '\0';
    total++;
  }

  // Output last N lines
  int start = total > n ? total - n : 0;
  for (int i = start; i < total; i++) {
    printf("%s\n", buf[i % MAX_LINES]);
  }

  if (!from_pipe) close(fd);
  exit(0);
}
