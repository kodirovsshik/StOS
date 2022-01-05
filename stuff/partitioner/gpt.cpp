#if 0
	  This file is a part of StOS project - a small operating system
	  	made for learning purposes
	  Copyright (C) 2021 Egorov Stanislav, kodirovsshik@mail.ru, kodirovsshik@gmail.com

	  This program is free software: you can redistribute it and/or modify
	  it under the terms of the GNU General Public License as published by
	  the Free Software Foundation, either version 3 of the License, or
	  (at your option) any later version.

	  This program is distributed in the hope that it will be useful,
	  but WITHOUT ANY WARRANTY; without even the implied warranty of
	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	  GNU General Public License for more details.

	  You should have received a copy of the GNU General Public License
	  along with this program.  If not, see <https://www.gnu.org/licenses/>.
#endif



#include "mbr.h"
#include "gpt.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <locale.h>
#include <uchar.h>
#include <inttypes.h>

#include <chrono>
#include <utility>
#include <iterator>



void check(bool true_expr, int code, const char* fmt = "", ...)
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



const char* argv0 = nullptr;
const char* argv1 = nullptr;

const char* usage_table =
R"(table command - create new GPT:
	%s path/to/disk table N_entries
)";

const char* usage_create =
R"(create command - create new GPT entry:
	%s path/to/disk create N first last
)";

const char* usage_set =
R"(set command - set a GPT entry data:
	%s path/to/disk set N X Y
	Action: GPT[N].X = Y
)";

const char* usage_delete =
R"(delete command - delete an existing GPT entry:
	%s path/to/disk delete N
)";

const char* usage_zero =
R"(zero command - zero out GPT entry:
	%s path/to/disk zero N
)";

const char* usage_wipe =
R"(wipe command - wipe out entire GPT:
	%s path/to/disk wipe [-y]
)";

void usage()
{
	printf("Usage:\n");
	printf(usage_table, argv0);
	printf(usage_create, argv0);
	printf(usage_set, argv0);
	printf(usage_delete, argv0);
	printf(usage_zero, argv0);
	printf(usage_wipe, argv0);
}



#define ERR_INTERNAL 1
#define ERR_ARGS_COUNT 2
#define ERR_OPEN 3
#define ERR_READ 4
#define ERR_WRITE 5
#define ERR_SEEK 6
#define ERR_ARGS 7



extern "C"
{
	uint32_t crc32_init();
	uint32_t crc32(const void*, size_t);
	uint32_t crc32_update(const void*, size_t, uint32_t);
}


FILE* disk_f = nullptr;


void update_gpt(bool primary)
{
	gpt_header_t header;

	uint64_t file_size, file_sectors;
	fseek(disk_f, 0, SEEK_END);
	file_size = (uint64_t)ftell(disk_f);
	file_sectors = file_size / 512;

	const uint64_t headers_lba[2] = { file_sectors - 1, 1 };

	//fseek(disk_f, headers_lba[primary] * 512, SEEK_SET);
	//Always take the first header to update the second one
	fseek(disk_f, 512, SEEK_SET);
	check(fread(&header, 1, 512, disk_f) == 512, ERR_READ, "Read error on %s", argv1);

	uint32_t partition_table_size = header.partition_table_size;
	uint32_t partition_table_sectors = partition_table_size / 4 + bool(partition_table_size % 4);

	const uint64_t tables_lba[2] = { file_sectors - 1 - partition_table_sectors, 2 };

	memcpy(header.signature, "EFI PART", 8);
	header.revision = 0x00010000;
	header.header_size = sizeof(header) - sizeof(header.padding);
	header.header_crc = 0;
	header.reserved = 0;
	header.header_current_lba = headers_lba[primary];
	header.header_reserved_lba = headers_lba[!primary];
	header.partition_table_begin = tables_lba[primary];
	header.partition_table_entry_size = sizeof(gpt_entry_t);
	static_assert(sizeof(gpt_entry_t) == 128);
	header.usable_lba_begin = 2048;
	header.usable_lba_end = file_sectors - (2 + partition_table_sectors);

	fseek(disk_f, tables_lba[primary] * 512, SEEK_SET);
	gpt_entry_t table[4];
	uint32_t partition_table_crc = crc32_init();
	for (uint32_t i = 0; i < partition_table_size; ++i)
	{
		if ((i % 4) == 0)
		{
			check(fread(&table, 1, 512, disk_f) == 512, ERR_READ, "Read error on %s", argv1);
		}
		partition_table_crc = crc32_update(&table[i % 4], 128, partition_table_crc);
	}

	header.partition_table_crc = partition_table_crc;
	header.header_crc = crc32(&header, header.header_size);

	fseek(disk_f, headers_lba[primary] * 512, SEEK_SET);
	check(fwrite(&header, 1, 512, disk_f) == 512, ERR_WRITE, "Write error on %s", argv1);
	fflush(disk_f);
}

