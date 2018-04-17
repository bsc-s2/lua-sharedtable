#include "inc/util.h"

void st_util_bughandler_raise() {
    raise(SIGABRT);
}

st_util_bughandler_t st_util_bughandler_ = st_util_bughandler_raise;
