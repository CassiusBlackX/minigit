#include <stdlib.h>
#include <stdarg.h>

typedef struct {
  char *arr;
  size_t count;
  size_t capacity;
} buffer;

__attribute__((format(printf, 2, 3))) 
int buf_append_printf_func(buffer *buf, const char *format, ...);
#define buf_append_printf(buf, ...) buf_append_printf_func(&(buf), __VA_ARGS__)
