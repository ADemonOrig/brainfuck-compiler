#ifndef __bf__
#define __bf__ 1

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define bf_error_ok 0
#define bf_error_no 0
#define bf_error_unknow 1
#define bf_error_bad 2
#define bf_error_null 3
#define bf_error_out 4
#define bf_error_size 5
#define bf_error_alloc 6
#define bf_error_file 7
#define bf_error_loop 8
#define bf_error_end 9
#define bf_error_optim 10

void bf_error(int err) {
    if (err != bf_error_ok) printf("\x1b[38;2;255;0;0mbf_error\x1b[0m: ");
    if (err == bf_error_loop) printf("[\n");
    else if (err == bf_error_end) printf("]\n");
    else if (err == bf_error_file) printf("file\n");
    else if (err == bf_error_alloc) printf("allocate\n");
    else if (err == bf_error_size) printf("size\n");
    else if (err == bf_error_optim) printf("optimization\n");
    else if (err == bf_error_null) printf("null\n");
    else if (err == bf_error_unknow) printf("unknow\n");
    exit(err);
}

struct brainfuck {
    char *icode;
    size_t isize;
    size_t icap;
    char *ocode;
    size_t osize;
    size_t ocap;
    uint64_t ostring;
    const char **ofiles;
    size_t ofsize;
    size_t size;
    uint8_t optim;
    bool interp;
    bool interp_only;
};

struct brainfuck *bf_open() {
    struct brainfuck *bf = (struct brainfuck*)malloc(sizeof(struct brainfuck));
    if (bf == 0) bf_error(bf_error_alloc);
    bf->icode = 0;
    bf->isize = 0;
    bf->icap = 0;
    bf->ocode = 0;
    bf->osize = 0;
    bf->ocap = 0;
    bf->ostring = 0;
    bf->ofiles = 0;
    bf->ofsize = 0;
    bf->size = 30000;
    bf->optim = 2;
    bf->interp = false;
    bf->interp_only = false;
    return bf;
}

void bf_clear_input_code(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    if (bf->icode != 0) free(bf->icode);
    bf->icode = 0;
    bf->isize = 0;
    bf->icap = 0;
}

void bf_clear_output_code(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    if (bf->ocode != 0) free(bf->ocode);
    bf->ocode = 0;
    bf->osize = 0;
    bf->ocap = 0;
}

void bf_clear_output_files(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    if (bf->ofiles != 0) free(bf->ofiles);
    bf->ofiles = 0;
    bf->ofsize = 0;
}

void bf_clear_output_string(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf->ostring = 0;
}

void bf_let_size_cells(struct brainfuck *bf, size_t size) {
    if (bf == 0) bf_error(bf_error_null);
    bf->size = size;
}

void bf_let_optim(struct brainfuck *bf, uint8_t optim) {
    if (bf == 0) bf_error(bf_error_null);
    bf->optim = optim;
}

void bf_add_output_string(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf->ostring++;
}

void bf_interp(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf->interp = true;
}

void bf_interp_only(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf->interp_only = true;
}

void bf_clear(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf_clear_input_code(bf);
    bf_clear_output_code(bf);
    bf_clear_output_files(bf);
    bf_clear_output_string(bf);
}

struct brainfuck *bf_close(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf_clear(bf);
    free(bf);
    return 0;
}

void grow(void **data, size_t size, size_t *cap, size_t add, size_t sone) {
    if ((data == 0) || (cap == 0)) bf_error(bf_error_null);
    if ((size + add) >= *cap) {
        if (*cap == 0) *cap = 1;
        while ((size + add) >= *cap) {
            if ((*cap << 1) <= *cap) bf_error(bf_error_alloc);
            *cap <<= 1;
        }
        if (*data != 0) *data = (void*)realloc(*data, (*cap) * sone);
        else *data = (void*)malloc((*cap) * sone);
        if (*data == 0) bf_error(bf_error_alloc);
        memset(*data + (size * sone), 0, (*cap - size) * sone);
    }
}

void bf_input_grow(struct brainfuck *bf, size_t size) {
    if (bf == 0) bf_error(bf_error_null);
    grow((void*)&bf->icode, bf->isize, &bf->icap, size, 1);
}

void bf_output_grow(struct brainfuck *bf, size_t size) {
    if (bf == 0) bf_error(bf_error_null);
    grow((void*)&bf->ocode, bf->osize, &bf->ocap, size, 1);
}

