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

  logger_t logger = {0};
  set_path(&logger, argv[2]);
  begin_log(&logger);

  server_t server = {0};

  switch(start(&server, port)) {
    case SERVER_OK:
      log_msg(&logger, "server starrted listening on port %s\n", argv[1]);
      break;
    case SERVER_SOCKET_ERR:
      log_msg(&logger, "failed to create socket");
      exit(EXIT_FAILURE);
    case SERVER_BIND_ERROR:
      log_msg(&logger, "failed to bind socket to port %s\n", argv[1]);
      exit(EXIT_FAILURE);
    case SERVER_LISTEN_ERROR:
      log_msg(&logger, "socket failed to listen on port %s\n", argv[1]);
      exit(EXIT_FAILURE);
  }

  int client_fd = accept_client(server.socket_fd);
  if (client_fd == -1) {
    log_msg(&logger, "failed to accept client\n");
    stop(&server);
    exit(EXIT_FAILURE);
  }

  log_msg(&logger, "client connected\n");

  request_t request = {0};
  if (parse(client_fd, &request) == PARSE_OK)
    log_msg(&logger, "\t%s %s %s\n", request.method, request.path, request.http_prot);
  else
    log_msg(&logger, "failed to parse http request\n");

  close(client_fd);
  log_msg(&logger, "client disconnected\n");

  stop(&server);
  log_msg(&logger, "server stopped listening on port %s\n", argv[1]);

  end_log(&logger);

  exit(EXIT_SUCCESS);
}