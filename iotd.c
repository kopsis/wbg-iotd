#include "iotd.h"
#include <curl/curl.h>

#define LOG_MODULE "iotd"
#define LOG_ENABLE_DBG 0
#include "log.h"

int
iotd_get(const service_t svc, const char *dir)
{
    switch (svc) {
        case SVC_BING:
            return 0;
        default:
            LOG_WARN("Unrecognized IOTD service.");
            return -1;
    }
}

