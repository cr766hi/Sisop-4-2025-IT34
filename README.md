# Sisop-4-2025-IT34
Member :
1. Christiano Ronaldo Silalahi (5027241025)
2. Naila Cahyarani Idelia (5027241063)
3. Daniswara Fausta Novanto (5027241050)

<div align=center>

# Soal Modul 4

</div>

## Soal 1

### a.

Pertama, unduh file zip menggunakan perintah berikut:

```bash
FILEID="1hi_GDdP51Kn2JJMw02WmCOxuc3qrXzh5"
FILENAME="anomali.zip"

CONFIRM=$(wget --quiet --save-cookies cookies.txt --keep-session-cookies --no-check-certificate \
"https://docs.google.com/uc?export=download&id=${FILEID}" -O- | sed -rn 's/.*confirm=([0-9A-Za-z_]+).*/\1/p')

wget --load-cookies cookies.txt "https://docs.google.com/uc?export=download&confirm=${CONFIRM}&id=${FILEID}" \
-O ${FILENAME} 
```

Kemudian, unzip file

```bash
unzip anomali.zip -d .....
```

Setelah proses unzip file, hapus file zip

```bash
rm -r ..... anomali.zip
```

### b.

Buat sebuah *file* hexed.c untuk mengubah string hexadecimal menjadi sebuah gambar ketika file text tersebut dibuka

```bash
$ hexed.c 
```
Berikut adalah kode untuk mengubah string hexadecimal menjadi sebuah gambar

