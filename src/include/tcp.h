#ifndef hippcloud_server_h
#define hippcloud_server_h

#include "log.h"

#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include "clients.h"

typedef enum {
  SERVER_OK,
  SERVER_SOCKET_ERR,
  SERVER_BIND_ERROR,
  SERVER_LISTEN_ERROR
} server_status_e;

typedef struct {
  int socket_fd;
  struct sockaddr_in address;
} server_t;

server_status_e start(server_t * server, int port);
void stop(server_t * server);
int accept_client(int server_socket);
int poll_clients(clients_t * clients);

#endif // hippcloud_server_h