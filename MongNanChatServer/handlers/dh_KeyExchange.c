#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <memory.h>
#include <inttypes.h>

#ifdef _MSC_VER
#define INLINE __inline
#else
#define INLINE inline
#endif

#ifndef DIFFIE_HELLMAN_EXCHANGE_H
#define DIFFIE_HELLMAN_EXCHANGE_H

#define DH_KEY_LENGTH	(16)

typedef unsigned char DH_KEY[DH_KEY_LENGTH];

void DH_generate_key_pair(DH_KEY public_key, DH_KEY private_key);
void DH_generate_key_secret(DH_KEY secret_key, const DH_KEY my_private, const DH_KEY another_public);

#endif



/*--------------------------------------------------------------------------*/
typedef union _uint128_t {
	struct {
		uint64_t low;
		uint64_t high;
	};
	unsigned char byte[DH_KEY_LENGTH];
} uint128_t;

/* P =  2^128-159 = 0xffffffffffffffffffffffffffffff61 (The biggest 64bit prime) */
static const uint128_t P = { 0xffffffffffffff61ULL, 0xffffffffffffffffULL };
static const uint128_t INVERT_P = { 159 };
static const uint128_t G = { 5 };

/*--------------------------------------------------------------------------*/
static void INLINE
_u128_make(uint128_t* dq, const DH_KEY key) {
	memcpy(dq->byte, key, DH_KEY_LENGTH);
}

/*--------------------------------------------------------------------------*/
static int INLINE
_u128_is_zero(const uint128_t dq) {
	return (dq.low == 0 && dq.high == 0);
}

/*--------------------------------------------------------------------------*/
static int INLINE
_u128_is_odd(const uint128_t dq) {
	return (dq.low & 1);
}

/*--------------------------------------------------------------------------*/
static void INLINE 
_u128_lshift(uint128_t* dq) {
	uint64_t t = (dq->low >> 63) & 1;
	dq->high = (dq->high << 1) | t;
	dq->low = dq->low << 1;
}

/*--------------------------------------------------------------------------*/
static void INLINE
_u128_rshift(uint128_t* dq) {
	uint64_t t = (dq->high & 1) << 63;
	dq->high = dq->high >> 1;
	dq->low = (dq->low >> 1) | t;
}

/*--------------------------------------------------------------------------*/
static int INLINE
_u128_compare(const uint128_t a, const uint128_t b) {
	if (a.high > b.high) return 1;
	else if (a.high == b.high) {
		if (a.low > b.low) return 1;
		else if (a.low == b.low) return 0;
		else return -1;
	} else 
		return -1;
}

/*--------------------------------------------------------------------------*/
static void INLINE
_u128_add(uint128_t* r, const uint128_t a, const uint128_t b) {
	uint64_t overflow = 0;
	uint64_t low = a.low + b.low;
	if (low < a.low || low < b.low) {
		overflow = 1;
	}

	r->low = low;
	r->high = a.high + b.high + overflow;
}

/*--------------------------------------------------------------------------*/
static void INLINE
_u128_add_i(uint128_t* r, const uint128_t a, const uint64_t b) {
	uint64_t overflow = 0;
	uint64_t low = a.low + b;
	if (low < a.low || low < b) {
		overflow = 1;
	}

	r->low = low;
	r->high = a.high + overflow;
}

/*--------------------------------------------------------------------------*/
static void INLINE
_u128_sub(uint128_t* r, const uint128_t a, const uint128_t b) {
	uint128_t invert_b;
	invert_b.low = ~b.low;
	invert_b.high = ~b.high;
	_u128_add_i(&invert_b, invert_b, 1);
	_u128_add(r, a, invert_b);
}

