#ifndef hippcloud_tcp_h
#define hippcloud_tcp_h

#include "http.h"

#include <arpa/inet.h>
#include <poll.h>

#define MAX_CLIENTS 12

typedef enum {
  SERVER_OK,
  SERVER_SOCKET_ERR,
  SERVER_INVALID_PORT,
  SERVER_BIND_ERR,
  SERVER_LISTEN_ERR
} server_status_e;

typedef enum {
  ACCEPT_OK,
  ACCEPT_SERVER_FULL_ERR,
  ACCEPT_SOCKET_ERR
} accept_status_e;

typedef struct {
  int sock_fd;
  int poll_event;
  struct sockaddr_in addr;
  request_t request;
  http_response_t response;
} client_t;

typedef struct {
  int is_running;
  int sock_fd;
  struct sockaddr_in addr;
  int client_count;
  client_t clients[MAX_CLIENTS];
} server_t;

server_status_e start_server(server_t * server, char * port);
int stop_server(server_t * server);
int poll_events(server_t * server, struct pollfd * poll_fds);
accept_status_e accept_client(server_t * server, int * index);
int get_client_index(int fd, client_t * clients);

#endif // hippcloud_tcp_h