void duplicate_gpt_array()
{
	gpt_header_t header;
	fseek(disk_f, 512, SEEK_SET);
	check(fread(&header, 1, 512, disk_f) == 512, ERR_READ, "Read error on %s", argv1);

	uint64_t file_size, file_sectors;
	fseek(disk_f, 0, SEEK_END);
	file_size = (uint64_t)ftell(disk_f);
	file_sectors = file_size / 512;

	uint32_t n = header.partition_table_size;
	n = n / 4 + bool(n % 4);
	uint64_t src = 2;
	uint64_t dst = file_sectors - 1 - n;
	src *= 512; dst *= 512;

	gpt_entry_t table[4];
	for (uint32_t i = 0; i < n; ++i)
	{
		fseek(disk_f, src, SEEK_SET);
		check(fread(&table, 1, 512, disk_f) == 512, ERR_READ, "Read error on %s", argv1);
		src += 512;

		fseek(disk_f, dst, SEEK_SET);
		check(fwrite(&table, 1, 512, disk_f) == 512, ERR_WRITE, "Write error on %s", argv1);
		dst += 512;

		fflush(disk_f);
	}
}

void update_gpt()
{
	fflush(disk_f);
	update_gpt(true);
	duplicate_gpt_array();
	update_gpt(false);
}

void create_protective_mbr()
{
	fseek(disk_f, 0, SEEK_END);
	long file_size = ftell(disk_f);

	vbr_t vbr;
	mbr_t& mbr = *(mbr_t*)&vbr;
	auto& pmbre = mbr.table[0];

	memset(&mbr, 0, sizeof(mbr));

	pmbre.start_chs.sec = 2;
	pmbre.end_chs.head = 0xFF;
	pmbre.end_chs.sec = 63;
	pmbre.end_chs.cyl = 1023;
	pmbre.start_lba = 1;
	pmbre.count_lba = file_size / 512 - 1;
	pmbre.type = 0xEE;

	memcpy(vbr.code1, "\xeb\x58\x90", 3);
	memcpy(vbr.code1 + 90, "\xcd\x18", 2);
	mbr.signature = 0xAA55;

	fseek(disk_f, 0, SEEK_SET);
	check(fwrite(&mbr, 1, 512, disk_f) == 512, ERR_WRITE, "Write error on %s", argv1);
	fflush(disk_f);
}

int command_table(int argc, const char* const* argv)
{
	check(argc == 1, ERR_ARGS_COUNT, usage_table, argv0);

	uint32_t n;
	check(sscanf(argv[0], "%" PRIu32, &n) == 1 && n >= 128, ERR_ARGS, "Invalid partitions count: %s", argv[0]);

	create_protective_mbr();

	gpt_header_t hdr;
	memset(&hdr, 0, sizeof(hdr));
	for (char& x : hdr.guid)
		x = rand();
	hdr.partition_table_size = n;

	fseek(disk_f, 512, SEEK_SET);
	check(fwrite(&hdr, 1, 512, disk_f) == 512, ERR_WRITE, "Write error on %s", argv1);
	fflush(disk_f);

	memset(&hdr, 0, sizeof(hdr));
	n = n / 4 + bool(n % 4);
	fseek(disk_f, 1024, SEEK_SET);
	for (uint32_t i = 0; i < n; ++i)
	{
		check(fwrite(&hdr, 1, 512, disk_f) == 512, ERR_WRITE, "Write error on %s", argv1);
	}

	fflush(disk_f);
	update_gpt();
	return 0;
}

