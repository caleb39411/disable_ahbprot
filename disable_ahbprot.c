#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define PATTERN_SIZE 8
#define PATTERN "\xD0\x0B\x23\x08\x43\x13\x60\x0B"
#define PATCH "\x46\xC0\x23\x08\x43\x13\x60\x0B"

void print_help() {
    printf("Usage: disable_ahbprot [options] <file>\n");
    printf("Options:\n");
    printf("  -v, --verbose    Enable verbose output\n");
    printf("  -h, --help       Display this help and exit\n");
}

bool patch_file(const char *filename, bool verbose) {
    FILE *file = fopen(filename, "r+b");
    if (!file) {
        if (verbose) fprintf(stderr, "Error opening file: %s\n", filename);
        return false;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    unsigned char *buffer = malloc(size);
    if (!buffer) {
        if (verbose) fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return false;
    }

    fread(buffer, size, 1, file);
    bool found = false;
    for (long i = 0; i <= size - PATTERN_SIZE; i++) {
        if (memcmp(buffer + i, PATTERN, PATTERN_SIZE) == 0) {
            memcpy(buffer + i, PATCH, PATTERN_SIZE);
            if (verbose) printf("Patch applied at offset: %ld\n", i);
            found = true;
        }
    }

    rewind(file);
    fwrite(buffer, size, 1, file);
    free(buffer);
    fclose(file);
    return found;
}

int main(int argc, char *argv[]) {
    bool verbose = false;
    char *filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        } else {
            filename = argv[i];
        }
    }

    if (!filename) {
        fprintf(stderr, "No file specified.\n");
        print_help();
        return 1;
    }

    if (verbose) {
        printf("Patching file: %s\n", filename);
    }
    if (!patch_file(filename, verbose)) {
        if (verbose) {
            fprintf(stderr, "Failed to patch file or file does not exist: %s\n", filename);
        }
        return 1;
    }

    return 0;
}
