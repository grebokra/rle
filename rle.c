#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>

#define RLE_MAG0 0b01110010
#define RLE_MAG1 0b01101100
#define RLE_MAG2 0b01100101
#define DOT_EXT ".rle" 

void usage() {
    printf("usage: rle -cx <filename> [-o <output file name>]\n");
    printf("\"rle -h\" to see this message\n");
    printf("   -c to compress (requires filename as argument)\n");
    printf("   -x to extract compressed file (requires filename as argument)\n");
    printf("   -o to specify an output file (optional)\n");
    exit(EXIT_SUCCESS);
}

void rle_compress(char *read_filename, char *write_filename) {
    FILE *read_file = fopen(read_filename, "r");
    if (read_file == NULL) {
        fprintf(stderr,"ERROR: File %s was not open.\n", read_filename);
        exit(EXIT_FAILURE);
    }

    FILE *write_file = fopen(write_filename, "w");
    if (write_file == NULL) {
        fprintf(stderr,"ERROR: File %s was not open.\n", write_filename);
        exit(EXIT_FAILURE);
    }
    
    fputc(RLE_MAG0, write_file);
    fputc(RLE_MAG1, write_file);
    fputc(RLE_MAG2, write_file);

    uint8_t continuous_bytes_count = 0;
    uint8_t previous_byte = fgetc(read_file);
    uint8_t current_byte = fgetc(read_file);
    while (1) {
        if ((previous_byte == current_byte) && (continuous_bytes_count != 255)) {
            continuous_bytes_count+= 1;
        }
        else {
            if (continuous_bytes_count == 0) {
                fputc(previous_byte, write_file);
            }
            else {
                fputc(previous_byte, write_file);
                fputc(previous_byte, write_file);
                fputc(continuous_bytes_count, write_file);
                continuous_bytes_count = 0;
            }
        }

        previous_byte = current_byte;
        if (feof(read_file)) {
            break;
        }
        current_byte = fgetc(read_file);
    }

    fclose(write_file);
    fclose(read_file);
}

void rle_uncompress(char *read_filename, char *write_filename) {
    FILE *read_file = fopen(read_filename, "r");
    if (read_file == NULL) {
        fprintf(stderr,"\nERROR: File %s was not open.\n", read_filename);
        exit(EXIT_FAILURE);
    }
    
    FILE *write_file = fopen(write_filename, "w");
    if (write_file == NULL) {
        fprintf(stderr,"ERROR: File %s was not open.\n", write_filename);
        exit(EXIT_FAILURE);
    }

    for (int i= 0; i < 3; i++) {
        switch (fgetc(read_file)) {
        case RLE_MAG0:
            break;
        case RLE_MAG1:
            break;
        case RLE_MAG2:
            break;
        default:
            fprintf(stderr, "\nError: unknown filetype\n");
            exit(EXIT_FAILURE);
        }
    }
    
    uint8_t current_byte = fgetc(read_file);
    uint8_t next_byte = fgetc(read_file);
    while(1) {
        if(current_byte == next_byte) {
            uint8_t continuous_bytes_count = fgetc(read_file);
            for (int i = 0; i <= continuous_bytes_count; i++) {
                fputc(current_byte, write_file);
            }
            current_byte = fgetc(read_file);
            if (feof(read_file)) {
                break;
            }
            next_byte = fgetc(read_file);
            continue;
        }
        else {
            fputc(current_byte, write_file);
        }
        if (feof(read_file)) {
            break;
        }
        current_byte = next_byte;
        next_byte = fgetc(read_file);
    }

    fclose(write_file);
    fclose(read_file);
}

int main(int argc, char **argv) {
    int opt;
    bool compress = false;
    bool uncompress = false;
    char *read_filename = NULL;
    char *write_filename = NULL;
    while ((opt= getopt(argc, argv, "c:x:o:h")) != -1) {
		switch (opt) {
		case 'h':
			usage();
			break;
		case 'o':
            write_filename = malloc(strlen(optarg)+1);
            if (!write_filename) {
                fprintf(stderr, "Error: malloc failed\n");
            }
            strcpy(write_filename, optarg);
            break;
        case 'c':
            compress = true;
            read_filename = malloc(strlen(optarg)+1);
            if (!read_filename) {
                fprintf(stderr, "Error: malloc failed\n");
            }
            strcpy(read_filename, optarg);
            break;
        case 'x':
            uncompress = true;
            read_filename = malloc(strlen(optarg)+1);
            if (!read_filename) {
                fprintf(stderr, "Error: malloc failed\n");
            }
            strcpy(read_filename, optarg);
            break;
		default:
			usage();
		}
	}

    if (uncompress && compress) {
        fprintf(stderr, "Error: Can't compress and uncompress file at the same time.\n");
        usage();
        exit(EXIT_FAILURE);
    }
    else if (!(uncompress || compress)) {
        usage();
        exit(EXIT_FAILURE);
    }

    if (!read_filename) {
        fprintf(stderr, "Error: You should pass a filename after -c/-x\n");
        usage();
        exit(EXIT_FAILURE);
    }

    if (compress) {
        if (!write_filename) {
            int length_of_extension = strlen(DOT_EXT);
            int length_of_filename = strlen(read_filename);
            write_filename = malloc(length_of_filename + length_of_extension);
            strcpy(write_filename, read_filename);
            strcat(write_filename, DOT_EXT);
        }
        rle_compress(read_filename, write_filename);
    }
    else if (uncompress) {
        if (!write_filename) {
            char *extension_start = strstr((const char*)read_filename, DOT_EXT);
            ptrdiff_t write_filename_length;
            if (extension_start) {
                write_filename_length = extension_start - read_filename;
            }
            else {
                write_filename_length = strlen(read_filename) + strlen(DOT_EXT);
            }
            
            char *write_filename = malloc(write_filename_length);
            if (!write_filename) {
                fprintf(stderr, "\nError: malloc failed!\n");
                exit(EXIT_FAILURE);
            }

            strncpy(write_filename, read_filename, write_filename_length);
        }
        rle_uncompress(read_filename, write_filename);

    }

    free(read_filename);
    free(write_filename);
    
    exit(EXIT_SUCCESS);
}