void bf_file_grow(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    grow((void*)&bf->ofiles, bf->ofsize, &bf->ofsize, 1, sizeof(const char*));
}

void bf_append_input_string(struct brainfuck *bf, const char *str) {
    if (bf == 0) bf_error(bf_error_null);
    if (str == 0) return;
    size_t size = strlen(str);
    bf_input_grow(bf, size);
    memcpy(bf->icode + bf->isize, str, size);
    bf->isize += size;
}

void bf_append_input_file(struct brainfuck *bf, const char *path) {
    if (bf == 0) bf_error(bf_error_null);
    if (path == 0) return;
    FILE *file = fopen(path, "rb");
    if (file == 0) bf_error(bf_error_file);
    fseek(file, 0, 2);
    size_t size = ftell(file);
    fseek(file, 0, 0);
    bf_input_grow(bf, size);
    fread(bf->icode + bf->isize, size, 1, file);
    bf->isize += size;
    fclose(file);
}

void bf_append_output_string(struct brainfuck *bf, const char *str) {
    if (bf == 0) bf_error(bf_error_null);
    if (str == 0) return;
    size_t size = strlen(str);
    bf_output_grow(bf, size);
    memcpy(bf->ocode + bf->osize, str, size);
    bf->osize += size;
}

void bf_append_output_int(struct brainfuck *bf, int64_t num) {
    if (bf == 0) bf_error(bf_error_null);
    static char buf[22];
    snprintf((char*)buf, sizeof(buf), "%zu", num);
    bf_append_output_string(bf, buf);
}

void bf_append_output_file(struct brainfuck *bf, const char *path) {
    if (bf == 0) bf_error(bf_error_null);
    bf_file_grow(bf);
    ((const char**)(bf->ofiles))[bf->ofsize] = path;
    bf->ofsize++;
}

void bf_io_input_s0(uint8_t *buf, uint64_t offset, uint64_t size) {
    int chr;
    while ((chr = getchar()) != EOF && chr != 10);
}

void bf_io_input_s1(uint8_t *buf, uint64_t offset, uint64_t size) {
    int chr;
    while ((chr = getchar()) != EOF && chr != 10);
    buf[0] = 0;
}

void bf_io_input(uint8_t *buf, uint64_t offset, uint64_t size) {
    uint64_t ptr = offset % size;
    int chr;
    while ((chr = getchar()) != EOF && chr != 10) {
        buf[ptr] = (uint8_t)chr;
        ptr = (ptr + 1) % size;
    }
    buf[ptr] = 0;
}

void bf_run(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
}

void bf_flash(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    if (bf->interp_only == true) {
        bf_run(bf);
        bf_clear_input_code(bf);
        exit(0);
    }
    if ((bf->ostring == 0) && (bf->ofsize == 0)) {
        if (bf->ocode != 0) printf("%s\n", bf->ocode);
    }
    for (uint64_t i = 0; i < bf->ofsize; i++) {
        FILE *file = fopen(((const char**)bf->ofiles)[i], "wb");
        if (file == 0) bf_error(bf_error_file);
        fseek(file, 0, 0);
        if (bf->ocode != 0) fwrite(bf->ocode, bf->osize, 1, file);
        fclose(file);
    }
    bf_clear_output_files(bf);
    for (uint64_t i = 0; i < bf->ostring; i++) {
        if (bf->ocode != 0) printf("%s", bf->ocode);
    }
    if (bf->ostring > 0) printf("\n");
    bf_clear_output_string(bf);
    if (bf->interp == true) {
        bf_run(bf);
        bf_clear_input_code(bf);
    }
}

bool is_bf(char c) {
    return (
        (c == '+') ||
        (c == '-') ||
        (c == '>') ||
        (c == '<') ||
        (c == '.') ||
        (c == ',') ||
        (c == '[') ||
        (c == ']')
    );
}

bool bf_cancels(char a, char b) {
    return (
        ((a == '+') && (b == '-')) ||
        ((a == '-') && (b == '+')) ||
        ((a == '>') && (b == '<')) ||
        ((a == '<') && (b == '>'))
    );
}