```bash
#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <libgen.h>
#include <limits.h>

static const char *doc_dirpath = "/home/na-ux315/Documents";
static const char *dl_dirpath = "/home/na-ux315/Downloads";

// Function to reverse a string (excluding extension)
static char *reverse_string(char *str) {
    if (!str) return NULL;
    
    char *dot = strrchr(str, '.');
    int len = dot ? (int)(dot - str) : (int)strlen(str);
    
    for (int i = 0, j = len - 1; i < j; i++, j--) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
    return str;
}

// Function to log access to downloads folder
static void write_log(const char *path) {
    if (!path) return;

    FILE *logfile = fopen("conversion.log", "a");
    if (!logfile) return;

    time_t now;
    time(&now);
    struct tm *tm = localtime(&now);

    char month[4];
    strftime(month, sizeof(month), "%b", tm);

    char day[4];
    strftime(day, sizeof(day), "%a", tm);

    fprintf(logfile, "%.3s %.3s%3d %.2d:%.2d:%.2d %d: %s\n",
            day, month, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec,
            tm->tm_year + 1900, path);

    fclose(logfile);
}

void convert_and_log(const char *txt_path, const char *hex_data, size_t data_size);

void auto_convert_all_hex() {
    DIR *dp;
    struct dirent *de;

    const char *input_dir = "samples";
    char filepath[PATH_MAX];

    dp = opendir(input_dir);
    if (!dp) {
        perror("Failed to open samples directory");
        return;
    }

    while ((de = readdir(dp)) != NULL) {
        if (de->d_type == DT_REG && strstr(de->d_name, ".txt")) {
            snprintf(filepath, sizeof(filepath), "%s/%s", input_dir, de->d_name);

            FILE *f = fopen(filepath, "r");
            if (!f) continue;

            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            rewind(f);

            char *hex = malloc(fsize + 1);
            if (!hex) {
                fclose(f);
                continue;
            }

            size_t read_len = fread(hex, 1, fsize, f);
            hex[read_len] = '\0';
            fclose(f);

            if (read_len == 0 || strlen(hex) % 2 != 0) {
                free(hex);
                continue;
            }

            size_t len = strlen(hex) / 2;
            char *bin = malloc(len);
            if (!bin) {
                free(hex);
                continue;
            }

            int error = 0;
            for (size_t i = 0; i < len; i++) {
                if (sscanf(hex + 2 * i, "%2hhx", &bin[i]) != 1) {
                    error = 1;
                    break;
                }
            }

            if (!error) {
                char fullpath[PATH_MAX];
                snprintf(fullpath, sizeof(fullpath), "%s/%s", input_dir, de->d_name);
                convert_and_log(fullpath, bin, len);
            }

            free(hex);
            free(bin);
        }
    }

    closedir(dp);
}

void convert_and_log(const char *txt_path, const char *hex_data, size_t data_size) {
    char img_path[PATH_MAX];

    char *txt_copy = strdup(txt_path);                  
    char *base_name = basename(txt_copy);               
    char *dot = strrchr(base_name, '.');
    if (dot) *dot = '\0';

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);

    snprintf(img_path, sizeof(img_path), "samples/image/%s_image_%04d-%02d-%02d_%02d:%02d:%02d.png",
             base_name,
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec);

    mkdir("samples/image", 0755);

    FILE *img = fopen(img_path, "wb");
    if (img) {
        fwrite(hex_data, 1, data_size, img);
        fclose(img);

        FILE *log = fopen("conversion.log", "a");
        if (log) {
            fprintf(log, "[%04d-%02d-%02d][%02d:%02d:%02d]: Successfully converted hexadecimal text %s to %s.\n",
                    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                    tm->tm_hour, tm->tm_min, tm->tm_sec,
                    base_name, img_path);
            fclose(log);
        }
    }

    free(txt_copy); 
}

// Common getattr function
static int common_getattr(const char *base_path, const char *path, struct stat *stbuf) {
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", base_path, path);
    int res = lstat(fpath, stbuf);
    return (res == -1) ? -errno : 0;
}

// Documents filesystem operations (with filename reversal)
static int doc_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void)fi;
    return common_getattr(doc_dirpath, path, stbuf);
}

static int doc_readdir(const char *path, void *buf, fuse_fill_dir_t filler, 
                      off_t offset, struct fuse_file_info *fi, 
                      enum fuse_readdir_flags flags) {
    (void)offset;
    (void)fi;
    (void)flags;
    
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", doc_dirpath, path);

    DIR *dp = opendir(fpath);
    if (!dp) return -errno;

    struct dirent *de;
    int is_adfi = (strstr(path, "Adfi_") != NULL);
    
    while ((de = readdir(dp)) != NULL) {
        struct stat st = {0};
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        char *dname = strdup(de->d_name);
        if (!dname) continue;
        
        if (is_adfi && de->d_type == DT_REG) {
            reverse_string(dname);
        }

        if (filler(buf, dname, &st, 0, 0)) {
            free(dname);
            break;
        }
        free(dname);
    }
    
    closedir(dp);
    return 0;
}

static int doc_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi) {
    (void)fi;
    
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", doc_dirpath, path);
    
    int fd = open(fpath, O_RDONLY);
    if (fd == -1) return -errno;
    
    int res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
    
    close(fd);
    return res;
}

// Downloads filesystem operations (with logging)
static int dl_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void)fi;
    return common_getattr(dl_dirpath, path, stbuf);
}

static int dl_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                     off_t offset, struct fuse_file_info *fi,
                     enum fuse_readdir_flags flags) {
    (void)offset;
    (void)fi;
    (void)flags;
    
    write_log(path);

    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", dl_dirpath, path);

    DIR *dp = opendir(fpath);
    if (!dp) return -errno;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        struct stat st = {0};
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        
        if (filler(buf, de->d_name, &st, 0, 0)) break;
    }
    
    closedir(dp);
    return 0;
}

static int dl_read(const char *path, char *buf, size_t size, off_t offset,
                  struct fuse_file_info *fi) {
    (void)fi;
    
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", dl_dirpath, path);
    
    int fd = open(fpath, O_RDONLY);
    if (fd == -1) return -errno;
    
    int res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
    
    close(fd);
    return res;
}

static struct fuse_operations doc_oper = {
    .getattr = doc_getattr,
    .readdir = doc_readdir,
    .read = doc_read,
};

static struct fuse_operations dl_oper = {
    .getattr = dl_getattr,
    .readdir = dl_readdir,
    .read = dl_read,
};

int main(int argc, char *argv[]) {
    umask(0);

    // Convert
    auto_convert_all_hex();

    if (argc > 1 && strcmp(argv[1], "-downloads") == 0) {
        argv[1] = argv[0];
        return fuse_main(argc - 1, argv + 1, &dl_oper, NULL);
    }
    return fuse_main(argc, argv, &doc_oper, NULL);
}
```
Kita compile, ubah permission, dan run dengan

