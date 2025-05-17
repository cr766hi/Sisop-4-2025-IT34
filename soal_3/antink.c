#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static const char *it24_host_path = "/it24_host";
static const char *log_path = "/var/log/it24.log";

char* strrev(char* str) {
    if (!str || !*str) return str;
    
    int i = strlen(str) - 1, j = 0;
    char ch;
    while (i > j) {
        ch = str[i];
        str[i] = str[j];
        str[j] = ch;
        i--;
        j++;
    }
    return str;
}

// Fungsi ROT13
void rot13(char *str) {
    for (; *str; str++) {
        if (*str >= 'a' && *str <= 'z') {
            *str = ((*str - 'a' + 13) % 26) + 'a';
        } else if (*str >= 'A' && *str <= 'Z') {
            *str = ((*str - 'A' + 13) % 26) + 'A';
        }
    }
}

int is_dangerous(const char *filename) {
    return strstr(filename, "nafis") || strstr(filename, "kimcun");
}

void write_log(const char *message, const char *filename) {
    time_t now;
    time(&now);
    char *time_str = ctime(&now);
    time_str[strlen(time_str)-1] = '\0'; 
    
    FILE *log_file = fopen(log_path, "a");
    if (log_file) {
        fprintf(log_file, "[%s][%s] %s\n", time_str, message, filename);
        fclose(log_file);
    }
}

static int antink_getattr(const char *path, struct stat *stbuf) {
    char full_path[1024];
    sprintf(full_path, "%s%s", it24_host_path, path);
    
    int res = lstat(full_path, stbuf);
    if (res == -1) return -errno;
    return 0;
}

static int antink_readdir(const char *path, void *buf, fuse_fill_dir_t filler, 
                         off_t offset, struct fuse_file_info *fi) {
    char full_path[1024];
    sprintf(full_path, "%s%s", it24_host_path, path);
    
    DIR *dp = opendir(full_path);
    if (!dp) return -errno;
    
    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        
        char display_name[256];
        strcpy(display_name, de->d_name);
        
        if (is_dangerous(display_name)) {
            strrev(display_name);
            write_log("REVERSK", de->d_name);
        }
        
        if (filler(buf, display_name, &st, 0)) break;
    }
    closedir(dp);
    return 0;
}

static int antink_open(const char *path, struct fuse_file_info *fi) {
    char full_path[1024];
    sprintf(full_path, "%s%s", it24_host_path, path);
    
    int res = open(full_path, fi->flags);
    if (res == -1) return -errno;
    
    close(res);
    return 0;
}

static int antink_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    char full_path[1024];
    sprintf(full_path, "%s%s", it24_host_path, path);
    
    int fd = open(full_path, O_RDONLY);
    if (fd == -1) return -errno;
    
    int res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
    
    const char *ext = strrchr(path, '.');
    if (ext && strcmp(ext, ".txt") == 0) {
        if (!is_dangerous(path)) {
            rot13(buf);
        } else {
            write_log("ENCAPY1", path);
        }
    }
    
    close(fd);
    return res;
}

static struct fuse_operations antink_oper = {
    .getattr = antink_getattr,
    .readdir = antink_readdir,
    .open = antink_open,
    .read = antink_read,
};

int main(int argc, char *argv[]) {
    FILE *log_file = fopen(log_path, "a");
    if (log_file) {
        fclose(log_file);
    } else {
        perror("Failed to initialize log file");
    }
    
    umask(0);
    return fuse_main(argc, argv, &antink_oper, NULL);
}
