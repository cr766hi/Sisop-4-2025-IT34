#define FUSE_USE_VERSION 31
#include <errno.h>
#include <sys/types.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static const char *relics_path = "./relics";

static void log_activity(const char *action, const char *filename);
static int baymax_getattr(const char *path, struct stat *st);
static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int baymax_open(const char *path, struct fuse_file_info *fi);
static int baymax_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi);
static int baymax_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int baymax_unlink(const char *path);

static void log_activity(const char *action, const char *filename) {
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", localtime(&now));
    
    FILE *log = fopen("activity.log", "a");
    if (log) {
        fprintf(log, "%s %s: %s\n", timestamp, action, filename);
        fclose(log);
    }
}

static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    if (strcmp(path, "/") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "Baymax.jpeg", NULL, 0);
        return 0;
    }
    return -ENOENT;
}

static int baymax_getattr(const char *path, struct stat *st) {
    memset(st, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
        return 0;
    } else if (strcmp(path, "/Baymax.jpeg") == 0) {
        st->st_mode = S_IFREG | 0644;
        st->st_nlink = 1;
        st->st_size = 14 * 1024;
        return 0;
    }

    return -ENOENT;
}

static int baymax_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") != 0) {
        return -ENOENT;
    }
    return 0;
}

static int baymax_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") != 0) {
        return -ENOENT;
    }

    struct stat st;
    if (baymax_getattr(path, &st) != 0) {
        return -ENOENT;
    }

    FILE *fragments[14];
    char fragment_path[256];

    for (int i = 0; i < 14; i++) {
        snprintf(fragment_path, sizeof(fragment_path), "%s/Baymax.jpeg.%03d", relics_path, i);
        fragments[i] = fopen(fragment_path, "rb");
        if (!fragments[i]) {
            for (int j = 0; j < i; j++) {
                fclose(fragments[j]);
            }
            return -EIO;
        }
    }

    size_t total_read = 0;
    char temp_buf[1024];

    for (int i = 0; i < 14 && size > 0; i++) {
        fseek(fragments[i], 0, SEEK_SET);
        size_t frag_size = fread(temp_buf, 1, sizeof(temp_buf), fragments[i]);

        size_t start = (offset >= i * 1024) ? offset - i * 1024 : 0;
        size_t end = (offset + size <= (i + 1) * 1024) ? frag_size : (offset + size) - i * 1024;

        if (start < frag_size) {
            size_t copy_size = end - start;
            memcpy(buf + total_read, temp_buf + start, copy_size);
            total_read += copy_size;
            size -= copy_size;
            offset += copy_size;
        }
    }

    for (int i = 0; i < 14; i++) {
        fclose(fragments[i]);
    }

if (offset == 0 && size == st.st_size) {
        log_activity("COPY", path + 1);
    }
    return total_read;
}

static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") == 0) {
        return -EPERM;
    }

    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s%s", relics_path, path);

    log_activity("CREATE", path + 1);

    return 0;
}

static int baymax_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

struct stat st;
    if (stat(relics_path, &st) != 0 || !S_ISDIR(st.st_mode)) {
        return -ENOENT;
    }

    if (strcmp(path, "/Baymax.jpeg") == 0) {
        return -EPERM;
    }

    char fragment_path[256];
    FILE *fragment_file;
    size_t total_written = 0;
    int fragment_number = 0;

    while (size > 0) {
        snprintf(fragment_path, sizeof(fragment_path), "%s/%s.%03d", relics_path, path + 1, fragment_number);
        fragment_file = fopen(fragment_path, "wb");
        if (!fragment_file) {
            return -EIO;
        }

        size_t chunk_size = (size > 1024) ? 1024 : size;
        size_t written = fwrite(buf + total_written, 1, chunk_size, fragment_file);
        fclose(fragment_file);

        if (written != chunk_size) {
            return -EIO;
        }

        total_written += written;
        size -= written;
        fragment_number++;
    }

    char log_message[512];
    snprintf(log_message, sizeof(log_message), "%s -> %s.%03d (total %d fragments)", 
             path + 1, path + 1, fragment_number - 1, fragment_number);
    log_activity("WRITE", log_message);

    return total_written;
}

static int baymax_unlink(const char *path) {
    if (strcmp(path, "/Baymax.jpeg") == 0) {
        return -EPERM;
    }

    char fragment_path[256];
    int fragment_number = 0;
    int deleted_count = 0;

    while (1) {
        snprintf(fragment_path, sizeof(fragment_path), "%s/%s.%03d", relics_path, path + 1, fragment_number);
        if (remove(fragment_path) != 0) {
            if (errno == ENOENT && fragment_number == 0) {
                return -ENOENT;
            }
            break;
        }
        deleted_count++;
        fragment_number++;
    }

    char log_message[512];
    snprintf(log_message, sizeof(log_message), "%s.%03d - %s.%03d", 
             path + 1, 0, path + 1, fragment_number - 1);
    log_activity("DELETE", log_message);

    return 0;
}

static struct fuse_operations baymax_oper = {
    .getattr = baymax_getattr,
    .readdir = baymax_readdir,
    .open = baymax_open,
    .read = baymax_read,
    .create = baymax_create,
    .write = baymax_write,
    .unlink = baymax_unlink
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &baymax_oper, NULL);
}
