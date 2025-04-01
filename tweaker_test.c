#include "tweaker.c"

#include <stdio.h>

int main(void)
{
	int reg;
	uint32_t value;
	while (1 == 1) {
		printf("Enter the register: ");
		scanf("%d", &reg);
		printf("Enter the number in hex: ");
		scanf("%x", &value);
		struct instruction_sequence iq = tweak(reg, 0, value);
		for (unsigned i = 0; i < iq.length; i += 1) {
			struct instruction instr = iq.instr[i];
			switch (instr.op) {
				default:
					printf("Invalid operation ");
				break;
				case OP_ADC:
					if (i == 0)
						printf("movs\t");
					else
						printf("adc\t");
				break;
				case OP_SBC:
					if (i == 0)
						printf("mvn\t");
					else
						printf("sbc\t");
				break;
			}
			printf("#0x%08x", instr.value);
			puts("");
		}
	}
	return 0;
}
