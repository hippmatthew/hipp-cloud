#include "include/tcp.h"

server_status_e start(server_t * restrict server, int port) {
  server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->socket_fd == -1) return SERVER_SOCKET_ERR;

  int val = 1;
  if (setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
    return SERVER_SOCKET_ERR;


  server->address.sin_family = AF_INET;
  server->address.sin_addr.s_addr = INADDR_ANY;
  server->address.sin_port = htons(port);

  if (bind(server->socket_fd, (struct sockaddr *)&server->address, sizeof(server->address)) == -1) {
    (void)close(server->socket_fd);
    return SERVER_BIND_ERROR;
  }

  if (listen(server->socket_fd, 5) == -1) {
    (void)close(server->socket_fd);
    return SERVER_LISTEN_ERROR;
  }

  return SERVER_OK;
}

void stop(server_t * server) {
  (void)close(server->socket_fd);
}

int accept_client(int server_socket) {
  struct sockaddr_in client_address = {0};
  socklen_t client_socklen = sizeof(client_address);

  return accept(server_socket, (struct sockaddr *)&client_address, &client_socklen);
}