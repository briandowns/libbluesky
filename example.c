#include <stdio.h>
#include <stdlib.h>

#include <jansson.h>

#include "bluesky.h"

int
main(int argc, char **argv)
{
    int ret = bs_client_init("bdowns328.bsky.social", "jrp5-x4lh-zotm-cmkj");
    if (ret == 1) {
        fprintf(stderr, "failed to login to bluesky\n");
        return 1;
    }

    // bs_client_response_t *res = bs_resolve_did("bdowns328.bsky.social");

    // json_t *root;
    // json_error_t error;

    // root = json_loads(res->resp, 0, &error);

    // const char *did = {0};
    // json_unpack(root, "{s:s}", "did", &did);
    // printf("%s\n", did);
    // json_decref(root);

    bs_client_response_t *res = bs_profile_get("bdowns328.bsky.social");
    printf("%s\n", res->resp);
    bs_client_response_free(res);

    bs_client_free();

    return 0;
}
