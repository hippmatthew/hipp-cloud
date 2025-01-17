#include "include/server.h"

#include <stdio.h>
#include <getopt.h>

int main(int argc, char ** argv) {
  char * port = NULL;

  int c = -1;
  while ((c = getopt(argc, argv, "p:")) != -1) {
    switch (c) {
      case 'p':
        port = optarg;
        break;
      case '?':
        (void)printf("invalid argument.\ncorrect usage: server -p <port>\n");
        break;
      default:
        return -1;
    }
  }

  if (port == NULL) {
    (void)printf("must supply a port.\ncorrect usage: server -p <port> -o <path/to/log>\n");
    return -1;
  }

  server_t server = {0};
  server.is_running = 0;

  switch (start_server(&server, port)) {
    case SERVER_OK:
      (void)printf("listening on port %s\n", port);
      server.is_running = 1;
      break;
    case SERVER_SOCKET_ERR:
      (void)printf("failed to create non-waiting socket\n");
      return -1;
    case SERVER_INVALID_PORT:
      (void)printf("port %s deemed invalid\n", port);
      return -1;
    case SERVER_BIND_ERR:
      (void)printf("failed to bind server to port %s\n", port);
      return -1;
    case SERVER_LISTEN_ERR:
      (void)printf("failed to listen on port %s\n", port);
      return -1;
  }

  struct pollfd poll_fds[MAX_CLIENTS + 1];

  poll_fds[0].fd = server.sock_fd;
  poll_fds[0].events = POLLIN;

  while (server.is_running) {
    int event_count = poll_events(&server, poll_fds);

    if (poll_fds[0].revents & POLLIN) {
      int index;
      switch(accept_client(&server, &index)) {
        case ACCEPT_OK:
          (void)printf("%s (fd %d) connected\n",
            inet_ntoa(server.clients[index].addr.sin_addr), server.clients[index].sock_fd);
          break;
        case ACCEPT_SERVER_FULL_ERR:
          (void)printf("attempted to connect client but server is full\n");
          break;
        case ACCEPT_SOCKET_ERR:
          (void)printf("attempted to connect client but failed to create socket\n");
      }
      --event_count;
    }

    for (int i = 1; i <= server.client_count + 1 && event_count > 0; ++i) {
      if (poll_fds[i].revents & POLLIN) {
        // parse HTTP request

        --event_count;

        int index = get_client_index(poll_fds[i].fd, server.clients);
        if (index == -1) continue;

        char buf[4096] = {0};
        int bytes = read(poll_fds[i].fd, buf, 4096);
        if (bytes <= 0) {
          (void)close(poll_fds[i].fd);
          (void)printf("%s (fd %d) disconnected\n",
            inet_ntoa(server.clients[i - 1].addr.sin_addr), poll_fds[i].fd);
          continue;
        }

        server.clients[i - 1].poll_event = POLLOUT;
      }
      else if (poll_fds[i].revents & POLLOUT) {
        // send HTTP response

        --event_count;
        server.clients[i - 1].poll_event = POLLIN;
      }
    }
  }

  (void)printf("disconnected %d clients\nstopped listening", stop_server(&server));

  return 0;
}