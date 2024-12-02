#define _DEFAULT_SOURCE
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#define BS_REQ_JSON_HEADER "Accept: application/json"
#define TOKEN_HEADER_SIZE 256
#define DID_MAX_SIZE 2048
#define DEFAULT_URL_SIZE 2048

#define POST_FEED_URL 'https://bsky.social/xrpc/com.atproto.repo.createRecord'

#define SET_BASIC_CURL_CONFIG \
    curl_easy_setopt(curl, CURLOPT_URL, url); \
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); \
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk); \
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb); \
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)response);

#define CURL_CALL_ERROR_CHECK \
    if (res != CURLE_OK) { \
        char *err_msg = (char *)curl_easy_strerror(res); \
        response->err_msg = calloc(strlen(err_msg)+1, sizeof(char)); \
        strcpy(response->err_msg, err_msg); \
        CALL_CLEANUP; \
        return response; \
    }

#define CALL_CLEANUP \
    curl_slist_free_all(chunk); \
    free(url);

typedef struct {
    char *resp;
    char *err_msg;
    size_t size;
    long resp_code;
    int err_code;
} bs_client_response_t;

static CURL *curl = NULL;
static char did[DID_MAX_SIZE];
static char token_header[TOKEN_HEADER_SIZE];

/**
 * Create and return a new pointer for response data.
 */
static bs_client_response_t*
bs_client_response_new()
{
    bs_client_response_t *resp = calloc(1, sizeof(bs_client_response_t));
    return resp;
}

// HANDLE='felicitas.pojtinger.com'
// DID_URL="https://bsky.social/xrpc/com.atproto.identity.resolveHandle"
// export DID=$(curl -G \
//     --data-urlencode "handle=$HANDLE" \
//     "$DID_URL" | jq -r .did)
// {"did":"did:plc:d2pmhxz4ud7z3zwc5rejgl53"}
bs_client_response_t*
bs_resolve_did(const char *handle)
{
    bs_client_response_t *response = bs_client_response_new();
    struct curl_slist *chunk = NULL;

    chunk = curl_slist_append(chunk, token_header);
    char *escaped_handle = curl_easy_escape(curl, handle, strlen(handle));
    char *url = calloc(DEFAULT_URL_SIZE, sizeof(char));
    strcpy(url, "https://bsky.social/xrpc/com.atproto.identity.resolveHandle");
    strcat(url, "?handle=");
    strcat(url, handle);

    SET_BASIC_CURL_CONFIG;

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->resp_code);

    
    CURL_CALL_ERROR_CHECK;

    curl_free(escaped_handle);
    CALL_CLEANUP;
    
    return response;
}

static void
bs_set_did(const char *handle)
{
    bs_client_response_t *res = bs_resolve_did(handle);

    json_error_t error;
    json_t *root = json_loads(res->resp, 0, &error);

    const char *identify = {0};
    json_unpack(root, "{s:s}", "did", &did);
    json_decref(root);

    bs_client_response_free(res);

    strcpy(did, identify);

    return;
}

// API_KEY_URL='https://bsky.social/xrpc/com.atproto.server.createSession'
// POST_DATA="{ \"identifier\": \"${DID}\", \"password\": \"${APP_PASSWORD}\" }"
// export API_KEY=$(curl -X POST \
//     -H 'Content-Type: application/json' \
//     -d "$POST_DATA" \
//     "$API_KEY_URL" | jq -r .accessJwt)
bs_client_response_t*
bs_retrieve_api_key(const char *app_password)
{
    bs_client_response_t *response = bs_client_response_new();
    struct curl_slist *chunk = NULL;

    chunk = curl_slist_append(chunk, token_header);

    char *url = calloc(DEFAULT_URL_SIZE, sizeof(char));
    strcpy(url, "https://bsky.social/xrpc/com.atproto.server.createSession");

    SET_BASIC_CURL_CONFIG;

    json_t *json_obj = json_object();
    json_object_set(json_obj, "identifier", json_string(did));
    json_object_set(json_obj, "password", json_string(app_password));
    char *data = json_dumps(json_obj, 0);

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->resp_code);
    CURL_CALL_ERROR_CHECK;

    json_decref(json_obj);
    free(data);

    CALL_CLEANUP;
    
    return response;
}

int
bs_client_init(const char *handle, const char *app_password)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        return 1;
    }

    bs_client_auth(handle);

    bs_client_response_t *res = bs_retrieve_api_key(app_password);
    if (res->err_msg != NULL) {
        // 
    }

    json_t *root;
    json_error_t error;

    root = json_loads(res->resp, 0, &error);

    const char *jwt = {0};
    json_unpack(root, "{s:s}", "accessJwt", &jwt);
    printf("%s\n", jwt);
    json_decref(root);

    return 0;
}

void
bs_client_response_free(bs_client_response_t *res)
{
    if (res != NULL) {
        if (res->resp != NULL) free(res->resp);
        if (res->err_msg != NULL) free(res->err_msg);

        free(res);
        res = NULL;
    }
}

/**
 * Write a received response into a response type.
 */
static size_t
cb(char *data, size_t size, size_t nmemb, void *clientp)
{
    size_t realsize = size * nmemb;
    bs_client_response_t *mem = (bs_client_response_t*)clientp;

    char *ptr = realloc(mem->resp, mem->size + realsize+1);

    mem->resp = ptr;
    memcpy(&(mem->resp[mem->size]), data, realsize);
    mem->size += realsize;
    mem->resp[mem->size] = 0;

    return realsize;
}

int
main(int arc, char **argv)
{
    bs_client_init("bdowns328.bsky.social", "jrp5-x4lh-zotm-cmkj");
    // bs_client_response_t *res = bs_resolve_did("bdowns328.bsky.social");

    // json_t *root;
    // json_error_t error;

    // root = json_loads(res->resp, 0, &error);

    // const char *did = {0};
    // json_unpack(root, "{s:s}", "did", &did);
    // printf("%s\n", did);
    // json_decref(root);

    bs_client_response_t *res = bs_retrieve_api_key("jrp5-x4lh-zotm-cmkj");
    printf("%s\n", res->resp);
    bs_client_response_free(res);

    return 0;
}
