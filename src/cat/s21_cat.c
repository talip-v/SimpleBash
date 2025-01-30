#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct options {
  int b, E, n, s, T, v;
  int number_of_str;
} Options;

void read_file(char *name, Options flags);
Options parse_options(int argc, char *argv[], int *count_of_flags);

int main(int argc, char *argv[]) {
  int count_of_flags = 0;
  Options flags = parse_options(argc, argv, &count_of_flags);
  if (flags.n == 1 && flags.b == 1) {
    flags.n = 0;
  }
  flags.number_of_str = 1;
  for (int i = count_of_flags + 1; i < argc; i++) {
    read_file(argv[i], flags);
  }
  return 0;
}

void read_file(char *name, Options flags) {
  FILE *file = fopen(name, "r");
  int number_of_str = 1, count_of_empty_str = 0, symbol, flag_putchar = 0;
  char previous_symbol = '\n';
  if (file == NULL) {
    printf("cat: %s: No such file or directory", name);
  } else if (file != NULL) {
    while ((symbol = fgetc(file)) != EOF) {
      if (flags.s == 1 && symbol == '\n' && previous_symbol == '\n') {
        count_of_empty_str++;
        if (count_of_empty_str >= 2) flag_putchar = 1;
      }
      if (flags.n == 1 && (previous_symbol == '\n' || number_of_str == 1)) {
        printf("%6d\t", number_of_str);
        number_of_str++;
      }
      if (flags.b == 1 && symbol != '\n' &&
          (previous_symbol == '\n' || number_of_str == 1)) {
        printf("%6d\t", number_of_str);
        number_of_str++;
      }
      if (flags.v == 1 && (symbol < 32 || symbol == 127)) {
        if (symbol < 32 && symbol != '\n' && symbol != '\t') {
          printf("^%c", symbol + 64);
          flag_putchar = 1;
        }
        if (symbol == 127) {
          printf("^?");
          flag_putchar = 1;
        }
      }
      if (flags.E == 1 && symbol == '\n') printf("$");
      if (flags.T == 1 && symbol == '\t') {
        printf("^I");
        flag_putchar = 1;
      }
      if (symbol != '\n') count_of_empty_str = 0;
      if (flag_putchar == 0) putchar(symbol);
      flag_putchar = 0;
      previous_symbol = symbol;
    }
  }
  fclose(file);
}

Options parse_options(int argc, char *argv[], int *count_of_flags) {
  Options flags = {0};
  struct option GNU_options[] = {{"number-nonblank", no_argument, NULL, 'b'},
                                 {"number", no_argument, NULL, 'n'},
                                 {"squeeze-blank", no_argument, NULL, 's'},
                                 {0, 0, 0, 0}};
  int chosen_option;
  while ((chosen_option =
              getopt_long(argc, argv, "+beEnstTv", GNU_options, 0)) != -1) {
    switch (chosen_option) {
      case 'b':
        flags.b = 1;
        break;
      case 'e':
        flags.E = 1;
        flags.v = 1;
        break;
      case 'E':
        flags.E = 1;
        break;
      case 'n':
        flags.n = 1;
        break;
      case 's':
        flags.s = 1;
        break;
      case 't':
        flags.T = 1;
        flags.v = 1;
        break;
      case 'T':
        flags.T = 1;
        break;
      case 'v':
        flags.v = 1;
        break;
      default:
        break;
    }
    (*count_of_flags)++;
  }

  return flags;
}
