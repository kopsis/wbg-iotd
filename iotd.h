#pragma once

typedef enum service { SVC_BING } service_t;

int iotd_get(const service_t svc, const char *dir);
