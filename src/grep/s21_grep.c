#define _POSIX_C_SOURCE 200809L
#define GNU_SOURCE

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct options {
  int e, i, v, c, l, n, f, h, s, o;
  char **patterns, **files;
  int pattern_count, file_count, zero_flag, match_count, capacity;
} Options;

void find_match(Options flags);
Options parse_options(int argc, char *argv[]);
void print_match(const char *line, const char *file, int *line_count,
                 int *line_number, Options flags, int match, int *i, int *j,
                 int *match_count);
void flag_f(const char *file, Options *flags);
void switch_flags(int chosen_option, const char *optarg, Options *flags);
void process_line(Options flags, FILE *file, int *i);
void free_options(Options flags);

int main(int argc, char *argv[]) {
  Options flags = parse_options(argc, argv);
  find_match(flags);
  free_options(flags);
  return 0;
}

void find_match(Options flags) {
  for (int i = 0; i < flags.file_count; i++) {
    FILE *file = fopen(flags.files[i], "r");
    if (!file) {
      if (flags.s != 1) {
        fprintf(stderr, "grep: %s: No such file or directory\n",
                flags.files[i]);
      }
    } else {
      process_line(flags, file, &i);
    }
  }
}

void process_line(Options flags, FILE *file, int *i) {
  char *line = NULL;
  size_t len = 0;
  regex_t regex;
  int line_number = 1, line_count = 0, match_count = 0, last_string = 0,
      reg_flags = REG_EXTENDED;
  if (flags.i == 1) reg_flags |= REG_ICASE;
  while (getline(&line, &len, file) != -1) {
    for (int j = 0; j < flags.pattern_count; j++) {
      regcomp(&regex, flags.patterns[j], reg_flags);
      int match = regexec(&regex, line, 0, NULL, 0);
      if (match == 0 && flags.v != 1) {
        last_string = line_number;
      }
      if (match == 1 && flags.v == 1 && line[0] != '\n') {
        last_string = line_number;
      }
      print_match(line, flags.files[*i], &line_count, &line_number, flags,
                  match, i, &j, &match_count);
      regfree(&regex);
    }
  }
  if (flags.c == 1) {
    if (flags.file_count > 1) {
      printf("%s:%d\n", flags.files[*i], line_count);
    } else {
      printf("%d\n", line_count);
    }
  }
  if (flags.l == 1 && match_count > 0) {
    printf("%s\n", flags.files[*i]);
    match_count = 0;
  }
  line_number--;
  if (last_string == line_number && flags.c != 1 && flags.l != 1 &&
      flags.o != 1) {
    printf("\n");
  }
  line_count = 0;
  last_string = 1;
  line_number = 1;
  free(line);
  fclose(file);
}

void print_match(const char *line, const char *file, int *line_count,
                 int *line_number, Options flags, int match, int *i, int *j,
                 int *match_count) {
  if (flags.zero_flag != 1 && match == 0) {
    if (flags.file_count > 1) {
      printf("%s:%s", flags.files[*i], line);
    } else {
      printf("%s", line);
    }
    *j = flags.pattern_count;
  }
  if (flags.v == 1 && match != 0) {
    if (flags.file_count > 1) {
      printf("%s:%s", file, line);
    } else {
      printf("%s", line);
    }
    *j = flags.pattern_count;
  }
  if (flags.c == 1 && match == 0) {
    *line_count = *line_count + 1;
  }
  if (match == 0 && flags.l == 1) {
    *match_count = *match_count + 1;
  }
  if (flags.o == 1 && match == 0) {
    if (flags.file_count > 1) {
      printf("%s:%s\n", file, flags.patterns[*j]);
    } else {
      printf("%s\n", flags.patterns[*j]);
    }
    *j = flags.pattern_count;
  }
  if (match == 0 && flags.n == 1) {
    if (flags.file_count > 1) {
      printf("%s:%d:%s", flags.files[*i], *line_number, line);
    } else {
      printf("%d:%s", *line_number, line);
    }
    *j = flags.pattern_count;
  }
  if (match == 0 && flags.h == 1) {
    printf("%s", line);
    *j = flags.pattern_count;
  }
  *line_number = *line_number + 1;
}

Options parse_options(int argc, char *argv[]) {
  Options flags = {0};
  int chosen_option;
  flags.capacity = 10;
  flags.patterns = malloc(flags.capacity * sizeof(char *));
  while ((chosen_option = getopt_long(argc, argv, "e:f:ivclnhso", 0, 0)) !=
         -1) {
    switch_flags(chosen_option, optarg, &flags);
  }
  if (optind < argc) {
    if (flags.e != 1 && flags.f != 1) {
      flags.patterns[flags.pattern_count] =
          malloc((strlen(argv[optind]) + 1) * sizeof(char));
      strcpy(flags.patterns[flags.pattern_count], argv[optind]);
      flags.pattern_count++;
      optind++;
    }
  }

  flags.files = malloc((argc - optind) * sizeof(char *));
  for (int i = optind; i < argc; i++) {
    flags.files[flags.file_count] = argv[i];
    flags.file_count++;
  }
  return flags;
}

void switch_flags(int chosen_option, const char *optarg, Options *flags) {
  switch (chosen_option) {
    case 'e':
      flags->patterns[flags->pattern_count] =
          malloc((strlen(optarg) + 1) * sizeof(char));
      strcpy(flags->patterns[flags->pattern_count], optarg);
      flags->pattern_count++;
      flags->e = 1;
      break;
    case 'f':
      free_options(*flags);
      flags->f = 1;
      flag_f(optarg, flags);
      break;
    case 'i':
      flags->i = 1;
      break;
    case 'v':
      flags->v = 1;
      flags->zero_flag = 1;
      break;
    case 'c':
      flags->c = 1;
      flags->zero_flag = 1;
      break;
    case 'l':
      flags->l = 1;
      flags->zero_flag = 1;
      break;
    case 'n':
      flags->n = 1;
      flags->zero_flag = 1;
      break;
    case 'h':
      flags->h = 1;
      flags->zero_flag = 1;
      break;
    case 's':
      flags->s = 1;
      break;
    case 'o':
      flags->o = 1;
      flags->zero_flag = 1;
      break;
    default:
      break;
  }
}

void flag_f(const char *namefile, Options *flags) {
  FILE *file = fopen(namefile, "r");
  char *line = NULL;
  size_t len = 0;
  flags->patterns = malloc(flags->capacity * sizeof(char *));
  if (!file) {
    fprintf(stderr, "grep: %s: No such file or directory\n", namefile);
  } else {
    while (getline(&line, &len, file) != -1) {
      line[strcspn(line, "\n")] = '\0';
      if (flags->pattern_count >= flags->capacity) {
        flags->capacity *= 2;
        flags->patterns =
            realloc(flags->patterns, flags->capacity * sizeof(char *));
      }
      flags->patterns[flags->pattern_count] =
          malloc((strlen(line) + 1) * sizeof(char));
      strcpy(flags->patterns[flags->pattern_count], line);
      flags->pattern_count++;
    }
  }
  free(line);
  fclose(file);
}

void free_options(Options flags) {
  for (int i = 0; i < flags.pattern_count; i++) {
    free(flags.patterns[i]);
  }
  free(flags.patterns);
  free(flags.files);
}