```bash
gcc -Wall $(pkg-config fuse3 --cflags) hexed.c -o hexed $(pkg-config fuse3 --libs)
```
```bash
chmod +x hexed
```
```bash
./hexed doc_mount
```

### c.

Untuk menampilkan hasil output dengan `tree`

![show output](assets/output.png)

### d.

Setelah sukses convert, kita lihat log di `conversion.log`

![show log_conversion](assets/log_conversion.png)

## Soal 2

Kita diminta untuk membuat sebuah sistem file virtual menggunakan FUSE (Filesystem in Userspace) yang dapat membantu seorang ilmuwan untuk menyatukan pecahan data dari robot perawat legendaris, Baymax. File-file tersebut terfragmentasi menjadi 14 bagian kecil (masing-masing 1 KB) dengan nama Baymax.jpeg.000 hingga Baymax.jpeg.013 yang tersimpan dalam direktori relics. Sistem file ini harus menampilkan file utuh Baymax.jpeg di direktori mount, serta menangani operasi seperti membaca, menulis, membuat, dan menghapus file dengan memecah atau menggabungkan file secara otomatis. Selain itu, semua aktivitas pengguna harus dicatat dalam file log activity.log.

### Perintah pada terminal

Pertama, unduh file zip menggunakan perintah berikut:
```bash
curl -L -o Baymax.zip "https://drive.google.com/uc?export=download&id=1MHVhFT57Wa9Zcx69Bv9j5ImHc9rXMH1c"
```

Kemudian, unzip file
```bash
unzip Baymax.zip -d relics
```

Buat direktori mount_dir dan relics jika belum ada:
```bash
mkdir -p mount_dir relics
```

Untuk kompilasi kode gunakan perintah:
```bash
gcc -Wall baymax.c -o baymax `pkg-config --cflags --libs fuse`
```

Untuk menjalankan sistem FUSE:
```bash
./baymax -f mount_dir
```

Untuk menghentikan FUSE:
```bash
fusermount -u mount_dir
```


### Berikut adalah kode untuk operasi dasar dalam FUSE:

Mengembalikan atribut file atau direktori:
```bash
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
```

Menampilkan isi direktori (hanya menampilkan Baymax.jpeg di root direktori):
```bash
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
```

Membuka file Baymax.jpeg:
```bash
static int baymax_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") != 0) {
        return -ENOENT;
    }
    return 0;
}
```

Membaca dan menggabungkan data dari 14 pecahan file saat Baymax.jpeg dibaca:
```bash
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
```

Menangani pembuatan file baru dan memecahnya menjadi pecahan 1 KB:
```bash
static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") == 0) {
        return -EPERM;
    }

    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s%s", relics_path, path);

    log_activity("CREATE", path + 1);

    return 0;
}
```

Menulis data ke file baru dan memecahnya menjadi pecahan 1 KB:
```bash
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
```

Menghapus file dan semua pecahannya:
```bash
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
```

Kode untuk bagian activity log:
```bash
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
```

Karena saya belum berhasil untuk connect ke direktori mount_dir nya, jadi saya masih belum mendapatkan output dari hasil uji coba dengan operasi dasar di atas.

## Soal 3
Nafis dan Kimcun merupakan dua mahasiswa anomali😱 yang paling tidak tahu sopan santun dan sangat berbahaya di antara angkatan 24. Maka dari itu, Pujo sebagai komting yang baik hati dan penyayang😍, memutuskan untuk membuat sebuah sistem pendeteksi kenakalan bernama Anti Napis Kimcun (AntiNK) untuk melindungi file-file penting milik angkatan 24. Pujo pun kemudian bertanya kepada Asisten bagaimana cara membuat sistem yang benar, para asisten pun merespon

### a. 
Pujo harus membuat sistem AntiNK menggunakan Docker yang menjalankan FUSE dalam container terisolasi. Sistem ini menggunakan docker-compose untuk mengelola container antink-server (FUSE Func.) dan antink-logger (Monitoring Real-Time Log). Asisten juga memberitahu bahwa docker-compose juga memiliki beberapa komponen lain yaitu
it24_host (Bind Mount -> Store Original File)
antink_mount (Mount Point)
antink-logs (Bind Mount -> Store Log)

Pertama kita buat dahulu file docker-compose.yml dan berisikan code dibawah ini :

