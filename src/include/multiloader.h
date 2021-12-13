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


/*

multiliader.h - Basic defines and typedefs for
MBR/VBR cooperation

*/

#ifndef _MULTILOADER_H_
#define _MULTILOADER_H_

#include <stdint.h>



//Common stuff

typedef struct
{
	uint16_t struct_size = 0;
	uint16_t request_number;
	uint16_t request_data; // IN/OUT
	uint16_t mbr_version;
	uint16_t free_memory_begin;
} stos_request_header_t;



inline const char* get_vbr_invoke_error_desc(int error);



//Request return error codes

#define STOS_REQ_INVOKE_OK 0
#define STOS_REQ_INVOKE_ERR_INVALID_REQUEST_ID 1
#define STOS_REQ_INVOKE_ERR_READ_ERR 2



//VBR services

#define STOS_REQ_GET_MBR_BOOT_OPTIONS 1
#define STOS_REQ_MBR_BOOT 2



//VBR services' individual data

//STOS_REQ_GET_MBR_BOOT_OPTIONS
enum class bootability_status_t : uint8_t
{
	bootable = 1,
	no_stos,
	no_partition,
	fs_error,
	read_error,
	no_disk
};


//STOS_REQ_MBR_BOOT
enum class boot_error_t : uint8_t
{
	read_error = 1,
	no_stos,
	no_disk,
	no_partition,
	out_of_memory,
	cpuid_error
};








inline const char* get_vbr_invoke_error_desc(int n)
{
	static const char* const arr[] =
	{
		"No error",
		"Invalid request ID",
		"Read error",
		"(unknown error)"
	};

	static const size_t N = sizeof(arr) / sizeof(arr[0]);

	if (n < 0 || (size_t)n >= N)
		n = N - 1;

	return arr[n];
}

inline const char* get_bootability_status_desc(bootability_status_t s)
{
	switch (s)
	{
	case bootability_status_t::bootable:     return "Bootable";
	case bootability_status_t::no_stos:      return "No StOS installation found";
	case bootability_status_t::no_partition: return "Partition not present";
	case bootability_status_t::fs_error:     return "Filesystem error";
	case bootability_status_t::read_error:   return "Read error";
	case bootability_status_t::no_disk:      return "Disk not present";
	}

	return "Unknown status";
}

inline bool get_bootability_status_severity(bootability_status_t s)
{
	return (int)s >= (int)bootability_status_t::fs_error;
}

#endif //!_MULTILOADER_H_
