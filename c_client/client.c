// system lib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <strings.h>


// header files
#include <client.h>

#define PORT 5005
#define SERVER_IP "127.0.0.1"
#define BASE_PATH "../dummy_files/"
#define CLIENT_CERT_FILE "./cert.pem"  // Client certificate
#define CLIENT_KEY_FILE "./key.pem"    // Client private key

// #define VERIFY_CERTS

void client(char* filename) {

    SSL_CTX* ctx;
    SSL* ssl;
    int client;
    const char* hostname = SERVER_IP;
    const char* message = "Hello, Server!";
    int bytes;
    int rtv;

    init_openssl();
    ctx = create_context();
    configure_context(ctx);

    client = create_socket(hostname, PORT);

    char response_buffer[1024 * 2] = "";


    bzero(response_buffer, 1024 * 2);
    rtv = read(client, response_buffer, 1024 * 2);
    if (rtv < 1) {
        perror("read error...");
    }
    printf("[%d]: %s", rtv, response_buffer);

    // hello
    char* hello_text = "hello..";
    rtv = write(client, hello_text, 7);
    if (rtv < 1) {
        perror("write error...");
    }

    bzero(response_buffer, 1024 * 2);
    rtv = read(client, response_buffer, 1024 * 2);
    if (rtv < 1) {
        perror("read error...");
    }
    printf("[%d]: %s", rtv, response_buffer);
    printf("lol...\n");

    // start with tls
    char* starttls_text = "starttls..";
    rtv = write(client, starttls_text, 10);
    if (rtv < 1) {
        perror("write error...");
    }

    bzero(response_buffer, 1024 * 2);
    rtv = read(client, response_buffer, 1024 * 2);
    if (rtv < 1) {
        perror("read error...");
    }
    printf("[%d]: %s", rtv, response_buffer);
    printf("lol...\n");

    // make tsl connection

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        printf("error in hand shack..\n");
        exit(1);
    }
    else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        //SSL_write(ssl, message, strlen(message));
    }

    // file name

    char* name_text = "name..";
    rtv = SSL_write(ssl, name_text, 6);
    if (rtv < 1) {
        perror("read error...");
    }

    bzero(response_buffer, 1024 * 2);
    rtv = SSL_read(ssl, response_buffer, 1024 * 2);
    if (rtv < 1) {
        perror("read error...");
    }
    printf("[%d]: %s", rtv, response_buffer);
    printf("lol...\n");

    rtv = SSL_write(ssl, filename, strlen(filename));
    if (rtv < 1) {
        perror("read error...");
    }

    char* rndotrn = "\r\n.\r\n";
    rtv = SSL_write(ssl, rndotrn, 5);
    if (rtv < 1) {
        perror("read error...");
    }

    bzero(response_buffer, 1024 * 2);
    rtv = SSL_read(ssl, response_buffer, 1024 * 2);
    if (rtv < 1) {
        perror("read error...");
    }
    printf("[%d]: %s", rtv, response_buffer);
    printf("lol...\n");

    // file send..
    char* file_text = "file..";
    rtv = SSL_write(ssl, file_text, 6);
    if (rtv < 1) {
        perror("read error...");
    }

    bzero(response_buffer, 1024 * 2);
    rtv = SSL_read(ssl, response_buffer, 1024 * 2);
    if (rtv < 1) {
        perror("read error...");
    }
    printf("[%d]: %s", rtv, response_buffer);
    printf("lol...\n");

    // send file buffer..

    char full_path[120] = BASE_PATH;
    strcat(full_path, filename);

    FILE* file = fopen(full_path, "rb");
    if (file == NULL) {
        perror("file open error...");
    }

    char* buff = (char*)malloc(sizeof(char) * 1024 * 2);
    while (1)
    {
        rtv = fread(buff, sizeof(char), 1024 * 2, file);
        if (rtv < 1) {
            break;
        }
        rtv = SSL_write(ssl, buff, rtv);
        if (rtv < 1) {
            perror("write error...");
        }
    }

    rtv = SSL_write(ssl, rndotrn, 5);
    if (rtv < 1) {
        perror("read error...");
    }

    bzero(response_buffer, 1024 * 2);
    rtv = SSL_read(ssl, response_buffer, 1024 * 2);
    if (rtv < 1) {
        perror("read error...");
    }
    printf("[%d]: %s", rtv, response_buffer);
    printf("lol...\n");

    // quit
    char* quit_text = "quit..";
    rtv = SSL_write(ssl, quit_text, 6);
    if (rtv < 1) {
        perror("read error...");
    }

    bzero(response_buffer, 1024 * 2);
    rtv = SSL_read(ssl, response_buffer, 1024 * 2);
    if (rtv < 1) {
        perror("read error...");
    }
    printf("[%d]: %s", rtv, response_buffer);
    printf("lol...\n");

    // free

    free(buff);
    SSL_shutdown(ssl);
    close(client);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    cleanup_openssl();

}

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX* create_context() {
    const SSL_METHOD* method;
    SSL_CTX* ctx;

    method = SSLv23_client_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

int create_socket(const char* hostname, int port) {
    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, hostname, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void configure_context(SSL_CTX* ctx) {
    // Load client certificate
    if (SSL_CTX_use_certificate_file(ctx, CLIENT_CERT_FILE, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        printf("Erroe block 1\n");
        exit(EXIT_FAILURE);
    }

    // Load client private key
    if (SSL_CTX_use_PrivateKey_file(ctx, CLIENT_KEY_FILE, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        printf("Erroe block 2\n");
        exit(EXIT_FAILURE);
    }

    // Verify private key
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the public certificate\n");
        exit(EXIT_FAILURE);
    }

#ifndef VERIFY_CERTS

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

#else

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_verify_depth(ctx, 4);  // Adjust as needed

#endif

}
