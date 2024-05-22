#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BLOCK_SIZE 4

void encrypt(FILE *input_file, FILE *output_file, uint32_t key) {
    char buffer[BLOCK_SIZE];
    uint32_t input_bytes, output_bytes = 0;
    int eof_reached = 0;
    size_t bytes_read, bytes_written;
    char bytes_remaining;
    while (!eof_reached) {
        bytes_read = fread(buffer, 1, BLOCK_SIZE, input_file);

        if (bytes_read != BLOCK_SIZE) { // handle EOF
            if (bytes_read == 0) { // file length % BLOCK_SIZE = 0
                for (size_t i = 0; i < BLOCK_SIZE; i++) {
                    fputc(0, output_file);
                }
                return;
            }
            eof_reached = 1;
            bytes_remaining = BLOCK_SIZE - bytes_read;
            memcpy(&input_bytes, buffer, bytes_read);
            for (size_t i = 0; i < bytes_remaining; i++) {
                ((char *)&input_bytes)[i + bytes_read] = 0;
            }
        }
        else {
            memcpy(&input_bytes, buffer, BLOCK_SIZE);
        }
        
        // encrypt and write out
        output_bytes = input_bytes;
        output_bytes = input_bytes ^ key;
        bytes_written = fwrite((char *)&output_bytes, 1, BLOCK_SIZE, output_file);
        if (bytes_written != BLOCK_SIZE) {
            perror("Error writing to file");
            fclose(output_file);
            exit(EXIT_FAILURE);
        }
        key = output_bytes;
    }

    char final_block[BLOCK_SIZE];
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        final_block[i] = bytes_remaining;
    }
    bytes_written = fwrite(&final_block, 1, BLOCK_SIZE, output_file);
}

void decrypt(FILE *input_file, FILE *output_file, uint32_t key) {
    // char output_byte, input_byte;
    // while ((input_byte = fgetc(input_file)) != EOF) {
    //     output_byte = input_byte ^ key;
    //     if (fputc(output_byte, output_file) == EOF) {
    //         perror("Error writing to output file");
    //         fclose(input_file);
    //         fclose(output_file);
    //         exit(EXIT_FAILURE);
    //     }
    //     key = input_byte;
    // }
    char buffer[BLOCK_SIZE];
    uint32_t input_bytes, output_bytes, first_output_buffer, second_output_buffer = 0;
    int eof_reached, past_first_iter, past_second_iter = 0;
    size_t bytes_read, bytes_written;
    
    while (!eof_reached) {
        bytes_read = fread(buffer, 1, BLOCK_SIZE, input_file);

        if (bytes_read != BLOCK_SIZE) { // handle EOF
            // if (bytes_read == 1) { // just read EOF
            //     bytes_written = fwrite((char *)&first_output_buffer, 1, BLOCK_SIZE, output_file);
            //     return;
            // }
            // eof_reached = 1;
            // char bytes_remaining = BLOCK_SIZE - bytes_read;
            // memcpy(&input_bytes, buffer, bytes_read);
            // for (size_t i = 0; i < bytes_remaining; i++) {
            //     ((char *)&input_bytes)[i + bytes_read] = bytes_remaining;
            // }
            int bytes_remaining = ((char *)&second_output_buffer)[0];
            first_output_buffer ^= key;
            bytes_written = fwrite((char *)&first_output_buffer, 1, BLOCK_SIZE - bytes_remaining, output_file);
            return;
        }
        memcpy(&input_bytes, buffer, BLOCK_SIZE);
        if (past_second_iter) {
            uint32_t temp = first_output_buffer ^ key;
            key = first_output_buffer;
            bytes_written = fwrite((char *)&temp, 1, BLOCK_SIZE, output_file);
        }
        
        // decrypt and write out
        output_bytes = input_bytes;
        first_output_buffer = second_output_buffer;
        second_output_buffer = output_bytes;
        
        // if (bytes_written != 4) {
        //     perror("Error writing to file");
        //     fclose(output_file);
        //     exit(EXIT_FAILURE);
        // }
        // key = input_bytes;

        if (past_first_iter) {
            past_second_iter = 1;
        }
        past_first_iter = 1;
    }
}

int main(int argc, char *argv[]) {
    // usage check and get command line args
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <-e|-d> <key> <source_file> <destination_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *flag = argv[1];
    char *key_str = argv[2];
    const char *input_filename = argv[3];
    const char *output_filename = argv[4];

    // open files and err check
    FILE *input_file = fopen(input_filename, "rb");
    FILE *output_file = fopen(output_filename, "wb");
    if (input_file == NULL) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }
    if (output_file == NULL) {
        perror("Error opening output file");
        return EXIT_FAILURE;
    }

    // validate flag and key
    if (strcmp(flag, "-e") != 0 && strcmp(flag, "-d") != 0) {
        fprintf(stderr, "Invalid flag. Use -e for encryption or -d for decryption.\n");
        return EXIT_FAILURE;
    }
    if (strlen(key_str) != BLOCK_SIZE) {
        fprintf(stderr, "Invalid key. Key must be of length %d\n", BLOCK_SIZE);
        return EXIT_FAILURE;        
    }
    uint32_t key = 0;
    memcpy(&key, key_str, BLOCK_SIZE);

    // perform encryption/decryption
    if (!strcmp(flag, "-e")) {
        encrypt(input_file, output_file, key);
    }
    else {
        decrypt(input_file, output_file, key);
    }

    // close files
    if (ferror(input_file)) {
        perror("Error reading input file");
        fclose(input_file);
        return EXIT_FAILURE;
    }
    fclose(input_file);
    if (ferror(output_file)) {
        perror("Error reading output file");
        fclose(output_file);
        return EXIT_FAILURE;
    }
    fclose(output_file);

    return EXIT_SUCCESS;
}
