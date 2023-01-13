
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

template<class... Args>
void xassert(bool cond, int code, const char* fmt, Args ...args)
{
	if (!cond)
	{
		fprintf(stderr, fmt, args...);
		exit(code);
	}
}

int main(int argc, char** argv)
{
	xassert(argc == 2, -1, "%s: needs 1 argument", argv[0]);
	
	uint64_t val;
	xassert(1 == sscanf(argv[1], "%" SCNu64, &val), -1, "%s: failed to parse %s", argv[0], argv[1]);
	
	xassert(0 < fwrite(&val, 1, sizeof(val), stdout), -1, "%s: write error", argv[0]);
	return 0;
}
