#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "params.h"
#include "nistapi.h"

#ifndef VARIANT_NAME
#define VARIANT_NAME "unknown"
#endif

static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

int main(void) {
    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[CRYPTO_SECRETKEYBYTES];
    uint8_t *sig = malloc(CRYPTO_BYTES);
    size_t siglen;
    uint8_t msg[32];
    double start, elapsed;
    int iterations;
    int rc;

    if (!sig) {
        fprintf(stderr, "Failed to allocate %d bytes for signature\n", CRYPTO_BYTES);
        return 1;
    }

    memset(msg, 0x42, sizeof(msg));

    /* Keygen benchmark: single iteration (some variants take minutes) */
    start = get_time_ms();
    crypto_sign_keypair(pk, sk);
    double keygen_ms = get_time_ms() - start;

    /* Sign benchmark: single iteration */
    start = get_time_ms();
    rc = crypto_sign_signature(sig, &siglen, msg, sizeof(msg), sk);
    if (rc != 0) {
        fprintf(stderr, "%s: sign failed\n", VARIANT_NAME);
        free(sig);
        return 1;
    }
    double sign_ms = get_time_ms() - start;

    /* Verify benchmark: run enough iterations for stable timing */
    iterations = 0;
    start = get_time_ms();
    do {
        rc = crypto_sign_verify(sig, siglen, msg, sizeof(msg), pk);
        if (rc != 0) {
            fprintf(stderr, "%s: verify failed\n", VARIANT_NAME);
            free(sig);
            return 1;
        }
        iterations++;
        elapsed = get_time_ms() - start;
    } while (iterations < 10 || elapsed < 1000.0);
    double verify_ms = elapsed / iterations;

    printf("%s,%.3f,%.3f,%.3f,%d,%d,%d\n",
           VARIANT_NAME, keygen_ms, sign_ms, verify_ms,
           CRYPTO_BYTES, CRYPTO_PUBLICKEYBYTES, CRYPTO_SECRETKEYBYTES);

    free(sig);
    return 0;
}
