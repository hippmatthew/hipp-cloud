#include "include/log.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>

void set_log_path(log_t * log, const char * path) {
  if (strlen(path) > 255) {
    (void)printf("given path exceeds character limit of 256\n");
    return;
  }

  (void)memcpy((void *)log->path, path, strlen(path));
  log->path[strlen(path)] = '\0';
}

void begin_log(log_t * log) {
  log->fd = open(log->path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log->fd == -1)
    (void)perror("open");
}

void end_log(log_t * log) {
  if (log->fd == -1) return;

  (void)close(log->fd);
  log->fd = -1;
}

void log_msg(int fd, const char * format, ...) {
  va_list args;
  va_start(args, format);

  char buf[256] = {0};
  vsprintf(buf, format, args);

  (void)write(fd, buf, strlen(buf));
  (void)fsync(fd);

  va_end(args);
}