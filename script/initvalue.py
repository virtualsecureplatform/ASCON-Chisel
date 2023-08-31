#!/bin/python3

def bytes_to_int(bytes):
    return sum([bi << ((len(bytes) - 1 - i)*8) for i, bi in enumerate(to_bytes(bytes))])
def to_bytes(l): # where l is a list or bytearray or bytes
    return bytes(bytearray(l))
def int_to_bytes(integer, nbytes):
    return to_bytes([(integer >> ((nbytes - 1 - i) * 8)) % 256 for i in range(nbytes)])
def bytes_to_state(bytes):
    return [bytes_to_int(bytes[8*w:8*(w+1)]) for w in range(5)]
def zero_bytes(n):
    return n * b"\x00"
def rotr(val, r):
    return (val >> r) | ((val & (1<<r)-1) << (64-r))
def ascon_permutation(S, rounds=1):
    """
    Ascon core permutation for the sponge construction - internal helper function.
    S: Ascon state, a list of 5 64-bit integers
    rounds: number of rounds to perform
    returns nothing, updates S
    """
    assert(rounds <= 12)
    for r in range(12-rounds, 12):
        # --- add round constants ---
        S[2] ^= (0xf0 - r*0x10 + r*0x1)
        # --- substitution layer ---
        S[0] ^= S[4]
        S[4] ^= S[3]
        S[2] ^= S[1]
        T = [(S[i] ^ 0xFFFFFFFFFFFFFFFF) & S[(i+1)%5] for i in range(5)]
        for i in range(5):
            S[i] ^= T[(i+1)%5]
        S[1] ^= S[0]
        S[0] ^= S[4]
        S[3] ^= S[2]
        S[2] ^= 0XFFFFFFFFFFFFFFFF
        # --- linear diffusion layer ---
        S[0] ^= rotr(S[0], 19) ^ rotr(S[0], 28)
        S[1] ^= rotr(S[1], 61) ^ rotr(S[1], 39)
        S[2] ^= rotr(S[2],  1) ^ rotr(S[2],  6)
        S[3] ^= rotr(S[3], 10) ^ rotr(S[3], 17)
        S[4] ^= rotr(S[4],  7) ^ rotr(S[4], 41)

tagspec = int_to_bytes(0, 4)
rate = 8
a = 12
b = 12
S = bytes_to_state(to_bytes([0, rate * 8, a, a-b]) + tagspec + zero_bytes(32))
ascon_permutation(S, 12)
print([hex(s) for s in S])
