/*
 * Copyright (c) 2024 Parthka <hello@parthka.dev>
 * Licensed under the unlicense
 * made with 💻 🧠 🤖 by Parthka
 */

#define DEV_MOD

 // sys libs
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// header file
#include <main.h>

#define CERT_FILE "./cert.pem"
#define KEY_FILE "./key.pem"


int main() {
    server_t server;
    server_set_port(&server, 5005);
    server_make(&server);
    server_listen(&server);
    server_start(&server);
}

void server_make(server_t* server) {

    init_openssl();
    SSL_CTX* ctx = create_context();
    configure_context(ctx);

    server->ssl_ctx = ctx;


    server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket_fd < 0) {
        PANIC("socket create error");
    }

    struct sockaddr_in server_addr;
    int socket_opts = 1;

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server->port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, &socket_opts, sizeof(socket_opts));

    int bind_rtv = bind(server->socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bind_rtv < 0) {
        PANIC("bind error");
    }

    server->event_loop_fd = epoll_create1(0);
    if (server->event_loop_fd < 0) {
        PANIC("event poll make error");
    }

    event_server* ev_server = (event_server*)malloc(sizeof(event_server));
    ev_server->fd = server->socket_fd;
    ev_server->server = server;
    ev_server->handler = accept_new_connection;

    ev_poll_set_read_fd(server, server->socket_fd, (void*)ev_server);
}

void server_listen(server_t* server) {
    int rtv = listen(server->socket_fd, 5);
    if (rtv < 0) {
        PANIC("listen error");
    }
}

void server_start(server_t* server) {
    ev_poll_start(server);
}

void ev_poll_start(server_t* server) {

    struct epoll_event* events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * 1);

    while (1) {
        int n = epoll_wait(server->event_loop_fd, events, 1, -1);

        for (int i = 0; i < n; i++) {
            // DEBUG("new event\n");
            event_server* ev_server = (event_server*)events[i].data.ptr;
            ev_server->handler(events[i].data.ptr);
        }
    }

}

void accept_new_connection(void* ptr) {
    event_server* s = (event_server*)ptr;

    struct sockaddr_in client_address;
    socklen_t client_address_len = 0;

    int client_fd = accept(s->fd, (struct sockaddr*)&client_address, &client_address_len);
    if (client_fd < 0) {
        PANIC("accept error");
    }

    DEBUG("Accept new Connection\n");

    event_client* client = client_init();
    client->fd = client_fd;
    client->server = s->server;
    client->handler = send_hello;

    printf("FD: %d\n", client_fd);

    ev_poll_set_write_fd(s->server, client_fd, (void*)client);
    DEBUG("Add new connection in event poll\n");
}

void send_hello(void* ptr) {
    event_client* client = (event_client*)ptr;
    char* hello = "Hello, Client!\r\n";
    client->send_func(client->fd, hello, strlen(hello), client);

    client->handler = read_commond_and_write;
    ev_poll_to_read(client->server, client->fd, ptr);
}

void read_and_write(void* ptr) {
    event_client* client = (event_client*)ptr;

    bzero(client->buf, 1024);
    client->recv_func(client->fd, client->buf, 1024, client);

    printf("message from client: %s\n", client->buf);
}

void read_commond_and_write(void* ptr) {
    event_client* client = (event_client*)ptr;
    if (recv_chunk(client) == 0) {
        return;
    }

    char commond[100] = "";
    switch (get_commond_from_buf(client, commond)) {
    case COMMOND_ERROR:
        // printf("commond: %s\n", commond);
        PANIC("error on commond read");
        break;
    case COMMOND_MORE:
        // send(client->fd, "MORE BUF\n", 9, 0);
        return;
        break;
    case COMMOND_OK:
        client->buf_len = 0;
        client->buf_cur = 0;
        commond_handle(client, commond);
        break;
    }
}

