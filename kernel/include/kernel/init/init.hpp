
#ifndef _KERNEL_INIT_HPP_
#define _KERNEL_INIT_HPP_

struct loader_data_t
{
	uint8_t boot_disk_uuid[16];
	uint64_t boot_partition_lba;
	uint32_t free_paging_area;
	uint32_t memory_map_addr;
	uint16_t memory_map_size;
	uint16_t output_buffer_index;
};

#endif //!_KERNEL_INIT_HPP_
