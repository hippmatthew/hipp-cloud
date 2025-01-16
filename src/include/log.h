#ifndef hippcloud_log_h
#define hippcloud_log_h

typedef struct {
  char path[257];
  int fd;
} log_t;

void set_log_path(log_t * log, const char * path);
void begin_log(log_t * log);
void end_log(log_t * log);
void log_msg(int fd, const char * format, ...);

#endif // hippcloud_log_h