void commond_handle(event_client* client, char* commond) {
    if (strcmp(commond, "HELLO") == 0) {
        int send_rtv = client->send_func(client->fd, "OK HELLO\r\n", 10, client);
        if (send_rtv < 1) {
            PANIC("send error");
        }
    }
    else if (strcmp(commond, "INC") == 0) {
        client->ind += 1;
        int send_rtv = client->send_func(client->fd, "OK INC+\r\n", 10, client);
        if (send_rtv < 1) {
            PANIC("send error");
        }
    }
    else if (strcmp(commond, "DEC") == 0) {
        client->ind -= 1;
        int send_rtv = client->send_func(client->fd, "OK DEC-\r\n", 10, client);
        if (send_rtv < 1) {
            PANIC("send error");
        }
    }
    else if (strcmp(commond, "TEXT") == 0) {
        client->handler = text_handle;
        int send_rtv = client->send_func(client->fd, "OK GO AHEAD\r\n", 13, client);
    }
    else if (strcmp(commond, "FILE") == 0) {
        if (client->f == NULL) {
            int send_rtv = client->send_func(client->fd, "ERROR FILE NOT FOUND\r\n", 22, client);
            if (send_rtv < 1) {
                PANIC("send error");
            }
            return;
        }
        client->handler = file_handle;
        int send_rtv = client->send_func(client->fd, "OK GO AHEAD\r\n", 13, client);
    }
    else if (strcmp(commond, "NAME") == 0) {
        client->text->t_cur = 0;
        client->handler = filename_handle;
        int send_rtv = client->send_func(client->fd, "OK GO AHEAD\r\n", 13, client);
    }
    else if (strcmp(commond, "STARTTLS") == 0) {
        DEBUG("go1..\n");
        SSL* ssl = SSL_new(client->server->ssl_ctx);
        SSL_set_fd(ssl, client->fd);

        int send_rtv = client->send_func(client->fd, "OK HANDSHACK\r\n", 14, client);

        if (SSL_accept(ssl) <= 0) {
            client_close(client);
            return;
        }
        else {
            client->ssl = ssl;
            client->recv_func = recv_func_ssl;
            client->send_func = send_func_ssl;
        }
    }
    else if (strcmp(commond, "STATES") == 0) {
        char buff[120] = "";
        int sprintf_rtv = sprintf(buff, "Secure: %s\nIND: %d\nTEXT: %s\r\n", 0 ? "YEP" : "NOP", client->ind, client->text->t_cur == 0 ? "NO ANY TEXT" : " ");

        int send_rtv = client->send_func(client->fd, buff, sprintf_rtv, client);
        if (send_rtv < 1) {
            PANIC("send error");
        }

        if (client->text->t_cur > 0) {
            // DEBUG("SET FD TO WRITR...\n");
            client->handler = send_text;
            ev_poll_to_write(client->server, client->fd, (void*)client);
        }

    }
    else if (strcmp(commond, "QUIT") == 0) {
        int send_rtv = client->send_func(client->fd, "OK BYYY\r\n", 9, client);
        if (send_rtv < 1) {
            PANIC("send error");
        }
        client_close(client);
    }
    else {
        int send_rtv = client->send_func(client->fd, "ERROR INVALID COMMOND\r\n", 23, client);
        if (send_rtv < 1) {
            PANIC("send error");
        }
    }
}

void text_handle(void* ptr) {
    event_client* client = (event_client*)ptr;
    int start = recv_text_chunk(client);
    DEBUG("NOP\n");
    if (parse_text(client, start) == PARSE_OK) {
        client->handler = read_commond_and_write;
        int send_rtv = client->send_func(client->fd, "OK TEXT WRITE\r\n", 15, client);
        if (send_rtv < 1) {
            PANIC("send error");
            DEBUG("NOP3\n");
        }
    }
}

void filename_handle(void* ptr) {
    event_client* client = (event_client*)ptr;
    int start = recv_text_chunk(client);
    DEBUG("NOP\n");
    if (parse_text(client, start) == PARSE_OK) {

        char* filename = (char*)malloc(sizeof(char) * 200);
        memcpy(filename, client->text->t, client->text->t_cur - 2);
        filename[client->text->t_cur - 2] = '\0';

        char base_file_path[300] = BASEFILE_PATH;
        strcat(base_file_path, filename);
        free(filename);

        client->text->t_cur = 0;
        printf("full path: %s\n", base_file_path);

        remove(base_file_path);

        FILE* f = fopen(base_file_path, "a");

        if (f == NULL) {
            perror("file open error");
            exit(0);
        }

        client->f = f;

        client->handler = read_commond_and_write;
        int send_rtv = client->send_func(client->fd, "OK TEXT WRITE\r\n", 15, client);
        if (send_rtv < 1) {
            PANIC("send error");
            DEBUG("NOP3\n");
        }
    }
}

