
/*
Invoke:
	a.out VM_DISK VM_MNT
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>
#include <format>


using namespace std;
using namespace std::string_literals;


string pipe(string_view command, int* p_ret = nullptr)
{
	cout << command << "\n";
	FILE* pipe = popen(command.data(), "r");

	if (!pipe)
		exit(-1);
	
	string result;
	char buff[128];

	while (!feof(pipe))
	{
		auto p = fgets(buff, sizeof(buff), pipe);
		if (p == nullptr)
			break;
		size_t len = strlen(p);
		while (len && p[len - 1] == '\n')
		{	
			p[len - 1] = 0;
			--len;
		}	
		result.append(p, len);
	}

	int ret = pclose(pipe);
	if (p_ret)
		*p_ret = ret;
	return result;
}

string create_loop_device(string_view disk)
{
	return pipe(format("losetup --show -o 1MiB -f {}", disk).data());
}

#define rassert(cond, ret, fmt, ...)		\
{											\
	if (!(cond))							\
	{										\
		fprintf(stderr, fmt, __VA_ARGS__);	\
		fputc('\n', stderr);				\
		return ret;							\
	}										\
}
#define tassert(cond, excp)					\
{											\
	if (!(cond))							\
		throw (excp);						\
}

int xsystem(string_view cmd)
{
	cout << cmd << "\n";
	return system(cmd.data());
}

int main(int argc, char** argv)
{
	rassert(argc == 4, 1, "%s: 3 arguments required", argv[0]);
	string_view disk = argv[1];
	string_view mount_point = argv[2];
	string_view kernel_bin = argv[3];
	string loop_device;
	bool mounted = false;
	bool err = false;

	try
	{
		loop_device = create_loop_device(disk);
		tassert(!loop_device.empty(), "losetup failed");

		tassert(0 == xsystem(
			format("mkfs.fat -F32 {}", loop_device)
			), "mkfs.fat failed"
		);

		tassert(0 == xsystem(
			format("mount {} {}", loop_device, mount_point)
			), "mount failed"
		);
		mounted = true;

		tassert(0 == xsystem(
			format("cp {} {}/", kernel_bin, mount_point)
			), "cp failed"
		);

		//cp loader
		//create sector tables for loader and kernel by parsing FS
	}
	catch(const char* s)
	{
		cerr << s << '\n';
		err = true;
	}

	if (mounted)
		xsystem(format("umount {}", loop_device));
	if (!loop_device.empty())
		xsystem(format("losetup -d {}", loop_device));

	return (int)err;
}
