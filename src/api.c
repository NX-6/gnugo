#include "emscripten.h"
#include "gtp.h"

int EMSCRIPTEN_KEEPALIVE
post_gtp(char line[]) {
  return gtp_handle_line(line);
}
