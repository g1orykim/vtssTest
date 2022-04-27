#ifndef SSL_NO_FP_H
#define SSL_NO_FP_H


/* Success: Retrun Not 0    Fail: Retrun 0 */
int ssl_no_fp_fwrite(char *buf, int size, void *fp);

/* Success: Retrun Not 0    Fail: Retrun 0 */
char *ssl_no_fp_fgets(char *buf, int size, void *fp);

/* Success: Retrun 0    Fail: Retrun -1 */
int ssl_no_fp_fclose(void *fp);


#endif /*SSL_NO_FP_H */