void bf_optim(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    if (bf->isize <= 1) return;
    int64_t level = 0;
    char *w = bf->icode;
    for (const char *r = bf->icode; *r != 0; r++) {
        if (!is_bf(*r)) continue;
        if ((w > bf->icode) && (bf_cancels(*(w - 1), *r))) w--;
        else {
            if (*r == '[') level++;
            else if (*r == ']') {
                if (level == 0) bf_error(bf_error_end);
                level--;
            }
            *(w++) = *r;
        }
    }
    if (level != 0) bf_error(bf_error_loop);
    *w = 0;
    bf->isize = w - bf->icode;
}

void bf_compile_start(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf_append_output_string(bf, "#include <stdio.h>\n");
    if (bf->size == 0) {
        bf_append_output_string(bf, "void i(){int c;while((c=getchar())!=EOF&&c!=10);}");
    }
    if (bf->size == 1) {
        bf_append_output_string(bf, "static unsigned char t[1];void i(){int c;while((c=getchar())!=EOF&&c!=10);*t=0;}");
    } else {
        bf_append_output_string(bf, "#define s ");
        bf_append_output_int(bf, bf->size);
        bf_append_output_string(bf, "\nstatic unsigned long long p=s/2;static unsigned char t[s];void i(){unsigned long long u=s/2;int c;while((c=getchar())!=EOF&&c!=10){t[u]=(unsigned char)c;u=(u+1)%s;}t[u]=0;}");
    }
    bf_append_output_string(bf, "int main(){");
}

void bf_compile_end(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf_append_output_string(bf, "return 0;}");
}

void bf_compile_s0(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    int64_t level = 0;
    for (uint64_t i = 0; i < bf->isize; i++) {
        if (bf->icode[i] == ',') bf_append_output_string(bf, "i();");
        else if (bf->icode[i] == '.') bf_append_output_string(bf, "putchar(0);");
        else if (bf->icode[i] == '[') level++;
        else if (bf->icode[i] == ']') {
            if (level == 0) bf_error(bf_error_end);
            level--;
        }
    }
    if (level != 0) bf_error(bf_error_loop);
}

void bf_compile_0_s1(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    int64_t level = 0;
    for (uint64_t i = 0; i < bf->isize; i++) {
        if (bf->icode[i] == '+') bf_append_output_string(bf, "*t+=1;");
        else if (bf->icode[i] == '-') bf_append_output_string(bf, "*t-=1;");
        else if (bf->icode[i] == ',') bf_append_output_string(bf, "i();");
        else if (bf->icode[i] == '.') bf_append_output_string(bf, "putchar((int)*t);");
        else if (bf->icode[i] == '[') {
            level++;
            bf_append_output_string(bf, "while(*t){");
        }
        else if (bf->icode[i] == ']') {
            if (level == 0) bf_error(bf_error_end);
            level--;
            bf_append_output_string(bf, "}");
        }
    }
    if (level != 0) bf_error(bf_error_loop);
}

void bf_compile_1_s1(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf_optim(bf);
    for (uint64_t i = 0; i < bf->isize; i++) {
        if (bf->icode[i] == '+') bf_append_output_string(bf, "*t+=1;");
        else if (bf->icode[i] == '-') bf_append_output_string(bf, "*t-=1;");
        else if (bf->icode[i] == ',') bf_append_output_string(bf, "i();");
        else if (bf->icode[i] == '.') bf_append_output_string(bf, "putchar((int)*t);");
        else if (bf->icode[i] == '[') bf_append_output_string(bf, "while(*t){");
        else if (bf->icode[i] == ']') bf_append_output_string(bf, "}");
    }
}

void bf_compile_2_s1(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf_optim(bf);
    for (uint64_t i = 0; i < bf->isize; i++) {
        if ((bf->icode[i] == '+') || (bf->icode[i] == '-')) {
            int8_t num = 0;
            for (; (i < bf->isize) && ((bf->icode[i] == '+') || (bf->icode[i] == '+')); i++) {
                if (bf->icode[i] == '+') num++;
                else num--;
            }
            i--;
            if (num > 0) {
                bf_append_output_string(bf, "*t+=");
                bf_append_output_int(bf, num);
                bf_append_output_string(bf, ";");
            } else if (num < 0) {
                bf_append_output_string(bf, "*t-=");
                bf_append_output_int(bf, -num);
                bf_append_output_string(bf, ";");
            }
        }
        else if (bf->icode[i] == ',') bf_append_output_string(bf, "i();");
        else if (bf->icode[i] == '.') bf_append_output_string(bf, "putchar((int)*t);");
        else if (bf->icode[i] == '[') {
            if (((i + 2) < bf->isize) && (((bf->icode[i + 1] == '-') || (bf->icode[i + 1] == '+')) && (bf->icode[i + 2] == ']'))) {
                i += 3;
                if ((i < bf->isize) && ((bf->icode[i] == '+') || (bf->icode[i] == '-'))) {
                    uint8_t num = 0;
                    for (; (i < bf->isize) && ((bf->icode[i] == '+') || (bf->icode[i] == '+')); i++) {
                        if (bf->icode[i] == '+') num++;
                        else num--;
                    }
                    bf_append_output_string(bf, "*t=");
                    bf_append_output_int(bf, num);
                    bf_append_output_string(bf, ";");
                }
                i--;
            }
        }
        else if (bf->icode[i] == ']') bf_append_output_string(bf, "}");
    }
}

