#ifndef SPX_NISTAPI_H
#define SPX_NISTAPI_H

#include "params.h"

#define CRYPTO_SECRETKEYBYTES SPX_SK_BYTES
#define CRYPTO_PUBLICKEYBYTES SPX_PK_BYTES
#define CRYPTO_BYTES SPX_BYTES
#define CRYPTO_SEEDBYTES (3 * SPX_N)

#define crypto_sign_keypair SPX_NAMESPACE(crypto_sign_keypair)
#define crypto_sign_seed_keypair SPX_NAMESPACE(crypto_sign_seed_keypair)
#define crypto_sign_signature SPX_NAMESPACE(crypto_sign_signature)
#define crypto_sign_verify SPX_NAMESPACE(crypto_sign_verify)
#define crypto_sign SPX_NAMESPACE(crypto_sign)
#define crypto_sign_open SPX_NAMESPACE(crypto_sign_open)
#define crypto_sign_secretkeybytes SPX_NAMESPACE(crypto_sign_secretkeybytes)
#define crypto_sign_publickeybytes SPX_NAMESPACE(crypto_sign_publickeybytes)
#define crypto_sign_bytes SPX_NAMESPACE(crypto_sign_bytes)
#define crypto_sign_seedbytes SPX_NAMESPACE(crypto_sign_seedbytes)

#endif
