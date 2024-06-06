/*
 * Copyright (c) 2024 Parthka <hello@parthka.dev>
 * Licensed under the unlicense
 * made with 💻 🧠 🤖 by Parthka
 */

#ifdef DEV_MOD
#define DEBUG(a1) printf(a1)
#else
#define DEBUG(a1)
#endif

#define PANIC(a1) \
    perror(a1);   \
    exit(2)

typedef struct _server_t {
    int socket_fd;
    int port;
    int event_loop_fd;
} server_t;

typedef struct _event_client event_client;
typedef struct _event_server event_server;

typedef void (*handler_func)(void*);
typedef int (*sendandrecvfunc)(int, char*, int, event_client*);

struct _event_server {
    int fd;
    server_t* server;
    handler_func handler;
};

typedef struct _crlf_t
{
    int f_r;
    int f_n;
    int d;
    int s_r;
    int s_n;
} crlf_t;

typedef struct _text_buf {
    char* t;
    int t_cur;
    int t_cap;
    int t_w_cur;
    crlf_t crlf;
} text_buf;

struct _event_client {
    int fd;
    server_t* server;
    handler_func handler;
    char* buf;
    int buf_len;
    int buf_cur;
    int ind;
    text_buf* text;
    sendandrecvfunc recv_func;
    sendandrecvfunc send_func;
};

#define TEXT_CAP 20

void server_set_port(server_t*, int);
void server_make(server_t*);
void server_listen(server_t*);
void server_start(server_t*);

void ev_poll_set_read_fd(server_t*, int, void*);
void ev_poll_set_write_fd(server_t*, int, void*);
void ev_poll_to_write(server_t*, int, void*);
void ev_poll_to_read(server_t*, int, void*);
void ev_poll_remove(server_t*, int);
void ev_poll_start(server_t*);

event_client* client_init();
void client_close(event_client*);

void accept_new_connection(void*);
void send_hello(void*);
void read_commond_and_write(void*);
void read_and_write(void*);
void send_text(void*);
void recv_chunk(event_client*);
void recv_chunk(event_client*);
int recv_text_chunk(event_client*);
int get_commond_from_buf(event_client*, char*);

enum PARSE_PARSE {
    PARSE_OK,
    PARSE_MORE
};
int parse_text(event_client*, int);

enum GET_COMMOND {
    COMMOND_MORE = 0,
    COMMOND_OK = 1,
    COMMOND_ERROR = -1
};

void commond_handle(event_client*, char*);
void text_handle(void*);

void to_upper(char*, int);

int send_func(int, char*, int, event_client*);
int recv_func(int, char*, int, event_client*);
