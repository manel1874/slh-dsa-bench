# SLH-DSA Benchmark: FIPS 205 vs NIST SP 800-230 Parameter Sets

Benchmarks comparing the existing [FIPS 205](https://doi.org/10.6028/NIST.FIPS.205) SLH-DSA parameter sets against the six new limited-signature parameter sets proposed in [NIST SP 800-230 (Initial Public Draft, April 2026)](https://doi.org/10.6028/NIST.SP.800-230.ipd).

The new SP 800-230 parameter sets (denoted `-24`) restrict signing to **2²⁴ signatures per key** and are optimized for **sign-once, verify-many** use cases such as firmware, software, and certificate signing.

## Build & Run

```bash
make -j$(nproc)

# Run FIPS 205 variants (fast)
for v in sha2-128s sha2-128f sha2-192s sha2-192f sha2-256s sha2-256f; do
    ./bench_$v
done

# Run SP 800-230 variants (keygen/sign take ~2-6 minutes each)
for v in sha2-128-24 sha2-192-24 sha2-256-24; do
    ./bench_$v
done
```

## Benchmark Results

**Environment:** Ubuntu 22.04, GCC 11.4 (`-O3 -mavx2`), AVX2-optimized PQClean implementation (8-way parallel SHA-256, 4-way parallel SHA-512), Intel Core i7-12700H.

### Full Results

| Parameter Set | Spec | Cat | Keygen (ms) | Sign (ms) | Verify (ms) | Sig (B) | PK (B) | SK (B) |
|---|---|---|---:|---:|---:|---:|---:|---:|
| SLH-DSA-SHA2-128s | FIPS 205 | 1 | 23.9 | 220.9 | 0.343 | 7,856 | 32 | 64 |
| SLH-DSA-SHA2-128f | FIPS 205 | 1 | 0.4 | 9.6 | 0.742 | 17,088 | 32 | 64 |
| **SLH-DSA-SHA2-128-24** | **SP 800-230** | **1** | **149,531** | **200,093** | **0.105** | **3,856** | **32** | **64** |
| SLH-DSA-SHA2-192s | FIPS 205 | 3 | 34.8 | 375.3 | 0.460 | 16,224 | 48 | 96 |
| SLH-DSA-SHA2-192f | FIPS 205 | 3 | 0.6 | 16.1 | 1.452 | 35,664 | 48 | 96 |
| **SLH-DSA-SHA2-192-24** | **SP 800-230** | **3** | **148,992** | **295,700** | **0.188** | **7,752** | **48** | **96** |
| SLH-DSA-SHA2-256s | FIPS 205 | 5 | 35.3 | 359.2 | 0.623 | 29,792 | 64 | 128 |
| SLH-DSA-SHA2-256f | FIPS 205 | 5 | 3.5 | 38.9 | 1.200 | 49,856 | 64 | 128 |
| **SLH-DSA-SHA2-256-24** | **SP 800-230** | **5** | **160,716** | **333,935** | **0.163** | **14,944** | **64** | **128** |

### Key Sizes

The secret key size is **4 × n** bytes (SK.seed ∥ SK.prf ∥ PK.seed ∥ PK.root):

| Security Category | n (bytes) | SK (bytes) | PK (bytes) |
|---|---|---|---|
| 1 (128-bit) | 16 | **64** | 32 |
| 3 (192-bit) | 24 | **96** | 48 |
| 5 (256-bit) | 32 | **128** | 64 |

Key sizes are **identical** between FIPS 205 and SP 800-230 parameter sets at the same security level — only the internal tree structure differs.

### Comparison: SP 800-230 (`-24`) vs FIPS 205 (`-s`, smallest signatures)

| Metric | Level 1 | Level 3 | Level 5 |
|---|---|---|---|
| **Signature size** | 3,856 vs 7,856 B | 7,752 vs 16,224 B | 14,944 vs 29,792 B |
| **↓ Decrease** | **−50.9%** | **−52.2%** | **−49.8%** |
| **Verify time** | 0.105 vs 0.343 ms | 0.188 vs 0.460 ms | 0.163 vs 0.623 ms |
| **↓ Speedup** | **−69.4% (3.3×)** | **−59.1% (2.4×)** | **−73.8% (3.8×)** |
| Sign time | 200,093 vs 221 ms | 295,700 vs 375 ms | 333,935 vs 359 ms |
| ↑ Increase | 906× slower | 788× slower | 930× slower |
| Keygen time | 149,531 vs 24 ms | 148,992 vs 35 ms | 160,716 vs 35 ms |
| ↑ Increase | 6,252× slower | 4,286× slower | 4,559× slower |

### Comparison: SP 800-230 (`-24`) vs FIPS 205 (`-f`, fastest)

| Metric | Level 1 | Level 3 | Level 5 |
|---|---|---|---|
| **Signature size** | 3,856 vs 17,088 B | 7,752 vs 35,664 B | 14,944 vs 49,856 B |
| **↓ Decrease** | **−77.4%** | **−78.3%** | **−70.0%** |
| **Verify time** | 0.105 vs 0.742 ms | 0.188 vs 1.452 ms | 0.163 vs 1.200 ms |
| **↓ Speedup** | **−85.8% (7.1×)** | **−87.1% (7.7×)** | **−86.4% (7.4×)** |

## Why Is Signing So Slow?

The SP 800-230 parameter sets use **d = 1** (a single XMSS tree instead of a hypertree), which means:

- **Keygen** must build a Merkle tree with **2²¹ – 2²²** leaves (~2–4 million), each requiring a full WOTS+ key generation
- **Signing** must traverse this entire tree to produce an authentication path
- **Verification** only needs to check one WOTS+ signature and walk one authentication path — hence the dramatic speedup

This is the core trade-off: collapse the hypertree into a single layer to reduce signature size and verification time, at the cost of much slower keygen/sign. The spec explicitly states these are for "sign-once, verify-many" scenarios.

### Winternitz Parameter Impact

The new parameter sets also use **smaller Winternitz values** (w=4 or w=8 vs w=16 in FIPS 205):

| Variant | w | Effect |
|---|---|---|
| FIPS 205 (all) | 16 | Fewer, longer WOTS chains → smaller WOTS signatures, slower verify |
| SP 800-230 128-24 | 4 | More, shorter chains → faster verify, larger WOTS component |
| SP 800-230 192-24 | 8 | Middle ground |
| SP 800-230 256-24 | 4 | More, shorter chains → faster verify, larger WOTS component |

The net effect (combined with d=1 and adjusted FORS parameters) still produces smaller total signatures.

## Production Considerations

SP 800-230 §3 explicitly allows **caching parts or all of the hypertree** to amortize keygen cost. A production implementation would:

1. **Pre-compute and cache** the full tree at key generation time (one-time cost)
2. Use **AVX2/AVX-512** SHA-256 parallelism (already used in these benchmarks — ~3× faster than the clean reference)
3. Use **multi-threaded** tree construction for further speedup

The verification side — the critical path for end users — is already **sub-0.2ms** with AVX2.

## Project Structure

```
slh-dsa-bench/
├── common/             # SHA-2 and randombytes (from PQClean)
├── variants/           # params.h for each parameter set
│   ├── sha2-128s/      # FIPS 205
│   ├── sha2-128f/
│   ├── sha2-192s/
│   ├── sha2-192f/
│   ├── sha2-256s/
│   ├── sha2-256f/
│   ├── sha2-128-24/    # SP 800-230 (new)
│   ├── sha2-192-24/
│   └── sha2-256-24/
├── src/                # SPHINCS+ AVX2 implementation (adapted from PQClean)
├── bench.c             # Benchmark harness
└── Makefile
```

## Implementation Source

The SPHINCS+ code is taken from [PQClean](https://github.com/PQClean/PQClean)'s **AVX2-optimized** C implementation (`crypto_sign/sphincs-sha2-*-simple/avx2/`), which uses 8-way parallel SHA-256 (`sha256x8`) and 4-way parallel SHA-512 (`sha512x4`) via AVX2 intrinsics. The shared source files (WOTS+, FORS, Merkle tree, SHA-2 hashing) were copied into `src/`, and the common SHA-2 and `randombytes` utilities from `PQClean/common/` into `common/`.

For the SP 800-230 variants, new `params.h` files were created with the parameters from the NIST draft — adjusted `n`, `h`, `d=1`, Winternitz `w` (4 or 8), FORS dimensions (`k`, `a`), `lgw`, and precomputed `WOTS_LEN2`. The `hash_sha2.c` file was also patched to handle the `d=1` edge case: when `SPX_TREE_BITS = h' × (d−1) = 0`, the original code would right-shift a 64-bit value by 64 (undefined behavior in C). The fix guards this with a preprocessor conditional that sets `tree = 0` when `SPX_TREE_BITS == 0`.

## References

- [FIPS 205 — SLH-DSA](https://doi.org/10.6028/NIST.FIPS.205)
- [NIST SP 800-230 (IPD) — Additional SLH-DSA Parameter Sets](https://doi.org/10.6028/NIST.SP.800-230.ipd)
- [Fluhrer & Dang, "Smaller SPHINCS+", ePrint 2024/018](https://eprint.iacr.org/2024/018)
- [Kölbl & Philipoom, "A Note on SPHINCS+ Parameter Sets", ePrint 2022/1725](https://eprint.iacr.org/2022/1725)
- [PQClean — Clean reference implementations](https://github.com/PQClean/PQClean)