```bash
version: '3.8'

services:
  antink-server:
    build: .
    container_name: soal_3-antink-server
    privileged: true
    devices:
      - /dev/fuse:/dev/fuse
    cap_add:
      - SYS_ADMIN
    security_opt:
      - apparmor:unconfined
    volumes:
      - ./it24_host:/it24_host:ro
      - ./antink_mount:/antink_mount
      - ./antink-logs:/var/log
    restart: unless-stopped
    command: bash -c "/app/antink /antink_mount & tail -f /dev/null"

  antink-logger:
    image: ubuntu:latest
    container_name: soal_3-antink-logger
    depends_on:
      - antink-server
    volumes:
      - ./antink-logs:/var/log
    command: bash -c "while [ ! -f /var/log/it24.log ]; do sleep 1; done; tail -f /var/log/it24.log"
```

Sebelum jalankan sistem pastikan :
```bash
mkdir -p it24_host antink_mount antink-logs
chmod -R 777 antink_mount antink-logs 
```

Mekanisme Isolasi :

File Asli: Disimpan di ./it24_host (host) → /it24_host (container, read-only)
File Termodifikasi: Tampil di ./antink_mount (host) → /antink_mount (container)
Log: Disimpan di ./antink-logs (host) → /var/log (container)

Coba di jalankan :
```bash
docker-compose up --build
```
![image](https://github.com/user-attachments/assets/a87113cc-5535-461e-8461-aed591809547)

lalu kita bisa menjalankan perintah lain pada terminal lain.

### b. 
Sistem harus mendeteksi file dengan kata kunci "nafis" atau "kimcun" dan membalikkan nama file tersebut saat ditampilkan. Saat file berbahaya (kimcun atau nafis) terdeteksi, sistem akan mencatat peringatan ke dalam log.
Ex: "docker exec [container-name] ls /antink_mount" 
Output: 
test.txt  vsc.sifan  txt.nucmik

Digunakan untuk: Membalik nama file berbahaya.
```BASH
char* strrev(char* str) {
    if (!str || !*str) return str;
    
    int i = strlen(str) - 1, j = 0;
    char ch;
    while (i > j) {
        ch = str[i];
        str[i] = str[j];
        str[j] = ch;
        i--; j++;
    }
    return str;
}
```

Fungsi: Membalik string in-place.
Contoh: "nafis" → "sifan"

![image](https://github.com/user-attachments/assets/52eaab7b-fcd0-4ffc-ac2a-8a549af73cf4)

nafis = sifan, 
kimchun = nucmik

dengan lognya :
![image](https://github.com/user-attachments/assets/2f1001f9-1339-49e5-94b8-143a88b2338f)

### c.
Dikarenakan dua anomali tersebut terkenal dengan kelicikannya, Pujo mempunyai ide bahwa isi dari file teks normal akan di enkripsi menggunakan ROT13 saat dibaca, sedangkan file teks berbahaya tidak di enkripsi. 
Ex: "docker exec [container-name] cat /antink_mount/test.txt" 
Output: 
enkripsi teks asli

```bash
void rot13(char *str) {
    for (; *str; str++) {
        if (*str >= 'a' && *str <= 'z') {
            *str = ((*str - 'a' + 13) % 26) + 'a';
        } else if (*str >= 'A' && *str <= 'Z') {
            *str = ((*str - 'A' + 13) % 26) + 'A';
        }
    }
}
```

ini berfungsi untuk mengenkripsi teks dengan chiper ROT13

![image](https://github.com/user-attachments/assets/49c6e412-00d9-46ec-976e-0051b7d2d856)

### d. 
Semua aktivitas dicatat dengan ke dalam log file /var/log/it24.log yang dimonitor secara real-time oleh container logger.

buka termnial 1 dan log akan otomatis tercatat
![image](https://github.com/user-attachments/assets/495f7fdd-0763-4968-9a39-384299245d21)

### e. 
Semua perubahan file hanya terjadi di dalam container server jadi tidak akan berpengaruh di dalam direktori host. 
![image](https://github.com/user-attachments/assets/ac8b5485-2317-4284-8622-86d29ba531df)
vs
![image](https://github.com/user-attachments/assets/4d631997-9fa5-42df-906f-370141a2ddd1)