void file_handle(void* ptr) {
    event_client* client = (event_client*)ptr;
    int start = recv_file_chunk(client);
    // for (int i = start; i < client->file->t_cur; i++) {
    //     printf("%d %c\n", client->file->t[i], client->file->t[i]);
    // }
    DEBUG("NOP\n");
    if (parse_file(client, start) == PARSE_OK) {
        client->handler = read_commond_and_write;
        fwrite(client->file->t, sizeof(char), client->file->t_cur, client->f);
        fclose(client->f);
        client->f = NULL;
        int send_rtv = client->send_func(client->fd, "OK FILE WRITE\r\n", 15, client);
        if (send_rtv < 1) {
            PANIC("send error");
            DEBUG("NOP3\n");
        }
    }
    DEBUG("NOP2\n");
}

int parse_file(event_client* client, int start) {
    crlf_t* crlf = &client->file->crlf;

    for (int i = start; i < client->file->t_cur; i++) {
        if (client->file->t[i] == '\r') {
            if (crlf->d == 1) {
                crlf->s_r = 1;
            }
            else {
                crlf->f_r = 1;
            }
            // DEBUG("read <CR>\n");
            // printf("%d %d %d %d %d\n", crlf->f_r, crlf->f_n, crlf->d, crlf->s_r, crlf->s_n);
        }
        else if (client->file->t[i] == '\n') {
            if (crlf->s_r == 1) {
                client->file->t_cur = i - 4;
                // printf("%d\n", i);
                DEBUG("READ <CRLF>.<CRLF> IN TEXT...\n");
                return PARSE_OK;
            }
            else {
                if (crlf->f_r == 1) {
                    crlf->f_n = 1;
                }
                else {
                    crlf->f_n = 0;
                    crlf->f_r = 0;
                }
            }
            // DEBUG("read <LF>\n");
            // printf("%d %d %d %d %d\n", crlf->f_r, crlf->f_n, crlf->d, crlf->s_r, crlf->s_n);
        }
        else if (client->file->t[i] == '.') {
            if (crlf->f_n == 1) {
                crlf->d = 1;
            }
            // DEBUG("read <.>\n");
            // printf("%d %d %d %d %d\n", crlf->f_r, crlf->f_n, crlf->d, crlf->s_r, crlf->s_n);
        }
        else {
            crlf->f_r = 0; crlf->f_n = 0; crlf->d = 0; crlf->s_r = 0; crlf->s_n = 0;
            // printf("%d %d %d %d %d\n", crlf->f_r, crlf->f_n, crlf->d, crlf->s_r, crlf->s_n);
        }

    }

    return PARSE_MORE;
}

int parse_text(event_client* client, int start) {
    crlf_t* crlf = &client->text->crlf;

    for (int i = start; i < client->text->t_cur; i++) {
        if (client->text->t[i] == '\r') {
            if (crlf->d == 1) {
                crlf->s_r = 1;
            }
            else {
                crlf->f_r = 1;
            }
            // DEBUG("read <CR>\n");
            // printf("%d %d %d %d %d\n", crlf->f_r, crlf->f_n, crlf->d, crlf->s_r, crlf->s_n);
        }
        else if (client->text->t[i] == '\n') {
            if (crlf->s_r == 1) {
                client->text->t_cur = i - 2;
                // printf("%d\n", i);
                DEBUG("READ <CRLF>.<CRLF> IN TEXT...\n");
                return PARSE_OK;
            }
            else {
                if (crlf->f_r == 1) {
                    crlf->f_n = 1;
                }
                else {
                    crlf->f_n = 0;
                    crlf->f_r = 0;
                }
            }
            // DEBUG("read <LF>\n");
            // printf("%d %d %d %d %d\n", crlf->f_r, crlf->f_n, crlf->d, crlf->s_r, crlf->s_n);
        }
        else if (client->text->t[i] == '.') {
            if (crlf->f_n == 1) {
                crlf->d = 1;
            }
            // DEBUG("read <.>\n");
            // printf("%d %d %d %d %d\n", crlf->f_r, crlf->f_n, crlf->d, crlf->s_r, crlf->s_n);
        }
        else {
            crlf->f_r = 0; crlf->f_n = 0; crlf->d = 0; crlf->s_r = 0; crlf->s_n = 0;
            // printf("%d %d %d %d %d\n", crlf->f_r, crlf->f_n, crlf->d, crlf->s_r, crlf->s_n);
        }

    }

    return PARSE_MORE;
}

