#include <openssl/ssl_no_fp.h>


struct ssl_no_fp_entry
{
    void *fp;
    int offset;
};

#define SSL_NO_FP_SEEK_SET          0
#define SSL_NO_FP_SEEK_CUR          1
#define SSL_NO_FP_SEEK_END          2

#define SSL_NO_FP_MAX_ENTRIES      16


static struct ssl_no_fp_entry ssl_no_fp_entries[SSL_NO_FP_MAX_ENTRIES];

/* Success: Retrun Not -1    Fail: Retrun -1 */
static int ssl_no_fp_fseek(void *fp, int offset, int whence)
{
    int idx;

    /* Search entry exist ? */
    for (idx = 0; idx < SSL_NO_FP_MAX_ENTRIES; idx++) {
        if (ssl_no_fp_entries[idx].fp == fp)
            break;
    }

    if (whence == SSL_NO_FP_SEEK_SET) {
        if (idx < SSL_NO_FP_MAX_ENTRIES) {
            /* Entry exist; store offset */
            ssl_no_fp_entries[idx].offset = offset;
            return 0;
        }

    } else if (whence == SSL_NO_FP_SEEK_CUR) {
        if (idx < SSL_NO_FP_MAX_ENTRIES) {
            /* Entry exist; get offset */
            return ssl_no_fp_entries[idx].offset;
        }
        
        for (idx = 0; idx < SSL_NO_FP_MAX_ENTRIES; idx++) {
            /* No found (means first time using),
               find a empty entry then store data */
            if (ssl_no_fp_entries[idx].fp == 0) {
                ssl_no_fp_entries[idx].fp = fp;
                ssl_no_fp_entries[idx].offset = 0;
                return 0;
            }
        }

    } else {
        if (idx < SSL_NO_FP_MAX_ENTRIES) {
            /* Entry exist; clear all data */
            ssl_no_fp_entries[idx].fp = 0;
            ssl_no_fp_entries[idx].offset = 0;
            return 0;
        }
    }

    /* Unknown parameter of whence */
    return -1; 
}

/* Success: Retrun Not 0    Fail: Retrun 0 */
int ssl_no_fp_fwrite(char *buf, int size, void *fp)
{
    int offset;
    char *fp_p, *buf_p;
    int idx;

    fp_p = fp;
    buf_p = (char *)buf;

    /* Get current offset */
    if ((offset = ssl_no_fp_fseek(fp, 0, SSL_NO_FP_SEEK_CUR)) == -1)
        return 0;
    fp_p += offset;

    if (*buf_p == '\0') {
        *fp_p = *buf_p;
        ssl_no_fp_fseek(fp, 0, SSL_NO_FP_SEEK_SET);
        return 0;
    }

    for (idx = 0; idx < size; idx++) {
        if (*buf_p == '\0') {
            offset = 0;
            break;
        } else {
            *(fp_p++) = *(buf_p++);
            offset++;
        }
    }

    *fp_p = '\0';

    /* Set current offset */
    ssl_no_fp_fseek(fp, offset, SSL_NO_FP_SEEK_SET);

    return idx;
}

/* Success: Retrun 0    Fail: Retrun -1 */
int ssl_no_fp_fclose(void *fp)
{
    return (ssl_no_fp_fseek(fp, 0, SSL_NO_FP_SEEK_END));
}

/* Success: Retrun Not 0    Fail: Retrun 0 */
char *ssl_no_fp_fgets(char *buf, int size, void *fp)
{
    int offset;
    char *buf_p, *fp_p;
    int idx;

    buf_p = buf;
    fp_p = (char *)fp;

    /* Get current offset */
    if ((offset = ssl_no_fp_fseek(fp, 0, SSL_NO_FP_SEEK_CUR)) == -1)
        return 0;
    fp_p += offset;

    if (*fp_p == '\0') {
        /* Do nothing */
        return 0;
    }

    for (idx = 0; idx < size - 1; idx++) {
        if (*fp_p == '\0') {
            break;
        } else if (*fp_p == '\n') {
            *(buf_p++) = 0x0D; /* CR */
            *(buf_p++) = 0x0A; /* LF */
            offset++;
            break;
        } else {
            *(buf_p++) = *(fp_p++);
            offset++;
        }
    }

    *buf_p = '\0';

    /* Set current offset */
    ssl_no_fp_fseek(fp, offset, SSL_NO_FP_SEEK_SET);

    return buf;
}
