#include "include/log.h"
#include "include/tcp.h"
#include "include/http.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
  if (argc != 3) {
    (void)printf("incorrect number of arguments. usage is as follows: server <port> <log file path>\n");
    exit(EXIT_FAILURE);
  }

  int port = 0;
  (void)sscanf(argv[1], "%d", &port);

  log_t log = {0};
  set_log_path(&log, argv[2]);
  begin_log(&log);

  server_t server = {0};

  switch(start(&server, port)) {
    case SERVER_OK:
      log_msg(log.fd, "server started listening on port %s\n", argv[1]);
      break;
    case SERVER_SOCKET_ERR:
      log_msg(log.fd, "failed to create socket");
      exit(EXIT_FAILURE);
    case SERVER_BIND_ERROR:
      log_msg(log.fd, "failed to bind socket to port %s\n", argv[1]);
      exit(EXIT_FAILURE);
    case SERVER_LISTEN_ERROR:
      log_msg(log.fd, "socket failed to listen on port %s\n", argv[1]);
      exit(EXIT_FAILURE);
  }

  while (1) {
    int client_fd = accept_client(server.socket_fd);
    if (client_fd == -1) {
      log_msg(log.fd, "failed to accept client\n");
      stop(&server);
      exit(EXIT_FAILURE);
    }

    log_msg(log.fd, "client connected\n");

    request_t request = {0};
    if (parse(client_fd, &request) == PARSE_INVALID) {
      log_msg(log.fd, "\tPARSE ERR\n");

      close(client_fd);
      log_msg(log.fd, "\tDISCONNECT\n");

      continue;
    }

    log_msg(log.fd, "\t%s %s %s\n", request.method, request.path, request.http_prot);

    if (!strcmp(request.path, "/stop")) {
      close(client_fd);
      log_msg(log.fd, "\tDISCONNECT\n");
      break;
    }

    response_t response = {0};
    switch (handle_request(&request, &response)) {
      case HANDLE_OK:
        log_msg(log.fd, "\tRES %s %d\n", response.path, response.stylesheet);
        break;
      case HANDLE_INVALID_METHOD:
        log_msg(log.fd, "\tINVALID METHOD\n");
        close(client_fd);
        log_msg(log.fd, "\tDISCONNECT\n");
        continue;
    }

    switch (respond(client_fd, &response)) {
      case RES_OK:
        log_msg(log.fd, "\tRESPONDED\n");
        break;
      case RES_FAILED_OPEN:
        log_msg(log.fd, "\tRES OPEN ERR\n");
        break;
      case RES_FAILED_STAT:
        log_msg(log.fd, "\tRES STAT ERR\n");
        break;
      case RES_FAILED_ALLOC:
        log_msg(log.fd, "\tRES ALLOC ERR\n");
        break;
      case RES_FAILED_READ:
        log_msg(log.fd, "\tRES READ ERR\n");
        break;
      case RES_FAILED_HEADER:
        log_msg(log.fd, "\tRES HEAD ERR\n");
        break;
      case RES_FAILED_SEND:
        log_msg(log.fd, "\tRES SEND ERR\n");
    }

    close(client_fd);
    log_msg(log.fd, "\tDISCONNECT\n");
  }

  stop(&server);
  log_msg(log.fd, "server stopped listening on port %s\n", argv[1]);

  end_log(&log);

  exit(EXIT_SUCCESS);
}