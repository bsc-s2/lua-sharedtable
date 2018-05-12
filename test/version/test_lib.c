#include <stdio.h>

#include "version/version.h"


void
show_version(void)
{
    const char *version = st_version_get_fully();

    printf("my library version number is %s\n", version);
}
