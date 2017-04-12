#include "coder.h"
#include "command.h"
#include <inttypes.h>
#include <stdio.h>

int encode(uint32_t point, CodeUnits *units)
{
	int code_128 = 128, code_63 = 63;

	if (point <= 0x7F) {
		units->length = 1;
		units->code[0] = point;
		return 0;
	} else if (point <= 0x7FF) {
		units->length = 2;
	} else if (point <= 0xFFFF) {
		units->length = 3;
	} else if (point <= 0x1FFFFF){
		units->length = 4;
	} else {
		return -1;
	}

	units->code[0] = code_128;
	for (size_t i = 1; i < units->length; i++) {
		units->code[0] = units->code[0] | (code_128 >> i);
	}
	units->code[0] = units->code[0] | (point >> 6 * (units->length - 1));

	for (size_t i = 1; i < units->length; i++) {
		units->code[i] = code_128 | ((point >> (6 * (units->length - i - 1))) & code_63);
	}

	return 0;
}

uint32_t decode(const CodeUnits *units)
{
	uint32_t point = 0;

	if (units->length == 1) {
		point = units->code[0];
	} else if (units->length == 2) {
		point = units->code[0] & 0x3f;
	} else if (units->length == 3) {
		point = units->code[0] & 0x1f;
	} else if (units->length == 4) {
		point = units->code[0] & 0x0f;
	} else {
		return -1;
	}

	for (size_t i = 1; i < units->length; i++) {
		point = (point << 6) | (units->code[i] & 0x7f);
	}

	return point;
}

int read_next_code_unit(FILE *in, CodeUnits *units)
{
	while (!feof(in)) {
		fread(&units->code[0], 1, 1, in);
		if (units->code[0] <= 0x7f) {
			units->length = 1;
		} else if (units->code[0] <= 0xdf) {
			units->length = 2;
		} else if (units->code[0] <= 0xef) {
			units->length = 3;
		} else if (units->code[0] <= 0xf7) {
			units->length = 4;
		} else {
			continue;
		}

		for (size_t i = 1; i < units->length; i++) {
			fread(&units->code[i], 1, 1, in);

			if ((units->code[i] & 0xc0) != 0x80) {
				fseek(in, -1, SEEK_CUR);
				continue;
			}
			if (i != units->length - 1) {
				if (feof(in) != 0) {
					return -1;
				}
			}
		}

		return 0;
	}

	return -1;
}

int write_code_unit(FILE *out, const CodeUnits *units)
{
	fwrite(units->code, units->length, sizeof(units->code), out);

	return 0;
}
