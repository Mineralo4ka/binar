#include "coder.h"
#include "command.h"
#include <inttypes.h>

int encode_file(const char *in_file_name, const char *out_file_name)
{
	FILE *in = fopen(in_file_name, "r");
	if (in == NULL) {
		return -1;
	}
	uint32_t point;
	CodeUnits units;

	fscanf(in, "%" SCNx32, &point);
	fclose(in);

	printf("%" "x\n", point);

	if (encode(point, &units) == -1) {
		return -1;
	}

	FILE *out = fopen(out_file_name, "w");
	write_code_unit(out, &units);
	fclose(out);

	return 0;
}

int decode_file(const char *in_file_name, const char *out_file_name)
{
	FILE *in = fopen(in_file_name, "r");
	if (in == NULL) {
		return -1;
	}
	CodeUnits units;
	uint32_t point;

	if (read_next_code_unit(in, &units) == -1) {
		return -1;
	}

	point = decode(&units);
	if (point == -1) {
		return -1;
	}

	FILE *out = fopen(out_file_name, "w");
	fprintf(out, "%" "x", point);
	fclose(out);
	fclose(in);

	return 0;
}
