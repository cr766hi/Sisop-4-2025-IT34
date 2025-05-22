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