void bf_compile_0(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    int64_t level = 0;
    for (uint64_t i = 0; i < bf->isize; i++) {
        if (bf->icode[i] == '+') bf_append_output_string(bf, "t[p]+=1;");
        else if (bf->icode[i] == '-') bf_append_output_string(bf, "t[p]-=1;");
        else if (bf->icode[i] == '<') bf_append_output_string(bf, "p-=1;p%=s;");
        else if (bf->icode[i] == '>') bf_append_output_string(bf, "p+=1;p%=s;");
        else if (bf->icode[i] == ',') bf_append_output_string(bf, "i();");
        else if (bf->icode[i] == '.') bf_append_output_string(bf, "putchar((int)t[p]);");
        else if (bf->icode[i] == '[') {
            level++;
            bf_append_output_string(bf, "while(t[p]){");
        }
        else if (bf->icode[i] == ']') {
            if (level == 0) bf_error(bf_error_end);
            level--;
            bf_append_output_string(bf, "}");
        }
    }
    if (level != 0) bf_error(bf_error_loop);
}

void bf_compile_1(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf_optim(bf);
    for (uint64_t i = 0; i < bf->isize; i++) {
        if (bf->icode[i] == '+') bf_append_output_string(bf, "t[p]+=1;");
        else if (bf->icode[i] == '-') bf_append_output_string(bf, "t[p]-=1;");
        else if (bf->icode[i] == '<') bf_append_output_string(bf, "p-=1;");
        else if (bf->icode[i] == '>') bf_append_output_string(bf, "p+=1;");
        else if (bf->icode[i] == ',') bf_append_output_string(bf, "i();");
        else if (bf->icode[i] == '.') bf_append_output_string(bf, "putchar((int)t[p]);");
        else if (bf->icode[i] == '[') bf_append_output_string(bf, "while(t[p]){");
        else if (bf->icode[i] == ']') bf_append_output_string(bf, "}");
    }
}

void bf_compile_2(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf_optim(bf);
    for (uint64_t i = 0; i < bf->isize; i++) {
        if ((bf->icode[i] == '+') || (bf->icode[i] == '-')) {
            int8_t num = 0;
            for (; (i < bf->isize) && ((bf->icode[i] == '+') || (bf->icode[i] == '+')); i++) {
                if (bf->icode[i] == '+') num++;
                else num--;
            }
            i--;
            if (num > 0) {
                bf_append_output_string(bf, "t[p]+=");
                bf_append_output_int(bf, num);
                bf_append_output_string(bf, ";");
            } else if (num < 0) {
                bf_append_output_string(bf, "t[p]-=");
                bf_append_output_int(bf, -num);
                bf_append_output_string(bf, ";");
            }
        }
        else if ((bf->icode[i] == '>') || (bf->icode[i] == '<')) {
            int8_t num = 0;
            for (; (i < bf->isize) && ((bf->icode[i] == '>') || (bf->icode[i] == '<')); i++) {
                if (bf->icode[i] == '>') num++;
                else num--;
            }
            i--;
            if (num > 0) {
                bf_append_output_string(bf, "p+=");
                bf_append_output_int(bf, num);
                bf_append_output_string(bf, ";p%=s;");
            } else if (num < 0) {
                bf_append_output_string(bf, "p-=");
                bf_append_output_int(bf, -num);
                bf_append_output_string(bf, ";p%=s;");
            }
        }
        else if (bf->icode[i] == ',') bf_append_output_string(bf, "i();");
        else if (bf->icode[i] == '.') bf_append_output_string(bf, "putchar((int)t[p]);");
        else if (bf->icode[i] == '[') {
            if (((i + 2) < bf->isize) && (((bf->icode[i + 1] == '-') || (bf->icode[i + 1] == '+')) && (bf->icode[i + 2] == ']'))) {
                i += 3;
                if ((i < bf->isize) && ((bf->icode[i] == '+') || (bf->icode[i] == '-'))) {
                    uint8_t num = 0;
                    for (; (i < bf->isize) && ((bf->icode[i] == '+') || (bf->icode[i] == '+')); i++) {
                        if (bf->icode[i] == '+') num++;
                        else num--;
                    }
                    bf_append_output_string(bf, "t[p]=");
                    bf_append_output_int(bf, num);
                    bf_append_output_string(bf, ";");
                }
                i--;
            } else bf_append_output_string(bf, "while(t[p]){");
        }
        else if (bf->icode[i] == ']') bf_append_output_string(bf, "}");
    }
}

