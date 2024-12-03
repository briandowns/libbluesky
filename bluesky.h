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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __BLUESKY_H
#define __BLUESKY_H

#include <stdlib.h>

/**
 * Default response structure returned for each call to the API. Contains the
 * API response, the response code, response size, any error code, and message
 * if applicable.
 */
typedef struct {
    char *resp;
    char *err_msg;
    size_t size;
    long resp_code;
    int err_code;
} bs_client_response_t;

/**
 * Initialize the library.
 */
int
bs_client_init(const char *handle, const char *app_password);

/**
 * Free the memory used in the client response.
 */
void
bs_client_response_free(bs_client_response_t *res);

bs_client_response_t*
bs_resolve_did(const char *handle);

bs_client_response_t*
bs_retrieve_api_key(const char *app_password);

bs_client_response_t*
bs_profile_get(const char *handle);

/**
 * Free the memory used by the client.
 */
void
bs_client_free();

#endif /* __BLUESKY_H */
#ifdef __cplusplus
}
#endif
