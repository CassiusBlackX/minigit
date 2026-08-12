#include "minigit/dynamic_arr.h"

#include <stdio.h>
#include <stdarg.h>

int buf_append_printf_func(buffer *buf, const char *format, ...) {
  if (!buf || !format)
    return -1;

  va_list args;
  va_start(args, format);

  int len = vsnprintf(NULL, 0, format, args);
  va_end(args);
  if (len < 0)
    return -1;

  size_t need = buf->count + (size_t)len + 1;
  if (need > buf->capacity) {
    if (buf->capacity == 0)
      buf->capacity = 256;
    while (buf->capacity < need)
      buf->capacity *= 2;
    buf->arr = realloc(buf->arr, sizeof(*buf->arr) * buf->capacity);
  }

  va_start(args, format);
  vsnprintf(buf->arr + buf->count, (size_t)len + 1, format, args);
  va_end(args);

  buf->count += (size_t)len;
  return 0;
}
