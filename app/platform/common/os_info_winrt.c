#include <string.h>
#include <stdio.h>
#include "util/os_info.h"

int os_info_get(os_info_t *info) {
    memset(info, 0, sizeof(*info));
    info->name = "Windows RT";
    info->version.major = 8;
    info->version.minor = 1;
    info->version.patch = 0;
    return 0;
}
