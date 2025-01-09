#include "include/http.h"

#include <unistd.h>
#include <stdio.h>

int parse(int socket, request_t * request) {
  char buf[HTTP_REQ_MAX_LEN] = {0};

  long bytes = read(socket, buf, sizeof(buf) - 1);
  if (bytes <= 0) return PARSE_INVALID;

  buf[bytes] = '\0';

  if (sscanf(buf, "%7s %2047s %15s", request->method, request->path, request->http_prot) != 3)
    return PARSE_INVALID;

  return PARSE_OK;
}