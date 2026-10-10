#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR_INVALID_PARAMETER = 1,
    STATUS_ERROR_FILE_OPEN = 2,
    STATUS_ERROR_MEMORY = 3,
    STATUS_ERROR_READ_WRITE = 4
} statCode;

int samePath(const char *p1, const char *p2) {
    if (!p1 || !p2) return 0;
    if (strcmp(p1, p2) == 0) return 1;

    struct stat st1, st2;
    if (stat(p1, &st1) == 0 && stat(p2, &st2) == 0) {
        return (st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino && st1.st_ino != 0);
    }

    return 0;
}

statCode gen_newfile(const char *const input_path, char **const output_path) {
    if (!input_path || !output_path) return STATUS_ERROR_INVALID_PARAMETER;

    const char *filename = strrchr(input_path, '/');
    if (!filename) {
        filename = strrchr(input_path, '\\');
    }

    size_t path_len = 0;
    if (filename) {
        filename++;
        path_len = filename - input_path;
    } else {
        filename = input_path;
    }

    size_t out_len = path_len + 4 + strlen(filename) + 1;
    *output_path = (char *)malloc(out_len);
    if (!*output_path) return STATUS_ERROR_MEMORY;

    if (path_len > 0) {
        strncpy(*output_path, input_path, path_len);
        (*output_path)[path_len] = '\0';
    } else {
        (*output_path)[0] = '\0';
    }

    strcat(*output_path, "out_");
    strcat(*output_path, filename);

    return STATUS_OK;
}

statCode key_d(FILE *in, FILE *out) {
    if (!in || !out) return STATUS_ERROR_INVALID_PARAMETER;
    int ch;
    while ((ch = fgetc(in)) != EOF) {
        if (!isdigit(ch)) {
            if (fputc(ch, out) == EOF) return STATUS_ERROR_READ_WRITE;
        }
    }
    return STATUS_OK;
}

statCode key_i(FILE *in, FILE *out) {
    if (!in || !out) return STATUS_ERROR_INVALID_PARAMETER;
    int ch;
    unsigned long long latin_count = 0;
    int has_chars = 0;
    while ((ch = fgetc(in)) != EOF) {
        has_chars = 1;
        if (isalpha(ch) && ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))) {
            latin_count++;
        }
        if (ch == '\n') {
            if (fprintf(out, "%llu\n", latin_count) < 0) return STATUS_ERROR_READ_WRITE;
            latin_count = 0;
            has_chars = 0;
        }
    }
    if (has_chars) {
        if (fprintf(out, "%llu\n", latin_count) < 0) return STATUS_ERROR_READ_WRITE;
    }
    return STATUS_OK;
}

statCode key_s(FILE *in, FILE *out) {
    if (!in || !out) return STATUS_ERROR_INVALID_PARAMETER;
    int ch;
    unsigned long long special_count = 0;
    int has_chars = 0;
    while ((ch = fgetc(in)) != EOF) {
        has_chars = 1;
        if (ch != '\n' && ch != ' ' && !isdigit(ch) &&
            !((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))) {
            special_count++;
        }
        if (ch == '\n') {
            if (fprintf(out, "%llu\n", special_count) < 0) return STATUS_ERROR_READ_WRITE;
            special_count = 0;
            has_chars = 0;
        }
    }
    if (has_chars) {
        if (fprintf(out, "%llu\n", special_count) < 0) return STATUS_ERROR_READ_WRITE;
    }
    return STATUS_OK;
}

statCode key_a(FILE *in, FILE *out) {
    if (!in || !out) return STATUS_ERROR_INVALID_PARAMETER;
    int ch;
    while ((ch = fgetc(in)) != EOF) {
        if (isdigit(ch)) {
            if (fputc(ch, out) == EOF) return STATUS_ERROR_READ_WRITE;
        } else {
            if (fprintf(out, "%02X", (unsigned char)ch) < 0) return STATUS_ERROR_READ_WRITE;
        }
    }
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Error: Invalid number of arguments\n");
        fprintf(stderr, "Usage: %s <flag> <input_file> [output_file]\n", argv[0]);
        return STATUS_ERROR_INVALID_PARAMETER;
    }

    const char *flag = argv[1];
    const char *input_path = argv[2];
    char *output_path = NULL;
    int has_n_modifier = 0;
    char action = '\0';

    if (flag[0] != '-' && flag[0] != '/') {
        fprintf(stderr, "Error: Flag must start with '-' or '/'\n");
        return STATUS_ERROR_INVALID_PARAMETER;
    }

    if (flag[1] == 'n') {
        has_n_modifier = 1;
        if (flag[2] == '\0' || flag[3] != '\0') {
            fprintf(stderr, "Error: Invalid flag format\n");
            return STATUS_ERROR_INVALID_PARAMETER;
        }
        action = flag[2];
        if (argc != 4) {
            fprintf(stderr, "Error: The n modifier requires  explicit output file path\n");
            return STATUS_ERROR_INVALID_PARAMETER;
        }
        output_path = argv[3];
    } else {
        if (flag[1] == '\0' || flag[2] != '\0') {
            fprintf(stderr, "Error: Invalid flag format\n");
            return STATUS_ERROR_INVALID_PARAMETER;
        }
        action = flag[1];
        if (argc != 3) {
            fprintf(stderr, "Error: Too many command line arguments\n");
            return STATUS_ERROR_INVALID_PARAMETER;
        }
        statCode status = gen_newfile(input_path, &output_path);
        if (status != STATUS_OK) {
            fprintf(stderr, "Error generating dynamic output filename\n");
            return status;
        }
    }

    if (action != 'd' && action != 'i' && action != 's' && action != 'a') {
        fprintf(stderr, "Error: Unknown flag '%c' Allowed flags d i s a\n", action);
        if (!has_n_modifier) {
            free(output_path);
        }
        return STATUS_ERROR_INVALID_PARAMETER;
    }

    if (samePath(input_path, output_path)) {
        fprintf(stderr, "Error: Input and output files cannot be the same\n");
        if (!has_n_modifier) {
            free(output_path);
        }
        return STATUS_ERROR_INVALID_PARAMETER;
    }

    FILE *in = fopen(input_path, "r");
    if (!in) {
        fprintf(stderr, "Error: Could not open input file '%s'\n", input_path);
        if (!has_n_modifier) {
            free(output_path);
        }
        return STATUS_ERROR_FILE_OPEN;
    }

    FILE *out = fopen(output_path, "w");
    if (!out) {
        fprintf(stderr, "Error: Could not create/open output file '%s'\n", output_path);
        fclose(in);
        if (!has_n_modifier) {
            free(output_path);
        }
        return STATUS_ERROR_FILE_OPEN;
    }

    statCode execution_status = STATUS_OK;

    if (action == 'd') {
        execution_status = key_d(in, out);
    } else if (action == 'i') {
        execution_status = key_i(in, out);
    } else if (action == 's') {
        execution_status = key_s(in, out);
    } else if (action == 'a') {
        execution_status = key_a(in, out);
    }

    fclose(in);
    fclose(out);

    if (!has_n_modifier) {
        free(output_path);
    }

    if (execution_status == STATUS_OK) {
        printf("Processing completed successfully\n");
    } else {
        fprintf(stderr, "A runtime error occurred during processing Code: %d\n", execution_status);
    }

    return execution_status;
}