void bf_compile_3(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    bf_optim(bf);
    for (uint64_t i = 0; i < bf->isize; i++) {
        if ((bf->icode[i] == '+') || (bf->icode[i] == '-')) {
            int8_t num = 0;
            for (; (i < bf->isize) && ((bf->icode[i] == '+') || (bf->icode[i] == '+')); i++) {
                if (bf->icode[i] == '+') num++;
                else num--;
            }
            i--;
            if (num > 0) {
                bf_append_output_string(bf, "t[p]+=");
                bf_append_output_int(bf, num);
                bf_append_output_string(bf, ";");
            } else if (num < 0) {
                bf_append_output_string(bf, "t[p]-=");
                bf_append_output_int(bf, -num);
                bf_append_output_string(bf, ";");
            }
        }
        else if ((bf->icode[i] == '>') || (bf->icode[i] == '<')) {
            int8_t num = 0;
            for (; (i < bf->isize) && ((bf->icode[i] == '>') || (bf->icode[i] == '<')); i++) {
                if (bf->icode[i] == '>') num++;
                else num--;
            }
            i--;
            if (num > 0) {
                bf_append_output_string(bf, "p+=");
                bf_append_output_int(bf, num);
                bf_append_output_string(bf, ";p%=s;");
            } else if (num < 0) {
                bf_append_output_string(bf, "p-=");
                bf_append_output_int(bf, -num);
                bf_append_output_string(bf, ";p%=s;");
            }
        }
        else if (bf->icode[i] == ',') bf_append_output_string(bf, "i();");
        else if (bf->icode[i] == '.') bf_append_output_string(bf, "putchar((int)t[p]);");
        else if (bf->icode[i] == '[') {
            if (((i + 2) < bf->isize) && (((bf->icode[i + 1] == '-') || (bf->icode[i + 1] == '+')) && (bf->icode[i + 2] == ']'))) {
                i += 3;
                if ((i < bf->isize) && ((bf->icode[i] == '+') || (bf->icode[i] == '-'))) {
                    uint8_t num = 0;
                    for (; (i < bf->isize) && ((bf->icode[i] == '+') || (bf->icode[i] == '+')); i++) {
                        if (bf->icode[i] == '+') num++;
                        else num--;
                    }
                    bf_append_output_string(bf, "t[p]=");
                    bf_append_output_int(bf, num);
                    bf_append_output_string(bf, ";");
                }
                i--;
            } else bf_append_output_string(bf, "while(t[p]){");
        }
        else if (bf->icode[i] == ']') bf_append_output_string(bf, "}");
    }
}

void bf_compile(struct brainfuck *bf) {
    if (bf == 0) bf_error(bf_error_null);
    if (!bf->interp_only) {
        bf_compile_start(bf);
        if (bf->size == 0) bf_compile_s0(bf);
        else if (bf->size == 1) {
            if (bf->optim == 0) bf_compile_0_s1(bf);
            else if (bf->optim == 1) bf_compile_1_s1(bf);
            else if (bf->optim >= 2) bf_compile_2_s1(bf);
        } else {
            if (bf->optim == 0) bf_compile_0(bf);
            else if (bf->optim == 1) bf_compile_1(bf);
            else if (bf->optim == 2) bf_compile_2(bf);
            else if (bf->optim >= 3) bf_compile_3(bf);
        }
        if (bf->interp == false) bf_clear_input_code(bf);
        bf_compile_end(bf);
    }
}

#endif
