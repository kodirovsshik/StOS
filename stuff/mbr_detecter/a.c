
#include <stdio.h>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "%s: Please specify a file\n", argv[0]);
		return -1;
	}

	FILE* f = fopen(argv[1], "rb");
	if (!f)
	{
		fprintf(stderr, "%s: Failed to open %s\n", argv[0], argv[1]);
		return -2;
	}

	fseek(f, 510, SEEK_SET);

	uint16_t signature = 0;
	fread(&signature, 1, 2, f);
	return signature != 0xAA55;
}
