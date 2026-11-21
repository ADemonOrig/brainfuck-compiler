#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "bf.h"

bool arg_is_input(char *arg) {
    return (
        (strcmp(arg, "--") == 0) ||
        (strcmp(arg, "-f") == 0) ||
        (strcmp(arg, "-file") == 0) ||
        (strcmp(arg, "-i") == 0) ||
        (strcmp(arg, "-input") == 0)
    );
}

bool arg_is_output(char *arg) {
    return (
        (strcmp(arg, "-o") == 0) ||
        (strcmp(arg, "-output") == 0)
    );
}

bool arg_is_string(char *arg) {
    return (
        (strcmp(arg, "-") == 0) ||
        (strcmp(arg, "-s") == 0) ||
        (strcmp(arg, "-str") == 0) ||
        (strcmp(arg, "-string") == 0)
    );
}

bool arg_is_size(char *arg) {
    return (
        (strcmp(arg, "-c") == 0) ||
        (strcmp(arg, "-cells") == 0) ||
        (strcmp(arg, "-count") == 0) ||
        (strcmp(arg, "-count-cells") == 0) ||
        (strcmp(arg, "-n") == 0)
    );
}

bool arg_is_optim(char *arg) {
    return (
        (strcmp(arg, "-optim") == 0) ||
        (strcmp(arg, "-level") == 0) ||
        (strcmp(arg, "-optim-level") == 0) ||
        (strcmp(arg, "-level-optim") == 0)
    );
}

bool arg_is_run(char *arg) {
    return (
        (strcmp(arg, "-r") == 0) ||
        (strcmp(arg, "-run") == 0)
    );
}

bool arg_is_onlyrun(char *arg) {
    return (
        (strcmp(arg, "-ro") == 0) ||
        (strcmp(arg, "-or") == 0) ||
        (strcmp(arg, "-run-only") == 0) ||
        (strcmp(arg, "-only-run") == 0)
    );
}

bool arg_is_help(char *arg) {
    return (
        (strcmp(arg, "-h") == 0) ||
        (strcmp(arg, "-help") == 0)
    );
}

size_t args_parse(struct brainfuck *bf, int argc, char **argv) {
    size_t count_help = 0;
    for (uint64_t i = 1; i < argc; i++) {
        if (arg_is_help(argv[i])) {
            count_help++;
        }
        else if (arg_is_input(argv[i])) {
            if ((i + 1) < argc) {
                i++;
                if (arg_is_string(argv[i])) bf_append_input_string(bf, argv[i]);
                else bf_append_input_file(bf, argv[i]);
            }
        }
        else if (arg_is_output(argv[i])) {
            if ((i + 1) < argc) {
                i++;
                if (arg_is_string(argv[i])) bf_add_output_string(bf);
                else bf_append_output_file(bf, argv[i]);
            }
            else bf_add_output_string(bf);
        }
        else if (arg_is_string(argv[i])) {
            if ((i + 1) < argc) {
                i++;
                bf_append_input_string(bf, argv[i]);
            }
        }
        else if (arg_is_size(argv[i])) {
            if ((i + 1) < argc) {
                i++;
                errno = 0;
                char *end;
                uint64_t size = strtoull(argv[i], &end, 0);
                if ((end == argv[i]) || (errno == ERANGE)) bf_error(bf_error_size);
                bf_let_size_cells(bf, size);
            } else bf_let_size_cells(bf, 0);
        }
        else if (arg_is_optim(argv[i])) {
            if ((i + 1) < argc) {
                i++;
                errno = 0;
                char *end;
                uint64_t size = strtoull(argv[i], &end, 0);
                if ((end == argv[i]) || (errno == ERANGE)) bf_error(bf_error_optim);
                bf_let_optim(bf, size);
            }
            else bf_let_optim(bf, 0);
        }
        else if (arg_is_run(argv[i])) {
            bf_interp(bf);
        }
        else if (arg_is_onlyrun(argv[i])) {
            bf_interp_only(bf);
        }
        else bf_append_input_file(bf, argv[i]);
    }
    return count_help;
}

void print_help(const char *name_program) {
    printf("\nusage %s:\n  (...) = files\n  (-h, -help) = + 1 print help (this text)\n  (--, -f, -file, -i, -input)[1 or 2 with (-s)] = file or string appended in global code, default or null = (-s \"\")\n  (-o, -output)[1 or 2 with (-s)] = + 1 output file or + 1 echo result code, default or null = (-s \"\")\n  (-, -s, -str, -string)[1] = string appended in global code, default or null = \"\"\n  (-c, -cells, -count, -count-cells)[1] = count cells in brainfuck take memory, default = 30000, null = 0\n  (-r, -run) = run code after compilation, \n  (-ro, -or, -run-only, -only-run) = only run code, no result files compilation and no print result compilation\n  (-optim, -level, -optim-level, -level-optim)[1] = level optimization, default = 2, null = 0\n\nbrainfuck rule:\n  '+' memory[cell] = memory[cell] + 1\n  '-' memory[cell] = memory[cell] - 1\n  '>' cell = cell + 1, if cell > -count-cells then cell = cell - -count-cells\n  '<' if cell = 0 then cell = -count-cells, if not cell = cell - 1\n  '.' print ascii character from memory[cell]\n  ',' get text from user and save to memory range memory[cell]..memory[cell + size-text]\n  '[' if memory[cell] = 0 then move to character following the nearest ']' character, if not move to next character\n  ']' if memory[cell] = 0 then move to next character, if not then we go back to the nearest front character of the character '['\n\nlevel optimization:\n\n\n", name_program);
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        print_help(argv[0]);
        return 0;
    }
    struct brainfuck *bf = bf_open();
    size_t count_help = args_parse(bf, argc, argv);
    for (uint64_t i = 0; i < count_help; i++) print_help(argv[0]);
    bf_compile(bf);
    bf_flash(bf);
    bf_close(bf);
    return 0;
}
