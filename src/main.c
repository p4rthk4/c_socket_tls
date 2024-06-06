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

// header file
#include <main.h>

int main() {
    server_t server;
    server_set_port(&server, 5005);
    server_make(&server);
    server_listen(&server);
    server_start(&server);
}

void server_make(server_t* server) {
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
    client->recv_func = recv_func;
    client->send_func = send_func;

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
    recv_chunk(client);

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
        int send_rtv = client->send_func(client->fd, "ERROR NO IMPLI\r\n", 16, client);
        if (send_rtv < 1) {
            PANIC("send error");
        }
    }
    else if (strcmp(commond, "STARTTLS") == 0) {
        int send_rtv = client->send_func(client->fd, "ERROR NO IMPLI\r\n", 16, client);
        if (send_rtv < 1) {
            PANIC("send error");
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

    if (parse_text(client, start) == PARSE_OK) {
        client->handler = read_commond_and_write;
        int send_rtv = client->send_func(client->fd, "OK TEXT WRITE\r\n", 15, 0);
        if (send_rtv < 1) {
            PANIC("send error");
        }
    }
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

    int w_len = client->send_func(client->fd, client->text->t + client->text->t_w_cur, text_cap, 0);
    if (w_len < 1) {
        client_close(client);
        return;
    }
    DEBUG("WRITE TEXT...\n");

    if (rem_buf <= TEXT_CAP) {
        if (send(client->fd, "<END OF TEXT>\r\n", 15, 0) < 0) {
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
    }
    client->text->t_cur += recv_len;
    if (client->text->t_cur == client->text->t_cap) {
        DEBUG("realloc text buf...\n");
        client->text->t = (char*)realloc(client->text->t, sizeof(char) * (client->text->t_cap + TEXT_CAP));
        client->text->t_cap += TEXT_CAP;
    }
    return start;
}


void recv_chunk(event_client* client) {
    DEBUG("read chunck...\n");
    int recv_len = client->recv_func(client->fd, client->buf + client->buf_len, 1024 - client->buf_len, client);
    if (recv_len < 1) {
        client_close(client);
    }
    client->buf_len += recv_len;
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
    ev_client->buf = (char*)malloc(sizeof(char) * 1024);
    ev_client->buf_len = 0;
    ev_client->buf_cur = 0;
    ev_client->ind = 0;

    text_buf* text = (text_buf*)malloc(sizeof(text_buf));
    text->t = (char*)malloc(sizeof(char) * TEXT_CAP);
    text->t_cap = TEXT_CAP;
    text->t_cur = 0;

    ev_client->text = text;

    return ev_client;
}

void client_close(event_client* client) {
    ev_poll_remove(client->server, client->fd);
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

int send_func(int fd, char* buf, int len, event_client* clinet) {
    return send(fd, buf, len, 0);
}
int recv_func(int fd, char* buf, int len, event_client* clinet) {
    return recv(fd, buf, len, 0);
}

void server_set_port(server_t* server, int port) {
    server->port = port;
}

void to_upper(char* s, int l) {
    for (int i = 0; i < l; i++) {
        s[i] = toupper(s[i]);
    }
}