uint64_t get_partition_table_offset(uint32_t partition_number)
{
	gpt_header_t hdr;
	fseek(disk_f, 512, SEEK_SET);
	check(fread(&hdr, 1, 512, disk_f) == 512, ERR_READ, "Read error on %s", argv1);

	return 512 * (hdr.partition_table_begin + partition_number / 4);
}

int command_create(int argc, const char* const* argv)
{
	if (argc != 3)
	{
		fprintf(stderr, usage_create, argv0);
		return ERR_ARGS_COUNT;
	}

	uint64_t first_lba, last_lba, table_lba;
	uint32_t partition_number;
	check(sscanf(argv[0], "%" PRIu32, &partition_number) == 1, ERR_ARGS, "Invalid partition number: %s", argv[0]);
	check(sscanf(argv[1], "%" PRIu64, &first_lba) == 1, ERR_ARGS, "Invalid LBA: %s", argv[1]);
	check(sscanf(argv[2], "%" PRIu64, &last_lba) == 1, ERR_ARGS, "Invalid LBA: %s", argv[2]);

	table_lba = get_partition_table_offset(partition_number);

	gpt_entry_t table[4];
	fseek(disk_f, (long)table_lba, SEEK_SET);
	check(fread(&table, 1, 512, disk_f) == 512, ERR_READ, "Read error on %s", argv1);

	gpt_entry_t& partition_entry = table[partition_number % 4];
	for (auto& x : partition_entry.partition_guid)
		x = uint8_t(rand());

	partition_entry.first_lba = first_lba;
	partition_entry.last_lba = last_lba;

	fseek(disk_f, (long)table_lba, SEEK_SET);
	check(fwrite(&table, 1, 512, disk_f) == 512, ERR_READ, "Write error on %s", argv1);
	fflush(disk_f);

	update_gpt();
	return 0;
}

int extract_guid_le(char* dst, const char* src)
{
	check(strlen(src) == 32, ERR_ARGS, "GUID in little endian must be 32 hex digits");

	auto extract_hex_digit = []
	(char c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';
		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;
		return -1;
	};

	for (int i = 0; i < 32; i += 2)
	{
		uint8_t x = 0;
		for (int j = i; j < i + 2; ++j)
		{
			x *= 16;
			int d = extract_hex_digit(src[j]);
			check(d >= 0, ERR_ARGS, "Invalid hex digit: %c", src[i]);
		}
		*dst++ = char(x);
	}

	return 0;
}

int subcommand_set_type_str(const char* value, gpt_entry_t& entry)
{
	check(strlen(value) == 16, ERR_ARGS, "GUID str must be 16 characters long");
	memcpy(entry.type_guid, value, 16);
	return 0;
}
int subcommand_set_type_le(const char* value, gpt_entry_t& entry)
{
	return extract_guid_le(entry.type_guid, value);
}
int subcommand_set_guid_str(const char* value, gpt_entry_t& entry)
{
	check(strlen(value) == 16, ERR_ARGS, "GUID str must be 16 characters long");
	memcpy(entry.partition_guid, value, 16);
	return 0;
}
int subcommand_set_guid_le(const char* value, gpt_entry_t& entry)
{
	return extract_guid_le(entry.partition_guid, value);
}
int subcommand_set_first(const char* str, gpt_entry_t& entry)
{
	uint64_t value;
	check(sscanf(str, "%" PRIu64, &value) == 1, ERR_ARGS, "Invalid LBA: %s", str);
	entry.first_lba = value;
	return 0;
}
int subcommand_set_last(const char* str, gpt_entry_t& entry)
{
	uint64_t value;
	check(sscanf(str, "%" PRIu64, &value) == 1, ERR_ARGS, "Invalid LBA: %s", str);
	entry.last_lba = value;
	return 0;
}
int subcommand_set_name(const char* str, gpt_entry_t& entry)
{
	memset(&entry.name, 0, sizeof(entry.name));
	setlocale(LC_ALL, "");

	mbstate_t state{};
	size_t len = strlen(str) + 1;
	for (size_t rdi = 0; true; ++rdi)
	{
		check(rdi < std::size(entry.name), ERR_ARGS, "Unicode string must not exceed 32 UTF-16 code points");

		char16_t c;
		size_t result = mbrtoc16(&c, str, len, &state);
		if (result == 0)
			break;
		if (result == (size_t)-3)
			result = 0;

		check(result < (size_t)-3, ERR_ARGS, "Unicode conversion failed");

		str += result;
		len -= result;
		entry.name[rdi] = c;
	}

	return 0;
}

