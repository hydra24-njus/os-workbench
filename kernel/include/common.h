#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>


#ifdef LOCAL_MACHINE
  #define debug(...) printf(__VA_ARGS__)
#else
  #define debug(...)
#endif