void send_text(void* ptr) {
    event_client* client = (event_client*)ptr;
    int rem_buf = client->text->t_cur - client->text->t_w_cur;
    int text_cap = rem_buf <= TEXT_CAP ? rem_buf : TEXT_CAP;

    // printf("write: %d\n", text_cap);
    // for (int j = 0; j < text_cap; j++) {
    //     printf("R: %d %c\n", client->text->t[j], client->text->t[j]);
    // }

    DEBUG("Write text...\n");
    int w_len = client->send_func(client->fd, client->text->t + client->text->t_w_cur, text_cap, client);
    if (w_len < 1) {
        client_close(client);
        return;
    }

    if (rem_buf <= TEXT_CAP) {
        DEBUG("end of text...\n");
        if (client->send_func(client->fd, "<END OF TEXT>\r\n", 15, client) < 0) {
            PANIC("text write error");
        }

        client->text->t_w_cur = 0;
        client->handler = read_commond_and_write;
        ev_poll_to_read(client->server, client->fd, (void*)client);
    }
    else {
        client->text->t_w_cur += w_len;
    }

}

int recv_text_chunk(event_client* client) {
    DEBUG("read text chunck...\n");
    int start = client->text->t_cur;
    int recv_len = client->recv_func(client->fd, client->text->t + client->text->t_cur, client->text->t_cap - client->text->t_cur, client);
    if (recv_len < 1) {
        client_close(client);
        return 0;
    }
    client->text->t_cur += recv_len;

    int remine = client->text->t_cap - client->text->t_cur;
    int add = TEXT_CAP - remine;

    if (client->text->t_cur == client->text->t_cap) {
        DEBUG("realloc text buf...\n");
        client->text->t = (char*)realloc(client->text->t, sizeof(char) * (client->text->t_cap + add));
        client->text->t_cap += add;
    }
    DEBUG("read text chunck end...\n");
    // DEBUG("NOP2\n");
    if (client->ssl != NULL) {
        // DEBUG("COND1\n");
        if (SSL_pending(client->ssl) > 0) {
            // DEBUG("COND2\n");
            recv_text_chunk(client);
        }
    }

    return start;
}

int recv_file_chunk(event_client* client) {
    DEBUG("read text chunck...\n");
    int start = client->file->t_cur;
    int recv_len = client->recv_func(client->fd, client->file->t + client->file->t_cur, client->file->t_cap - client->file->t_cur, client);
    DEBUG("read text chunck...1\n");
    if (recv_len < 1) {
        client_close(client);
        return 0;
    }
    client->file->t_cur += recv_len;

    int remine = client->file->t_cap - client->file->t_cur;
    int add = TEXT_CAP - remine;

    if (client->file->t_cur == client->file->t_cap) {
        DEBUG("realloc text buf...\n");
        client->file->t = (char*)realloc(client->file->t, sizeof(char) * (client->file->t_cap + add));
        client->file->t_cap += add;
    }
    DEBUG("read text chunck end...\n");

    if (client->ssl != NULL) {
        DEBUG("COND1\n");
        if (SSL_pending(client->ssl) > 0) {
            // DEBUG("COND2\n");
            recv_file_chunk(client);
        }
    }

    return start;
}

int recv_chunk(event_client* client) {
    DEBUG("read commond chunck...\n");
    int recv_len = client->recv_func(client->fd, client->buf + client->buf_len, 16384 - client->buf_len, client);
    printf("recive commond %d\n", recv_len);
    if (recv_len < 1) {
        client_close(client);
        return 0;
    }
    client->buf_len += recv_len;
    return 1;
}