/*--------------------------------------------------------------------------*/
/* r = a*b mod P */
static void
_mulmodp(uint128_t* r, uint128_t a, uint128_t b)
{
	uint128_t t;
	uint128_t double_a;
	uint128_t P_a;

	r->low = r->high = 0;
	while (!_u128_is_zero(b)) {
		if (_u128_is_odd(b)) {
			_u128_sub(&t, P, a);

			if (_u128_compare(*r, t) >= 0) {
				_u128_sub(r, *r, t);
			}
			else {
				_u128_add(r, *r, a);
			}
		}
		double_a = a;
		_u128_lshift(&double_a);

		_u128_sub(&P_a, P, a);

		if (_u128_compare(a, P_a) >= 0) {
			_u128_add(&a, double_a, INVERT_P);
		}
		else {
			a = double_a;
		}
		_u128_rshift(&b);
	}
}

/*--------------------------------------------------------------------------*/
/* r = a^b mod P (reduce) */
static void
_powmodp_r(uint128_t* r, const uint128_t a, const uint128_t b)
{
	uint128_t t;
	uint128_t half_b = b;

	if (b.high == 0 && b.low == 1) {
		*r = a;
		return;
	}

	_u128_rshift(&half_b);

	_powmodp_r(&t, a, half_b);
	_mulmodp(&t, t, t);

	if (_u128_is_odd(b)) {
		_mulmodp(&t, t, a);
	}
	*r = t;
}

/*--------------------------------------------------------------------------*/
/* r = a^b mod P */
static void 
_powmodp(uint128_t* r, uint128_t a, uint128_t b)
{
	if (_u128_compare(a, P)>0)
		_u128_sub(&a, a, P);

	_powmodp_r(r, a, b);
}

/*--------------------------------------------------------------------------*/
void DH_generate_key_pair(DH_KEY public_key, DH_KEY private_key)
{
	uint128_t private_k;
	uint128_t public_k;

	/* generate random private key */
	int i;
	for (i = 0; i < DH_KEY_LENGTH; i++) {
		private_key[i] = rand() & 0xFF;
	}

	/* pub_key = G^prv_key mod P*/
	_u128_make(&private_k, private_key);
	_powmodp(&public_k, G, private_k);

	memcpy(public_key, public_k.byte, DH_KEY_LENGTH);
}

/*--------------------------------------------------------------------------*/
void
DH_generate_key_secret(DH_KEY secret_key, const DH_KEY my_private, const DH_KEY another_public)
{
	uint128_t private_k;
	uint128_t another_k;
	uint128_t secret_k;

	_u128_make(&private_k, my_private);
	_u128_make(&another_k, another_public);

	/* secret_key = other_key^prv_key mod P*/
	_powmodp(&secret_k, another_k, private_k);

	memcpy(secret_key, secret_k.byte, DH_KEY_LENGTH);
}



/*--------------------------------------------------------------------------*/
static void
_print_key(const char* name, const DH_KEY key) {
	int i;

	printf("%s=\t", name);
	for (i = DH_KEY_LENGTH-1; i>=0; i--) {
		printf("%02x", key[i]);
	}
	printf("\n");
}

/*--------------------------------------------------------------------------*/
int 
main(int argc, char* argv[])
{
	DH_KEY alice_private, bob_private;
	DH_KEY alice_public, bob_public;
	DH_KEY alice_secret, bob_secret;

	time_t seed;
	time(&seed);
	srand((unsigned int)seed);

	/*Alice generate her private key and public key */
	DH_generate_key_pair(alice_public, alice_private);

	/*Bob generate his private key and public key */
	DH_generate_key_pair(bob_public, bob_private);

	/*Bob send his public key to Alice, Alice generate the secret key */
	DH_generate_key_secret(alice_secret, alice_private, bob_public);

	/*Alice send her public key to Bob, Bob generate the secret key */
	DH_generate_key_secret(bob_secret, bob_private, alice_public);

	_print_key("alice_private", alice_private);
	_print_key("alice_public", alice_public);
	_print_key("bob_private", bob_private);
	_print_key("bob_public", bob_public);
	_print_key("alice_secret", alice_secret);
	_print_key("bob_secret", bob_secret);

	if (memcmp(alice_secret, bob_secret, DH_KEY_LENGTH) != 0) {
		printf("ERROR!\n");
		return 1;
	}

	return 0;
}
