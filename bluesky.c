/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024 Brian J. Downs
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <jansson.h>

#include "bluesky.h"

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

static CURL *curl = NULL;
static char did[DID_MAX_SIZE];
static char token_header[TOKEN_HEADER_SIZE];

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

/**
 * Create and return a new pointer for response data.
 */
static bs_client_response_t*
bs_client_response_new()
{
    bs_client_response_t *resp = calloc(1, sizeof(bs_client_response_t));
    return resp;
}

// API_KEY_URL='https://bsky.social/xrpc/com.atproto.server.createSession'
// POST_DATA="{ \"identifier\": \"${DID}\", \"password\": \"${APP_PASSWORD}\" }"
// export API_KEY=$(curl -X POST 
//     -H 'Content-Type: application/json' 
//     -d "$POST_DATA" 
//     "$API_KEY_URL" | jq -r .accessJwt)
bs_client_response_t*
bs_client_authenticate(const char *handle, const char *app_password)
{
    bs_client_response_t *response = bs_client_response_new();
    struct curl_slist *chunk = NULL;

    chunk = curl_slist_append(chunk, token_header);

    char *url = calloc(DEFAULT_URL_SIZE, sizeof(char));
    strcpy(url, "https://bsky.social/xrpc/com.atproto.server.createSession");

    json_t *json_obj = json_object();
    json_object_set(json_obj, "identifier", json_string(handle));
    json_object_set(json_obj, "password", json_string(app_password));
    char *data = json_dumps(json_obj, 0);

    SET_BASIC_CURL_CONFIG;
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->resp_code);
    if (res != CURLE_OK) {
        char *err_msg = (char *)curl_easy_strerror(res);
        response->err_msg = calloc(strlen(err_msg)+1, sizeof(char));
        strcpy(response->err_msg, err_msg);
        CALL_CLEANUP;
        return response;
    }

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

    bs_client_response_t *res = bs_client_authenticate(handle, app_password);
    if (res->err_msg != NULL) {
        printf("%s\n", res->err_msg);
        bs_client_response_free(res);
        return 1;
    }
    printf("%s\n", res->resp);

    json_t *root;
    json_error_t error;

    root = json_loads(res->resp, 0, &error);

    const char *token = {0};
    const char *refreshToken = {0};
    json_unpack(root, "{s:s, s:s}", "accessJwt", &token,
                      "refreshJwt", refreshToken);
    printf("%s - %s\n", token, refreshToken);
    json_decref(root);

    bs_client_response_free(res);

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

// HANDLE='felicitas.pojtinger.com'
// DID_URL="https://bsky.social/xrpc/com.atproto.identity.resolveHandle"
// export DID=$(curl -G 
//     --data-urlencode "handle=$HANDLE" 
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
    if (res != CURLE_OK) {
        char *err_msg = (char *)curl_easy_strerror(res);
        response->err_msg = calloc(strlen(err_msg)+1, sizeof(char));
        strcpy(response->err_msg, err_msg);
        CALL_CLEANUP;
        return response;
    }

    curl_free(escaped_handle);
    CALL_CLEANUP;
    
    return response;
}

// static void
// bs_set_did(const char *handle)
// {
//     bs_client_response_t *res = bs_resolve_did(handle);

//     json_error_t error;
//     json_t *root = json_loads(res->resp, 0, &error);

//     const char *identify = {0};
//     json_unpack(root, "{s:s}", "did", &did);
//     json_decref(root);

//     bs_client_response_free(res);

//     strcpy(did, identify);

//     return;
// }

bs_client_response_t*
bs_profile_get(const char *handle)
{
    bs_client_response_t *response = bs_client_response_new();
    struct curl_slist *chunk = NULL;

    chunk = curl_slist_append(chunk, token_header);
    char *escaped_handle = curl_easy_escape(curl, handle, strlen(handle));
    char *url = calloc(DEFAULT_URL_SIZE, sizeof(char));
    strcpy(url, "https://bsky.social/xrpc/app.bsky.actor.getProfile");
    strcat(url, "?actor=");
    strcat(url, handle);

    SET_BASIC_CURL_CONFIG;

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->resp_code);
    if (res != CURLE_OK) {
        char *err_msg = (char *)curl_easy_strerror(res);
        response->err_msg = calloc(strlen(err_msg)+1, sizeof(char));
        strcpy(response->err_msg, err_msg);
        CALL_CLEANUP;
        return response;
    }

    curl_free(escaped_handle);
    CALL_CLEANUP;
    
    return response;   
}

void
bs_client_free()
{
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}
