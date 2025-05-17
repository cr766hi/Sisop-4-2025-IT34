#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <libgen.h>

void log_conversion(const char *log_path, const char *text_filename, const char *image_filename) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    FILE *log_file = fopen(log_path, "a");
    if (log_file) {
        fprintf(log_file, 
               "[%04d-%02d-%02d][%02d:%02d:%02d]: Successfully converted %s to %s.\n",
               t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
               t->tm_hour, t->tm_min, t->tm_sec,
               text_filename, image_filename);
        fclose(log_file);
    }
}

int is_valid_hex(const char *str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (!isxdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

int convert_hex_to_image(const char *input_path, const char *output_dir, const char *log_path) {
    FILE *input_file = fopen(input_path, "r");
    if (!input_file) {
        perror("Error opening input file");
        return 0;
    }

    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    if (file_size % 2 != 0) {
        fclose(input_file);
        fprintf(stderr, "Error: Hex data must have even number of characters\n");
        return 0;
    }

    char *hex_data = malloc(file_size + 1);
    if (!hex_data) {
        fclose(input_file);
        return 0;
    }
    fread(hex_data, 1, file_size, input_file);
    hex_data[file_size] = '\0';
    fclose(input_file);

    // Validate hex content
    if (!is_valid_hex(hex_data, file_size)) {
        free(hex_data);
        fprintf(stderr, "Error: Invalid hexadecimal data\n");
        return 0;
    }

    long binary_size = file_size / 2;
    unsigned char *binary_data = malloc(binary_size);
    if (!binary_data) {
        free(hex_data);
        return 0;
    }

    for (long i = 0; i < binary_size; i++) {
        if (sscanf(hex_data + i * 2, "%2hhx", &binary_data[i]) != 1) {
            free(hex_data);
            free(binary_data);
            return 0;
        }
    }

    mkdir(output_dir, 0755);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    char *base_name = basename(strdup(input_path));
    char *dot = strrchr(base_name, '.');
    if (dot) *dot = '\0';
    
    char output_filename[512];
    snprintf(output_filename, sizeof(output_filename),
             "%s/%s_image_%04d-%02d-%02d_%02d:%02d:%02d.png",
             output_dir,
             base_name,
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);

    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        free(hex_data);
        free(binary_data);
        return 0;
    }
    
    fwrite(binary_data, 1, binary_size, output_file);
    fclose(output_file);

    log_conversion(log_path, base_name, output_filename);

    free(hex_data);
    free(binary_data);
    return 1;
}

void process_directory(const char *dir_path) {
    printf("Processing directory: %s\n", dir_path);
    
    char image_dir[512];
    char log_path[512];
    snprintf(image_dir, sizeof(image_dir), "%s/image", dir_path);
    snprintf(log_path, sizeof(log_path), "%s/conversion.log", dir_path);

    FILE *log = fopen(log_path, "w");
    if (log) fclose(log);

    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".txt") == 0) {
                char full_path[512];
                snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
                printf("Converting: %s\n", entry->d_name);
                if (!convert_hex_to_image(full_path, image_dir, log_path)) {
                    fprintf(stderr, "Failed to convert: %s\n", entry->d_name);
                }
            }
        }
    }
    closedir(dir);
}

int main() {
    process_directory("anomali");
    process_directory("mnt");

    DIR *curr_dir = opendir(".");
    if (curr_dir) {
        struct dirent *entry;
        while ((entry = readdir(curr_dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                char *ext = strrchr(entry->d_name, '.');
                if (ext && strcmp(ext, ".zip") == 0) {
                    remove(entry->d_name);
                    printf("Removed zip file: %s\n", entry->d_name);
                }
            }
        }
        closedir(curr_dir);
    }

    printf("Conversion process completed.\n");
    return 0;
}
