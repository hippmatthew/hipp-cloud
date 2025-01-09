#ifndef hippcloud_log_h
#define hippcloud_log_h

#include <stdio.h>

typedef struct {
  char path[256];
  FILE * file;
  int initialized;
} logger_t;

void set_path(logger_t * logger, const char * path);
void begin_log(logger_t * logger);
void end_log(logger_t * logger);
void log_msg(logger_t * logger, const char * format, ...);

#endif // hippcloud_log_h