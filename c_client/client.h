#ifndef CLINT_HEADER
#define CLINT_HEADER

#include <openssl/ssl.h>

void client(char*);
void init_openssl();
void cleanup_openssl();
SSL_CTX* create_context();
int create_socket(const char*, int);

#endif