#include "include/http.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

parse_status_e parse_request(int socket, request_t * request) {
  char buf[HTTP_REQ_MAX_LEN] = {0};

  long bytes = read(socket, buf, sizeof(buf) - 1);
  if (bytes <= 0) return PARSE_INVALID;

  buf[bytes] = '\0';

  if (sscanf(buf, "%7s %2047s %15s", request->method, request->path, request->http_prot) != 3)
    return PARSE_INVALID;

  return PARSE_OK;
}

proc_status_e process_request(request_t request, response_t * response) {
  char ext[5];
  memcpy(ext, request.path + strlen(request.path) - 4, 4);
  (void)printf("ext: %s\n", ext);

  char src[12] = "data/src";
  char type[10] = "text/html";
  if (strcmp(ext, ".css") == 0) {
    (void)sscanf("data/styles", "%s", src);
    (void)sscanf("text/css", "%s", type);
  }

  char path[268] = "";
  strcat(path, src);
  strcat(path, request.path);
  (void)printf("request path: %s\n", path);

  int fd = open(path, O_RDONLY);
  if (fd == -1) return PROC_FILE_ERR;

  struct stat stat;
  if (fstat(fd, &stat) == -1)
    return PROC_BYTES_ERR;

  if ((response->data = (char *)malloc(stat.st_size)) == NULL) {
    (void)close(fd);
    return PROC_ALLOC_ERR;
  }

  if ((response->bytes = read(fd, response->data, stat.st_size)) <= 0) {
    (void)close(fd);

    free(response->data);
    response->data = NULL;

    return PROC_READ_ERR;
  }

  (void)close(fd);

  if ((response->header_size = snprintf(response->header, sizeof(response->header),
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %ld\r\n"
    "Connection: keep-alive\r\n"
    "\r\n",
    type,
    response->bytes
  )) <= 0) {
    free(response->data);
    response->data = NULL;
    return PROC_HEAD_ERR;
  }

  return PROC_OK;
}

res_status_e send_response(int fd, response_t response) {
  if (write(fd, response.header, response.header_size) == -1) {
    free(response.data);
    response.data = NULL;
    return RES_HEAD_ERR;
  }

  if (write(fd, response.data, response.bytes) == -1) {
    free(response.data);
    response.data = NULL;
    return RES_DATA_ERR;
  }

  free(response.data);
  response.data = NULL;

  return RES_OK;
}

// response_status_e respond(int socket, response_t * response) {
//   int fd = open(response->path, O_RDONLY);
//   if (fd == -1) return RES_FAILED_OPEN;

//   struct stat st = {0};
//   if (fstat(fd, &st) == -1) {
//     (void)close(fd);
//     return RES_FAILED_STAT;
//   }

//   char * data = (char *)malloc(st.st_size);
//   if (data == NULL) {
//     (void)close(fd);
//     return RES_FAILED_ALLOC;
//   }

//   long bytes = read(fd, data, st.st_size);
//   if (bytes == 0) {
//     free(data);
//     close(fd);
//     return RES_FAILED_READ;
//   }

//   (void)close(fd);

//   char header[1024];
//   int header_size = snprintf(header, sizeof(header),
//     "HTTP/1.1 200 OK\r\n"
//     "Content-Type: %s\r\n"
//     "Content-Length: %ld\r\n"
//     "Connection: close\r\n"
//     "\r\n",
//     response->stylesheet ? "text/html" : "test/css",
//     bytes
//   );

//   if (write(socket, header, header_size) == -1) {
//     free(data);
//     return RES_FAILED_HEADER;
//   }

//   if (write(socket, data, bytes) == -1) {
//     free(data);
//     return RES_FAILED_SEND;
//   }

//   free(data);
//   return RES_OK;
// }