int command_set(int argc, const char* const* argv)
{
	if (argc != 3)
	{
		fprintf(stderr, usage_create, argv0);
		return ERR_ARGS_COUNT;
	}

	const std::pair<const char*, int(*)(const char*, gpt_entry_t&)> handlers[] =
	{
		{ "type_str", subcommand_set_type_str },
		{ "type_le",  subcommand_set_type_le },
		{ "guid_str", subcommand_set_guid_str },
		{ "guid_le",  subcommand_set_guid_le },
		{ "first",    subcommand_set_first },
		{ "last",     subcommand_set_last },
		{ "name",     subcommand_set_name },
	};

	uint32_t partition_number;
	check(sscanf(argv[0], "%" PRIu32, &partition_number) == 1, ERR_ARGS, "Invalid partition number: %s", argv[0]);
	uint64_t table_offset = get_partition_table_offset(partition_number);

	gpt_entry_t table[4];
	fseek(disk_f, (long)table_offset, SEEK_SET);
	check(fread(&table, 1, 512, disk_f) == 512, ERR_READ, "Read error on %s", argv1);

	for (const auto& [cmd, handler] : handlers)
	{
		if (strcmp(cmd, argv[1]) == 0)
		{
			auto result = handler(argv[2], table[partition_number % 4]);
			if (result != 0)
				return result;
			break;
		}
	}

	fseek(disk_f, (long)table_offset, SEEK_SET);
	check(fwrite(&table, 1, 512, disk_f) == 512, ERR_READ, "Write error on %s", argv1);
	fflush(disk_f);

	update_gpt();
	return 0;
}
//commands:
//table #n
//create #n first last
//set #n X Y
//	part[n].X = Y
//	X = { type_str, type_le, guid_str, guid_le, first, last, flags, name }
//delete #n
//zero #n
//wipe

int main(int argc, const char* const* argv)
{
	using namespace std::chrono;
	//I like chrono lib
	srand((unsigned)duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count());
	argv0 = argv[0];

	/*
	const char* const _argv[] =
	{
		nullptr,
		"/home/kodirovsshik/os6/system/disk",
		"create",
		"0",
		"36",
		"39"
	};

	argv = _argv;
	argc = sizeof(_argv) / sizeof(char*);
	//*/

	if (argc < 3)
	{
		usage();
		return ERR_ARGS_COUNT;
	}

	argv1 = argv[1];
	disk_f = fopen(argv[1], "rb+");
	check(disk_f, ERR_OPEN, "Failed to open %s", argv1);

	const std::pair<const char*, int(*)(int, const char* const*)> handlers[] =
	{
		{ "table",  command_table },
		{ "create", command_create },
		{ "set",    command_set },
		//{ "delete", command_delete },
		//{ "zero",   command_zero },
		//{ "wipe",   command_wipe },
	};

	for (const auto& [cmd, handler] : handlers)
	{
		if (strcmp(cmd, argv[2]) == 0)
			return handler(argc - 3, argv + 3);
	}

	usage();
	return ERR_ARGS_COUNT;
}
