#include "include/log.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void set_path(logger_t * logger, const char * path) {
  if (strlen(path) > 255) {
    (void)printf("given path exceeds character limit (256)\n");
    return;
  }

  (void)memcpy((void *)logger->path, path, strlen(path));
}

void begin_log(logger_t * logger) {
  logger->file = fopen(logger->path, "w");
  if (logger->file == NULL) {
    (void)printf("failed to open log file\n");
    logger->initialized = 0;
    return;
  }

  logger->initialized = 1;
}

void end_log(logger_t * logger) {
  if (fclose(logger->file) != 0) {
    (void)printf("failed to close log file\n");
    return;
  }

  logger->file = NULL;
  logger->initialized = 0;
}

void log_msg(logger_t * logger, const char * format, ...) {
  va_list args;
  va_start(args, format);

  (void)vfprintf(logger->file, format, args);
  (void)fflush(logger->file);

  va_end(args);
}