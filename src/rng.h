#pragma once
#include <stdint.h>
#include <stdio.h>

extern uint64_t __default_seed;
extern uint64_t __seed;
extern uint64_t __modulus;
extern uint64_t __multiplier;
extern uint64_t __increment;

void seed_rng(uint64_t seed);
uint64_t rng();
uint64_t rng_range(uint64_t min, uint64_t max);
void print_rng_info();
