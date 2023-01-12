
#include <stdint.h>
#include "include/farptr.hpp"
#include "include/interrupt.hpp"
#include "include/alloc.hpp"
#include "include/conio.hpp"
#include "include/loader.hpp"



uint16_t* vbe_video_modes_ptr;
uint16_t vbe_video_modes_count_usable;
uint16_t vbe_video_modes_count_total;



struct __attribute__((__packed__)) vbe_info_block
{
	union
	{
		char vbe_signature[4];
		uint32_t vbe_signature32;
	};
	union 
	{
		uint16_t vbe_version;
		struct 
		{
			uint8_t vbe_version_low;
			uint8_t vbe_version_high;
		};
	};
	FarPtr<char> oem_string_ptr; //far ptr
	union
	{
		uint32_t capabilities;
		struct 
		{
			bool capabilities_bit0: 1;
			bool capabilities_vga_incompatible : 1;
		};
		
	};
	FarPtr<uint16_t> video_mode_ptr;
	uint16_t total_memory;
	uint16_t oem_software_rev;
	FarPtr<const char> oem_vendor_name_ptr;
	FarPtr<const char> oem_product_name_ptr;
	FarPtr<const char> oem_product_rev_ptr;

	char _reserved[222];
	char _oem_data[256];
};
static_assert(sizeof(vbe_info_block) == 512, "");

struct __attribute__((__packed__)) mode_info_block
{
	//VBE 1.0

	union
	{
		uint16_t mode_attributes;
		struct 
		{
			bool attrib_supported : 1;
			bool attrib_bit1 : 1;
			bool attrib_bios_tty_supported : 1;
			bool attrib_color : 1;
			bool attrib_graphics : 1;
			bool attrib_vga_incompatible : 1;
			bool attrib_bit6 : 1;
			bool attrib_has_linear_frame_buffer : 1;
		};
	};
	uint8_t wina_attributes;
	uint8_t winb_attributes;
	uint16_t win_granularity;
	uint16_t win_size;
	uint16_t wina_segment;
	uint16_t winb_segment;
	uint32_t window_func_ptr;
	uint16_t bytes_per_scan_line;
	
	//VBE 1.2

	uint16_t x_resolution;
	uint16_t y_resolution;
	uint8_t x_char_size;
	uint8_t y_char_size;
	uint8_t number_of_planes;
	uint8_t bits_per_pixel;
	uint8_t number_of_banks;
	uint8_t memory_model;
	uint8_t bank_size;
	uint8_t number_of_image_pages;
	uint8_t reserved0;

	uint8_t red_mask_size;
	uint8_t red_field_position;
	uint8_t green_mask_size;
	uint8_t green_field_position;
	uint8_t blue_mask_size;
	uint8_t blue_field_position;
	uint8_t rsvd_mask_size;
	uint8_t rsvd_field_position;
	uint8_t direct_color_mode_info;

	//VBE 2.0

	uint32_t phys_base_ptr;
	uint32_t reserved1;
	uint16_t reserved2;

	//VBE 3.0

	uint16_t lin_bytes_per_scan_line;
	uint8_t bnk_number_of_image_pages;
	uint8_t lin_number_of_image_pages;
	uint8_t lin_red_mask_size;
	uint8_t lin_red_field_position;
	uint8_t lin_green_field_position;
	uint8_t lin_green_mask_size;
	uint8_t lin_blue_field_position;
	uint8_t lin_blue_mask_size;
	uint8_t lin_rsvd_field_position;
	uint8_t lin_rsvd_mask_size;
	uint32_t max_pixel_clock;

	uint8_t reserved_pad[190];
};
static_assert(sizeof(mode_info_block) == 256, "");



[[noreturn]]
void error_no_vbe3_support()
{
	cpanic("VBE 3.0 support required");
}


void get_vbe_information(vbe_info_block& vbe)
{
	vbe.vbe_signature32 = 0x32454256;
	
	regs_t regs{};
	regs.ax = 0x4F00;
	regs.di = ptr_cast(uint16_t, &vbe);
	interrupt(&regs, 0x10);

	if (regs.flags & EFLAGS_CARRY)
		error_no_vbe3_support();
	if (regs.ax != 0x004F)	
		error_no_vbe3_support();
	
	cputs("VBE ");
	cput32u(vbe.vbe_version_high);
	cputc('.');
	cput32u(vbe.vbe_version_low);
	cputc('\n');

	if (vbe.vbe_version_high < 3)
		error_no_vbe3_support();
	
	cputs(vbe.oem_vendor_name_ptr, endline);
	cputs(vbe.oem_product_name_ptr, endline);
	cputs(vbe.oem_product_rev_ptr, endline);
}

void vbe_print_video_mode_info(const mode_info_block& info)
{
#ifdef _DEBUG
	cputc("US"[info.attrib_supported]);
	cputc("tT"[info.attrib_bios_tty_supported]);
	cputc("MC"[info.attrib_color]); // monochrome/color
	cputc("TG"[info.attrib_graphics]); // text/graphics
	cputc("Vv"[info.attrib_vga_incompatible]);
	cputc("lL"[info.attrib_has_linear_frame_buffer]);
	cputc(' ');

	cput32u(info.x_resolution);
	cputc('x');
	cput32u(info.y_resolution);
	cputc('x');
	cput32u(info.bits_per_pixel);
	cputc(' ');

	cput32u(info.memory_model);
	cputc('\n');
#endif
}

bool vbe_check_video_mode(const mode_info_block& info)
{
	if (!info.attrib_supported)
		return false;
	return true;
}

void save_fitting_vbe_video_modes(FarPtr<uint16_t> p_video_modes)
{
	auto p_dst = (uint16_t*)c_heap_get_ptr();
	vbe_video_modes_ptr = p_dst;

	while (true)
	{
		uint16_t mode = *p_video_modes++;
		if (mode == 0xFFFF)
			break;
		++vbe_video_modes_count_total;

		mode_info_block info;

		regs_t regs{};
		regs.cx = mode;
		regs.ax = 0x4F01;
		regs.di = ptr_cast(uint16_t, &info);
		interrupt(&regs, 0x10);

		vbe_print_video_mode_info(info);
		if (!vbe_check_video_mode(info))
			continue;
		*p_dst++ = mode;
	}
	vbe_video_modes_count_usable = p_dst - vbe_video_modes_ptr;
	c_heap_set_ptr(p_dst);
}

void vbe_pick_video_mode()
{
	//TODO
}

void print_vbe_video_modes_info()
{
	cput32u(vbe_video_modes_count_total);
	cputs(" modes reported, ");
	cput32u(vbe_video_modes_count_usable);
	cputs(" modes usable\n");
}

extern "C"
void do_subtask_vbe()
{
	vbe_info_block vbe;
	get_vbe_information(vbe);
	save_fitting_vbe_video_modes(vbe.video_mode_ptr);
	print_vbe_video_modes_info();
	vbe_pick_video_mode();
}
