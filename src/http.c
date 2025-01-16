#include "include/http.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

method_e method_from_str(char * str) {
  if (!strcmp(str, "GET")) return GET;
  else if (!strcmp(str, "POST")) return POST;

  return METHOD_INVALID;
}

parse_status_e parse(int socket, request_t * request) {
  char buf[HTTP_REQ_MAX_LEN] = {0};

  long bytes = read(socket, buf, sizeof(buf) - 1);
  if (bytes <= 0) return PARSE_INVALID;

  buf[bytes] = '\0';

  if (sscanf(buf, "%7s %2047s %15s", request->method, request->path, request->http_prot) != 3)
    return PARSE_INVALID;

  return PARSE_OK;
}

handle_status_e handle_request(request_t * request, response_t * response) {
  char end[5];

  // switch(method_from_str(request->method)) {
  //   case GET:
      memcpy(end, request->path + strlen(request->path) - 5, 5);
      response->stylesheet = !strcmp(end, ".css");
      (void)sprintf(response->path, "data/%s%s", response->stylesheet ? "styles" : "src", request->path);
  //   case POST:
  //   case METHOD_INVALID:
  //     return HANDLE_INVALID_METHOD;
  // }

  return HANDLE_OK;
}

response_status_e respond(int socket, response_t * response) {
  int fd = open(response->path, O_RDONLY);
  if (fd == -1) return RES_FAILED_OPEN;

  struct stat st = {0};
  if (fstat(fd, &st) == -1) {
    (void)close(fd);
    return RES_FAILED_STAT;
  }

  char * data = (char *)malloc(st.st_size);
  if (data == NULL) {
    (void)close(fd);
    return RES_FAILED_ALLOC;
  }

  long bytes = read(fd, data, st.st_size);
  if (bytes == 0) {
    free(data);
    close(fd);
    return RES_FAILED_READ;
  }

  (void)close(fd);

  char header[1024];
  int header_size = snprintf(header, sizeof(header),
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %ld\r\n"
    "Connection: close\r\n"
    "\r\n",
    response->stylesheet ? "text/html" : "test/css",
    bytes
  );

  if (write(socket, header, header_size) == -1) {
    free(data);
    return RES_FAILED_HEADER;
  }

  if (write(socket, data, bytes) == -1) {
    free(data);
    return RES_FAILED_SEND;
  }

  free(data);
  return RES_OK;
}