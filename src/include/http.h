#ifndef hippcloud_protocol_h
#define hippcloud_protocol_h

#define HTTP_METHOD_MAX_LEN 8
#define HTTP_PROT_MAX_LEN   16
#define HTTP_PATH_MAX_LEN   256
#define HTTP_REQ_MAX_LEN    8192
#define HTTP_HEAD_MAX_LEN   1024

typedef enum {
  PARSE_OK,
  PARSE_INVALID,
} parse_status_e;

typedef enum {
  PROC_OK,
  PROC_FILE_ERR,
  PROC_BYTES_ERR,
  PROC_ALLOC_ERR,
  PROC_READ_ERR,
  PROC_HEAD_ERR
} proc_status_e;

typedef enum {
  RES_OK,
  RES_HEAD_ERR,
  RES_DATA_ERR
} res_status_e;

typedef struct {
  char method[HTTP_METHOD_MAX_LEN];
  char path[HTTP_PATH_MAX_LEN];
  char http_prot[HTTP_PROT_MAX_LEN];
} request_t;

typedef struct {
  int header_size;
  char header[HTTP_HEAD_MAX_LEN];
  long bytes;
  char * data;
} response_t;

parse_status_e parse_request(int socket, request_t * request);
proc_status_e process_request(request_t request, response_t * response);
res_status_e send_response(int fd, response_t response);


#endif // hippcloud_protocol_h