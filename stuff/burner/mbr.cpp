
#include "mbr.h"


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define ERR_ARGS_COUNT 1
#define ERR_MO_MBR 2
#define ERR_FILE_DRIVE 3
#define ERR_FILE_MBR 4
#define ERR_FILE_VBR 5
#define ERR_NO_FIT_PART 6


mbr_t mbr;


void check(bool true_expr, int code, const char* fmt, ...)
{
	if (!true_expr)
	{
		va_list ap;
		va_start(ap, fmt);

		vfprintf(stderr, fmt, ap);
		fputc('\n', stderr);

		va_end(ap);
		exit(code);
	}
}

int main(int argc, char** argv)
{

	check(argc == 5, ERR_ARGS_COUNT, "Usage: %s system_drive mbr_image vbr_image partition_number", argv[0]);

	FILE* dr = fopen(argv[1], "rb+");
	FILE* fm = fopen(argv[2], "rb");
	FILE* fv = fopen(argv[3], "rb");

	check(dr, ERR_FILE_DRIVE, "Failed to open %s", argv[1]);
	check(fm, ERR_FILE_DRIVE, "Failed to open %s", argv[2]);
	check(fv, ERR_FILE_DRIVE, "Failed to open %s", argv[3]);

	int part_n;
	check(
		sscanf(argv[4], "%i", &part_n) == 1 && part_n >= 0 && part_n <= 4,
		ERR_ARG_EXTRACT_PN,
		"Invalid partition number: %s",
		argv[4]
	);

	fseek(dr, 0, SEEK_SET);
	fread(&mbr, 1, sizeof(mbr), dr);
	check(mbr.signature == 0xAA55, ERR_NO_MBR, "No valid MBR found on %s", argv[1]);

	if (part_n == 0)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (mbr.table[i].active == 0x80 && mbr.table[i].type != 0)
			{
				part_n = i + 1;
				printf("Choosing partition %i\n", part_n);
				break;
			}
		}

		check(false, ERR_NO_FIT_PART, "No situable partition found on %s", argv[1]);
	}

	const uint32_t partition_begin = mbr.table[part_n - 1].start_lba;
	const uint32_t partition_size = mbr.table[part_n - 1].count_lba;

	mbr_t mbr1; //MBR with the code to be written
	fread(&mbr1, 1, 512, fm);

	memcpy(mbr.code, mbr1.code, sizeof(mbr_t::code));

	fseek(dr, 0, SEEK_SET);
	fwrite(mbr, 1, 512, dr);

	uint8_t buffer[4096];
	while (!feof(fm))
	{
		size_t n = fread(buffer, 1, 4096, fm);
		fwrite(buffer, 1, n, dr);
	}

	check(0 == fseek(dr, 512 * partition_begin, SEEK_SET), ERR_SEEK, "Failed to seek to begin of partition %i on %s", part_n, argv[1]);

	fread(&mbr, 1, 512, dr); //it will now contain first sector of drive's nth VBR
	fread(&mbr1, 1, 512, fv); //VBR to be written

	
	return 0;
}
