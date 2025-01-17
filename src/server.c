#include "include/server.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int find_index(client_t * clients);

server_status_e start_server(server_t * server, char * port_str) {
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    server->clients[i].sock_fd = -1;
    server->clients[i].poll_event = POLLIN;

    memset(&server->clients[i].addr, 0, sizeof(struct sockaddr_in));

    server->clients[i].response.bytes = 0;
    server->clients[i].response.data = NULL;

    server->client_cache[i] = -1;
  }

  server->client_count = 0;

  if ((server->sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    return SERVER_SOCKET_ERR;

  int opt = 1;
  if (setsockopt(server->sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    return SERVER_SOCKET_ERR;

  int port;
  if (sscanf(port_str, "%d", &port) != 1)
    return SERVER_INVALID_PORT;

  server->addr.sin_family = AF_INET;
  server->addr.sin_addr.s_addr = INADDR_ANY;
  server->addr.sin_port = htons(port);

  if (bind(server->sock_fd, (struct sockaddr *)&server->addr, sizeof(server->addr)) == -1) {
    (void)close(server->sock_fd);
    return SERVER_BIND_ERR;
  }

  if (listen(server->sock_fd, 0) == -1) {
    (void)close(server->sock_fd);
    return SERVER_LISTEN_ERR;
  }

  return SERVER_OK;
}

int stop_server(server_t * server) {
  int clients = 0;
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (server->clients[i].sock_fd == -1) continue;

    disconnect_client(&server->clients[i]);
    ++clients;
  }

  (void)close(server->sock_fd);
  server->is_running = 0;

  return clients;
}

int poll_events(server_t * server, struct pollfd * poll_fds) {
  int remaining_clients = server->client_count;
  for (int i = 0, j = 1; i < MAX_CLIENTS && remaining_clients > 0; ++i) {
    if (server->clients[i].sock_fd == -1) continue;

    server->client_cache[j - 1] = i;
    poll_fds[j].fd = server->clients[i].sock_fd;
    poll_fds[j].events = server->clients[i].poll_event;

    --remaining_clients;
    ++j;
  }

  return poll(poll_fds, server->client_count + 1, -1);
}

accept_status_e accept_client(server_t * server, int * index) {
  *index = find_index(server->clients);
  if (*index == -1) return ACCEPT_SERVER_FULL_ERR;

  socklen_t cl_socklen = sizeof(server->clients[*index].addr);
  server->clients[*index].sock_fd = accept(
    server->sock_fd,
    (struct sockaddr *)&server->clients[*index].addr,
    &cl_socklen
  );
  if (server->clients[*index].sock_fd == -1) return ACCEPT_SOCKET_ERR;

  ++server->client_count;

  return ACCEPT_OK;
}

void disconnect_client(client_t * client) {
  if (client->sock_fd == -1) return;

  (void)close(client->sock_fd);
  client->sock_fd = -1;

  if (client->response.data != NULL)
    free(client->response.data);
}

int find_index(client_t * clients) {
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i].sock_fd == -1)
      return i;
  }

  return -1;
}