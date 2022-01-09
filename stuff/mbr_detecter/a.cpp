
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char** argv)
{
	const char* const argv0 = argv[0];
	bool check_for_gpt = false;

	if (argc < 2)
	{
		fprintf(stderr, "%s: Please specify a file\n", argv0);
		return -1;
	}

	if (strcmp(argv[1], "--gpt") == 0)
	{
		argc--;
		argv++;
		check_for_gpt = true;
	}

	FILE* f = fopen(argv[1], "rb");
	if (!f)
	{
		fprintf(stderr, "%s: Failed to open %s\n", argv0, argv[1]);
		return -2;
	}

	fseek(f, 510, SEEK_SET);

	uint16_t mbr_signature = 0;
	fread(&mbr_signature, 1, 2, f);
	if (mbr_signature != 0xAA55)
		return 1;

	if (!check_for_gpt)
		return 0;

	char gpt_signature[8] = { 0 };
	fread(&gpt_signature, 1, 8, f);
	if (strncmp(gpt_signature, "EFI PART", 8) != 0)
		return 1;

	return 0;
}
