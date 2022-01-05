
#include <stdio.h>
#include <stdint.h>


#define ERR_ARGS_COUNT 1
#define ERR_DRIVE_OPEN 2
#define ERR_DRIVE_SIZE 3
#define ERR_ARG_EXTRACT_NUMBER 4
#define ERR_ARG_EXTRACT_TYPE 5
#define ERR_ARG_EXTRACT_BEGIN 6
#define ERR_ARG_EXTRACT_END 7
#define ERR_RANGE 8
#define ERR_ARG_EXTRACT_ACTIVE 9


int main(int argc, char** argv)
{
	if (argc != 7)
	{
		fprintf(stderr, "Usage: %s drive N type first count active\n", argv[0]);
		return ERR_ARGS_COUNT;
	}

	FILE* fd = fopen(argv[1], "rb+");
	if (!fd)
		return ERR_DRIVE_OPEN;

	fseek(fd, 0, SEEK_END);
	const size_t fsize = ftell(fd);

	const long long total_sectors = fsize / 512;
	const long long first_valid_sector = 1;
	const long long last_valid_sector = total_sectors - 1;

	int num;
	if (sscanf(argv[2], "%i", &num) != 1 || num < 1 || num > 4)
		return ERR_ARG_EXTRACT_NUMBER;

	int type;
	if (sscanf(argv[3], "%i", &type) != 1 || type < 0 || type > 0xFF)
		return ERR_ARG_EXTRACT_TYPE;

	long long pbegin;
	if (sscanf(argv[4], "%lli", &pbegin) != 1 || pbegin < -1)
		return ERR_ARG_EXTRACT_BEGIN;

	long long pcount, pend;
	if (sscanf(argv[5], "%lli", &pcount) != 1 || pcount < -1)
		return ERR_ARG_EXTRACT_END;

	int active;
	if (sscanf(argv[6], "%i", &active) != 1 || active < -1 || active > 0xFF)
		return ERR_ARG_EXTRACT_ACTIVE;


	if (pbegin == -1)
		pbegin = 2048;

	if (pcount == -1)
		pcount = last_valid_sector + 1 - pbegin;

	pend = pcount - 1 + pbegin;

	if (type && first_valid_sector > last_valid_sector)
		return ERR_DRIVE_SIZE;

	if (type && pbegin < first_valid_sector)
		return ERR_ARG_EXTRACT_BEGIN;

	if (type && pbegin > last_valid_sector)
		return ERR_ARG_EXTRACT_BEGIN;

	if (type && pend > last_valid_sector)
		return ERR_ARG_EXTRACT_END;

	if (type && pend < pbegin)
		return ERR_RANGE;



	fseek(fd, 0, SEEK_SET);

	uint8_t mbr[512];
	fread(mbr, 1, 512, fd);

	int offset = 0x1BE + 16 * (num - 1);

	if (type == -1)
		type = mbr[offset + 4];

	mbr[offset++] = active;
	if (type == 0xEE)
	{
		mbr[offset++] = 0;
		mbr[offset++] = 0;
		mbr[offset++] = 0;
	}
	else
	{
		mbr[offset++] = type ? 0xFE : 0;
		mbr[offset++] = type ? 0xFF : 0;
		mbr[offset++] = type ? 0xFF : 0;
	}
	mbr[offset++] = type;
	mbr[offset++] = type ? 0xFE : 0;
	mbr[offset++] = type ? 0xFF : 0;
	mbr[offset++] = type ? 0xFF : 0;

	*(uint32_t*)&mbr[offset + 0] = pbegin;
	*(uint32_t*)&mbr[offset + 4] = pcount;

	*(uint16_t*)&mbr[510] = 0xAA55;

	fseek(fd, 0, SEEK_SET);
	fwrite(mbr, 1, 512, fd);

	return 0;
}
