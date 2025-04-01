#include "tweaker.h"

static uint32_t base_imms[] = {
	0x00,
	0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB8,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,
	// 0xFF,
};
static struct cache {
	uint32_t values[MAX_ROT_IMMS];
	unsigned length;
	unsigned initialized;
} rot_imms[16];

static int approx(int depth, struct instruction_sequence *seq, uint32_t remainder, uint32_t *imms, int imms_len, enum operation op)
{
	if (depth < 0 || op == OP_INVALID)
		return FALSE;
	uint32_t incr = (op == OP_SBC ? 1 : (op == OP_ADC ? 0 : -1));

	/* Replaces remove_while */
	int u = 1, v = imms_len - 1;
	/*while (u < imms_len && imms[u] + incr > remainder)
		u += 1;*/
	while (u <= v) {
		unsigned w = u + (v - u) / 2;
		if (imms[w] + incr == remainder) {
			break;
		} else if (imms[w] + incr > remainder) {
			u = w + 1;
		} else if (imms[w] + incr < remainder) {
			v = w - 1;
		}
	}
	if (u >= imms_len)
		return FALSE;

	/* Check if we reached the goal */
	if (remainder - (imms[u] + incr)) {
		/* Try to generate the remainder additively */
		if (approx(depth - 1, seq, remainder - (imms[u] + incr), imms + u, imms_len - u, op)) {
			seq->instr[depth] = (struct instruction){imms[u], op};
			return TRUE;
		}
		/* Try to generate the remainder subtractively */
		else if (approx(depth - 1, seq, (imms[u-1] + incr) - remainder, imms + (u-1), imms_len - (u-1), op == OP_ADC ? OP_SBC : OP_ADC)) {
			seq->instr[depth] = (struct instruction){imms[u-1], op};
			return TRUE;
		}
		/* None worked, this is a dead end */
		else {
			return FALSE;
		}
	}
	/* We found it! */
	else {
		seq->instr[depth] = (struct instruction){imms[u], op};
		return TRUE;
	}
}

struct instruction_sequence tweak(int reg, uint32_t base, uint32_t target)
{
	struct instruction_sequence res = {{}, 0};
	struct cache *cache = &rot_imms[reg];

	if (!cache->initialized) {
		for (unsigned u = 0; u < sizeof(base_imms) / sizeof(*base_imms); u += 1) {
			/* Search for valid rotations given our register */
			int32_t cand = base_imms[u];
			if ((cand >> 4) == reg) {
				int32_t rotate = (cand & 0x0f) * 2;
				for (unsigned v = 0; v < sizeof(base_imms) / sizeof(*base_imms); v += 1) {
					uint32_t base = base_imms[v];
					uint32_t rotated = (base >> rotate) | (base << (32 - rotate));
					if (cache->length < MAX_ROT_IMMS) {
						cache->values[cache->length] = rotated;
						cache->length += 1;
					} else {
						/* Something must have seriously gone wrong, please increase MAX_ROT_IMMS */
						return res;
					}
				}
			}
		}
		/* Dumb sort */
		for (unsigned i = 0; i < cache->length; i += 1) {
			for (unsigned j = 0; j < cache->length; j += 1) {
				if (cache->values[i] > cache->values[j]) {
					/* Swap */
					uint32_t tmp = cache->values[i];
					cache->values[i] = cache->values[j];
					cache->values[j] = tmp;
				}
			}
		}
		cache->initialized = TRUE;
	}
	/* Greedily search for a solution */
	for (int i = 0; i < MAX_DEPTH; i += 1) {
		res.length = i + 1;
		for (unsigned u = 0; u < cache->length; u += 1) {
			uint32_t imm = cache->values[u];
			res.instr[i] = (struct instruction) {imm, OP_ADC};
			if (approx(i - 1, &res, (target - base) - imm, cache->values, cache->length, OP_ADC))
				goto exit;
			res.instr[i] = (struct instruction) {imm, OP_SBC};
			if (approx(i - 1, &res, (base - target) - (imm+1), cache->values, cache->length, OP_SBC))
				goto exit;
		}
	}
exit:
	/* Reverse the instructions */
	for (unsigned u = 0; u < res.length / 2; u += 1) {
		struct instruction tmp = res.instr[u];
		unsigned v = res.length - u;
		res.instr[u] = res.instr[v - 1];
		res.instr[v - 1] = tmp;
	}
	return res;
}
