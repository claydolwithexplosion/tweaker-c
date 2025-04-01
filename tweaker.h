#ifndef CAIC_H
#define CAIC_H

#include <stdint.h>

/* Enum for constants */
enum constant {
	MAX_ROT_IMMS = 4096,
	MAX_DEPTH = 8,
	TRUE = (0 == 0),
	FALSE = (0 != 0),
};

enum operation {
	OP_INVALID,
	OP_ADC,
	OP_SBC,
};

struct instruction {
	unsigned value : 32;
	enum operation op : 32;
};

struct instruction_sequence {
	struct instruction instr[8];
	unsigned length;
};

struct instruction_sequence tweak(int reg, uint32_t base, uint32_t target);

#endif
