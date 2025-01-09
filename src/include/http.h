#ifndef hippcloud_protocol_h
#define hippcloud_protocol_h

#define HTTP_REQ_MAX_LEN    32768
#define HTTP_METHOD_MAX_LEN 8
#define HTTP_PATH_MAX_LEN   2048
#define HTTP_PROT_MAX_LEN   16

typedef enum {
  PARSE_INVALID,
  PARSE_OK
} parse_status_e;

typedef struct {
  char method[HTTP_METHOD_MAX_LEN];
  char path[HTTP_PATH_MAX_LEN];
  char http_prot[HTTP_PROT_MAX_LEN];
} request_t;

int parse(int socket, request_t * request);

#endif // hippcloud_protocol_h