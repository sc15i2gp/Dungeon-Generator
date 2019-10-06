#include "rng.h"

uint64_t __default_seed = 0x12489AB3D8114;
uint64_t __seed = __default_seed;
uint64_t __modulus = (uint64_t)2e32;
uint64_t __multiplier = 1122695477;
uint64_t __increment = 1;

#if 0
void seed_rng(uint64_t seed)
{
	if(seed) __seed = seed;
	else __seed = __default_seed;
}

uint64_t rng()
{
	__seed = (__multiplier*__seed + __increment);
	return (__seed/65536) % __modulus;
}

uint64_t rng_range(uint64_t min, uint64_t max)
{
	uint64_t diff = max - min;
	uint64_t n = rng() % diff;
	return min + n;
}
#else
uint64_t rng()
{
	return rand();
}

uint64_t rng_range(uint64_t min, uint64_t max)
{
	uint64_t diff = max-min;
	uint64_t n = rand() % diff;
	return min+n;
}

void seed_rng(uint64_t seed) 
{
	srand(seed);
}
#endif

void print_rng_info()
{
	printf("RNG\n");
	printf("Seed = %lu\n", __seed);
	printf("Default seed = %lu\n", __default_seed);
	printf("Modulus = %lu\n", __modulus);
	printf("Multiplier = %lu\n", __multiplier);
	printf("Increment = %lu\n\n", __increment);
}
