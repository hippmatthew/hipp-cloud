#ifndef hippcloud_protocol_h
#define hippcloud_protocol_h

#define HTTP_METHOD_MAX_LEN 8
#define HTTP_PROT_MAX_LEN   16
#define HTTP_PATH_MAX_LEN   256
#define HTTP_REQ_MAX_LEN    8192

typedef enum {
  GET,
  POST,
  METHOD_INVALID
} method_e;

typedef enum {
  PARSE_INVALID,
  PARSE_OK
} parse_status_e;

typedef enum {
  HANDLE_OK,
  HANDLE_INVALID_METHOD
} handle_status_e;

typedef enum {
  RES_OK,
  RES_FAILED_OPEN,
  RES_FAILED_STAT,
  RES_FAILED_ALLOC,
  RES_FAILED_READ,
  RES_FAILED_HEADER,
  RES_FAILED_SEND
} response_status_e;

typedef struct {
  char method[HTTP_METHOD_MAX_LEN];
  char path[HTTP_PATH_MAX_LEN];
  char http_prot[HTTP_PROT_MAX_LEN];
} request_t;

typedef struct {
  char path[HTTP_PATH_MAX_LEN];
  int stylesheet;
} response_t;

typedef struct {
  long bytes;
  char * data;
} http_response_t;

parse_status_e parse(int socket, request_t * request);
handle_status_e handle_request(request_t * request, response_t * response);
response_status_e respond(int socket, response_t * response);

#endif // hippcloud_protocol_h