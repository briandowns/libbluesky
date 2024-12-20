#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <jansson.h>

#include "bluesky.h"

int
main(int argc, char **argv)
{
    char *bksy_app_password = getenv("BSKY_APP_PASSWORD");
    int ret = bs_client_init("bdowns328.bsky.social", bksy_app_password, NULL);
    if (ret > 0) {
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

    // bs_client_response_t *res = bs_profile_get("bdowns328.bsky.social");
    // printf("%s\n", res->resp);
    // bs_client_response_free(res);

    // bs_client_response_t *res = bs_timeline_get(NULL);
    // printf("%s\n", res->resp);
    // bs_client_response_free(res);

    // bs_client_response_t *res = bs_profile_preferences();
    // printf("%s\n", res->resp);
    // bs_client_response_free(res);

    // const char *msg = "{\"$type\": \"app.bsky.feed.post\",
    //     \"text\": \"Another post from libbluesky #c library!\",
    //     \"createdAt\": \"2024-12-19T22:47:30.0000Z\"}";
    // bs_client_response_t *res = bs_client_post(msg);
    // printf("%s\n", res->resp);
    // bs_client_response_free(res);

    // bs_client_response_t *res = bs_client_follows_get("bdowns328.bsky.social", NULL);
    // printf("%s ", res->resp);
    // printf("%d\n", res->err_code);
    // bs_client_response_free(res);

    bs_client_response_t *res = bs_client_followers_get("bdowns328.bsky.social", NULL);
    printf("%s ", res->resp);
    printf("%d\n", res->err_code);
    bs_client_response_free(res);

    bs_client_free();

    return 0;
}
