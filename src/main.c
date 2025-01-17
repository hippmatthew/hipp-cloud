#include "include/http.h"
#include "include/server.h"

#include <stdio.h>
#include <getopt.h>
#include <string.h>

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
        --event_count;
        int index = server.client_cache[i - 1];

        if (server.clients[index].sock_fd != poll_fds[i].fd) {
          (void)printf("fd mismatch for client %s (poll: %d, fd: %d). disconnecting\n",
            inet_ntoa(server.clients[index].addr.sin_addr),
            poll_fds[i].fd,
            server.clients[index].sock_fd
          );

          (void)close(poll_fds[i].fd);
          disconnect_client(&server.clients[index]);

          continue;
        }

        request_t request = {0};
        switch(parse_request(poll_fds[i].fd, &request)) {
          case PARSE_OK:
            (void)printf("%s (fd %d): %s %s %s\n",
              inet_ntoa(server.clients[index].addr.sin_addr), poll_fds[i].fd,
              request.method, request.path, request.http_prot
            );
            break;
          case PARSE_INVALID:
            (void)printf("%s (fd %d): failed to parse request. disconnecting\n",
              inet_ntoa(server.clients[index].addr.sin_addr), poll_fds[i].fd
            );
            disconnect_client(&server.clients[index]);
            continue;
        }

        switch(process_request(request, &server.clients[index].response)) {
          case PROC_OK:
            (void)printf("%s (fd %d): processed request\n",
              inet_ntoa(server.clients[index].addr.sin_addr), poll_fds[i].fd
            );
            break;
          case PROC_FILE_ERR:
            (void)printf("%s (fd %d): failed to process request. file failed to open\n",
              inet_ntoa(server.clients[index].addr.sin_addr), poll_fds[i].fd
            );
            continue;
          case PROC_BYTES_ERR:
            (void)printf("%s (fd %d): failed to process request. file size not obtained\n",
              inet_ntoa(server.clients[index].addr.sin_addr), poll_fds[i].fd
            );
            continue;
          case PROC_ALLOC_ERR:
            (void)printf("%s (fd %d): failed to process request. buffer not allocated\n",
              inet_ntoa(server.clients[index].addr.sin_addr), poll_fds[i].fd
            );
            continue;
          case PROC_READ_ERR:
            (void)printf("%s (fd %d): failed to process request. file could not be read\n",
              inet_ntoa(server.clients[index].addr.sin_addr), poll_fds[i].fd
            );
            continue;
          case PROC_HEAD_ERR:
            (void)printf("%s (fd %d): failed to process request. header was not constructed\n",
              inet_ntoa(server.clients[index].addr.sin_addr), poll_fds[i].fd
            );
            continue;
        }

        if (!strcmp(request.path, "/stop"))
          server.is_running = 0;

        server.clients[index].poll_event = POLLOUT;
      }
      else if (poll_fds[i].revents & POLLOUT) {
        --event_count;
        int index = server.client_cache[i - 1];

        if (server.clients[index].sock_fd != poll_fds[i].fd) {
          (void)printf("hipp-cloud: fd mismatch for client %s (poll: %d, fd: %d). disconnecting\n",
            inet_ntoa(server.clients[index].addr.sin_addr),
            poll_fds[i].fd,
            server.clients[index].sock_fd
          );

          (void)close(poll_fds[i].fd);
          disconnect_client(&server.clients[index]);

          continue;
        }

        switch(send_response(poll_fds[i].events, server.clients[index].response)) {
          case RES_OK:
            (void)printf("%s (fd %d): recieved response\n",
              inet_ntoa(server.clients[index].addr.sin_addr), poll_fds[i].fd
            );
            break;
          case RES_HEAD_ERR:
            (void)printf("%s (fd %d): failed to recieve header\n",
              inet_ntoa(server.clients[index].addr.sin_addr), poll_fds[i].fd
            );
           break;
          case RES_DATA_ERR:
            (void)printf("%s (fd %d): failed to recieve data\n",
              inet_ntoa(server.clients[index].addr.sin_addr), poll_fds[i].fd
            );
        }

        server.clients[index].poll_event = POLLIN;
      }
    }
  }

  (void)printf("disconnected %d clients\nstopped listening", stop_server(&server));

  return 0;
}