int get_commond_from_buf(event_client* client, char* commond) {
    int is_r = 0, i = 0;
    for (i = client->buf_cur; i < client->buf_len; i++) {
        // printf("%c %d\n", client->buf[i], i);
        if (client->buf[i] == '.' && is_r) {
            memcpy(commond, client->buf, i - 1);
            to_upper(commond, i - 1);
            return COMMOND_OK;
        }
        is_r = client->buf[i] == '.';
    }
    client->buf_cur = i;
    // printf("client->buf_cur %d\n", client->buf_cur);
    return COMMOND_MORE;
}

event_client* client_init() {
    event_client* ev_client = (event_client*)malloc(sizeof(event_client));
    ev_client->buf = (char*)malloc(sizeof(char) * 16384);
    ev_client->buf_len = 0;
    ev_client->buf_cur = 0;
    ev_client->ind = 0;
    ev_client->recv_func = recv_func;
    ev_client->send_func = send_func;
    ev_client->ssl = NULL;
    ev_client->f = NULL;

    text_buf* text = (text_buf*)malloc(sizeof(text_buf));
    text->t = (char*)malloc(sizeof(char) * TEXT_CAP);
    text->t_cap = TEXT_CAP;
    text->t_cur = 0;
    text->t_w_cur = 0;

    ev_client->text = text;

    text_buf* file = (text_buf*)malloc(sizeof(text_buf));
    file->t = (char*)malloc(sizeof(char) * TEXT_CAP);
    file->t_cap = TEXT_CAP;
    file->t_cur = 0;
    file->t_w_cur = 0;

    ev_client->file = file;

    return ev_client;
}

void client_close(event_client* client) {
    ev_poll_remove(client->server, client->fd);

    if (client->ssl != NULL) {
        SSL_shutdown(client->ssl);
        SSL_free(client->ssl);
    }

    if (client->f != NULL) {
        fclose(client->f);
    }

    close(client->fd);
    free(client->buf);
    free(client->text->t);
    free(client->text);
    free(client);
}

void ev_poll_set_read_fd(server_t* server, int fd, void* ptr) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = ptr;

    if (epoll_ctl(server->event_loop_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        PANIC("error on add fd in poll event poll");
    }
}

void ev_poll_set_write_fd(server_t* server, int fd, void* ptr) {
    struct epoll_event ev;
    ev.events = EPOLLOUT;
    ev.data.ptr = ptr;

    if (epoll_ctl(server->event_loop_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        PANIC("error on add fd in poll event poll");
    }
}

void ev_poll_to_write(server_t* server, int fd, void* ptr) {
    struct epoll_event ev;
    ev.events = EPOLLOUT;
    ev.data.ptr = ptr;

    if (epoll_ctl(server->event_loop_fd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        PANIC("error on set read to write fd event poll");
    }
}

void ev_poll_to_read(server_t* server, int fd, void* ptr) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = ptr;

    if (epoll_ctl(server->event_loop_fd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        PANIC("error on set write to read fd event poll");
    }
}

void ev_poll_remove(server_t* server, int fd) {
    if (epoll_ctl(server->event_loop_fd, EPOLL_CTL_DEL, fd, NULL) < 0) {
        PANIC("error on remove fd in event poll");
    }
}

int send_func(int fd, const void* buf, size_t len, event_client* clinet) {
    return send(fd, buf, len, 0);
}
int recv_func(int fd, void* buf, size_t len, event_client* clinet) {
    return recv(fd, buf, len, 0);
}

int send_func_ssl(int fd, const void* buf, size_t len, event_client* clinet) {
    return SSL_write(clinet->ssl, buf, len);
}
int recv_func_ssl(int fd, void* buf, size_t len, event_client* clinet) {
    int r = SSL_read(clinet->ssl, buf, len);
    printf("read: %d\n", r);
    return r;
}

void server_set_port(server_t* server, int port) {
    server->port = port;
}

void to_upper(char* s, int l) {
    for (int i = 0; i < l; i++) {
        s[i] = toupper(s[i]);
    }
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

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX* ctx) {
    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the certificate and private key */
    if (SSL_CTX_use_certificate_file(ctx, CERT_FILE, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, KEY_FILE, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}
