#include "iotd.h"
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#define LOG_MODULE "iotd"
#define LOG_ENABLE_DBG 1
#include "log.h"

#define BING_HOST "http://www.bing.com"

struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t 
write_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        /* out of memory! */
        LOG_WARN("not enough memory (realloc returned NULL)");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int
bing_extract_info(struct MemoryStruct* mem, char** url, char** urlbase)
{
    int status = 0;

    const cJSON *images = NULL;
    const cJSON *image = NULL;

    cJSON* resp;

    resp = cJSON_ParseWithLength(mem->memory, mem->size);
    images = cJSON_GetObjectItemCaseSensitive(resp, "images");
    cJSON_ArrayForEach(image, images)
    {
        cJSON *urlobj = cJSON_GetObjectItemCaseSensitive(image, "url");
        if (!cJSON_IsString(urlobj))
        {
            status = -1;
            goto done;
        }

        *url = malloc(strlen(urlobj->valuestring)+1);
        strcpy(*url, urlobj->valuestring);

        cJSON *urlbaseobj = cJSON_GetObjectItemCaseSensitive(image, "urlbase");
        if (!cJSON_IsString(urlbaseobj))
        {
            status = -1;
            goto done;
        }

        char* dot = strchr(urlbaseobj->valuestring, '.');
        if (dot) {
            *urlbase = malloc(strlen(urlbaseobj->valuestring)+1);
            strcpy(*urlbase, (dot+1));
        }
    }

done:
    cJSON_Delete(resp);
    return status;
}

int
bing_get_info(struct MemoryStruct* chunk)
{
    CURL *curl = NULL;
    CURLcode result = CURLE_OK;

    if (!chunk) {
        LOG_WARN("NULL chunk pointer");
        return -1;
    }

    curl = curl_easy_init();
    if (!curl) {
        result = CURLE_FAILED_INIT;
        LOG_WARN("CURL easy init failed.");
        goto done;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "http://www.bing.com/HPImageArchive.aspx?format=js&idx=0&n=1&mkt=en-US");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);

    result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        LOG_WARN("%s", curl_easy_strerror(result));
        goto done;
    }

    const char *ct;
    /* ask for the content-type */
    result = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
    if (result != CURLE_OK) {
        LOG_WARN("%s", curl_easy_strerror(result));
        goto done;
    }

    if (strncmp(ct, "application/json", 16) != 0) {
        LOG_WARN("Unexpected Content-Type: %s", ct);
        result = CURLE_WEIRD_SERVER_REPLY;
        goto done;
    }

    result = CURLE_OK;

done:
    if (curl) curl_easy_cleanup(curl);

    return (int)result;
}

int
bing_get_file(const char* url, const char* dir, const char* base)
{
    CURL* curl = NULL;
    char* path = NULL;
    char* full_url = NULL;
    CURLcode result = CURLE_OK;
    FILE* fp = NULL;

    if (!url || !dir || !base) {
        LOG_WARN("NULL pointer");
        return -1;
    }

    curl = curl_easy_init();
    if (!curl) {
        result = CURLE_FAILED_INIT;
        LOG_WARN("CURL easy init failed.");
        goto done;
    }

    path = malloc(strlen(dir) + strlen(base) + 16);
    strcpy(path, dir);
    strcat(path, base);
    strcat(path, ".jpg");
    LOG_INFO("File path: %s", path);

    fp = fopen(path, "wb");
    if (!fp) {
        LOG_ERRNO("Failed to open destination file.");
        result = CURLE_FAILED_INIT;
        goto done;
    }

    full_url = malloc(strlen(BING_HOST) + strlen(url) + 1);
    strcpy(full_url, BING_HOST);
    strcat(full_url, url);
    LOG_DBG("URL: %s", full_url);
    curl_easy_setopt(curl, CURLOPT_URL, full_url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)fp);

    result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        LOG_WARN("%s", curl_easy_strerror(result));
        goto done;
    }

    result = CURLE_OK;

done:
    if (fp) fclose(fp);
    if (curl) curl_easy_cleanup(curl);
    if (path) free(path);
    if (full_url) free(full_url);

    return (int)result;
}

int
bing_iotd_get(const char *dir)
{
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
    if(result != CURLE_OK) {
        LOG_WARN("CURL global init failed.");
        goto done;
    }

    LOG_DBG("Calling bing_get_info()");
    result = bing_get_info(&chunk);
    if (result) goto done;

    char* url = NULL;
    char* urlbase = NULL;
    LOG_DBG("Calling bing_extract_info()");
    result = bing_extract_info(&chunk, &url, &urlbase);
    if (result) goto done;

    LOG_DBG("Calling bing_get_file()");
    result = bing_get_file(url, dir, urlbase);
    if (result) goto done;

done:
    if (url) free(url);
    if (urlbase) free(urlbase);
    curl_global_cleanup();
    if (chunk.memory) free(chunk.memory);

    return (int)result;
}

int
iotd_get(const service_t svc, const char *dir)
{
    switch (svc) {
        case SVC_BING:
            LOG_DBG("Calling bing_iotd_get()");
            return bing_iotd_get(dir);
        default:
            LOG_WARN("Unrecognized IOTD service.");
            return -1;
    }
}

