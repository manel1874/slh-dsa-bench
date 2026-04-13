#!/bin/bash
set -e

VARIANTS="sha2-128s sha2-128f sha2-192s sha2-192f sha2-256s sha2-256f sha2-128-24 sha2-192-24 sha2-256-24"

echo "variant,keygen_ms,sign_ms,verify_ms,sig_bytes,pk_bytes,sk_bytes"

for v in $VARIANTS; do
    if [ ! -f "./bench_${v}" ]; then
        echo "ERROR: bench_${v} not found. Run 'make' first." >&2
        exit 1
    fi
    ./bench_${v}
done
