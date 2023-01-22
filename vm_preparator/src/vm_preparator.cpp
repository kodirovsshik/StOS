
/*
Invoke as:
	a.out VM_DISK VM_MNT KERNEL_BIN PBR_BIN LOADER_BIN
*/

#include <unistd.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>
#include <format>
#include <filesystem>


using namespace std;
using namespace std::string_literals;


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
		throw (std::string)(excp);						\
}



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
	int code = -1;
	auto result = pipe(format("losetup --show -o 1MiB -f {}", disk), &code);
	if (code != 0)
		throw format("losetup returned {}", code);
	return result;
}

void invoke_shell(string_view cmd)
{
	cout << cmd << "\n";
	if (system(cmd.data()) != 0)
		throw ""s;
}

auto filename(string_view path)
{
	while (path.size() && path.back() == '/')
		path = path.substr(0, path.size() - 1);
	return path.substr(path.find_last_of('/') + 1);
}

auto discard_prefix(string_view str, string_view prefix)
{
	return str.substr(str.find_first_not_of(prefix));
}



constexpr uint32_t sectors_per_cluster = 8;
constexpr uint32_t sector_size = 512;
constexpr uint32_t cluster_size = sector_size * sectors_per_cluster;
static_assert(sector_size == 512); //fundamental constant of nature


uint8_t cluster_buffer[cluster_size];
uint8_t fat_sector_buffer[sector_size];



struct __attribute__((__packed__)) fat32_bpb_sector
{
	uint8_t jmp_boot[3];
	char oem_name[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors_count;
	uint8_t number_of_fats;
	uint16_t root_entries_count;
	uint16_t total_sectors16;
	uint8_t media;
	uint16_t fat_size_sectors16;
	uint16_t sectors_per_track;
	uint16_t number_of_heads;
	uint32_t hidden_sectors;
	uint32_t total_sectors32;
	uint32_t fat_size_sectors32;
	uint8_t active_fat : 4;
	uint8_t _reserved1 : 3;
	bool mirroring_disabled : 1;
	uint8_t _reserved2;
	uint8_t fs_version_minor;
	uint8_t fs_version_major;
	uint32_t root_cluster;
	uint16_t fsinfo_sector;
	uint16_t backup_sector;
	uint8_t _reserved3[12];
	uint8_t drive_number;
	uint8_t _reserved4;
	uint8_t boot_signature;
	uint32_t volume_id;
	char volume_laber[11];
	char fs_type[8];
	char _padding[420];
	uint16_t AA55;
};
static_assert(sizeof(fat32_bpb_sector) == sector_size);

struct __attribute__((__packed__)) fat32_directory_entry
{
	union
	{
		struct
		{
			char name[8];
			char extention[3];
		};
		char shortname[11];
	};
	uint8_t attributes;
	uint8_t _reserved;
	uint8_t creation_time_tenth;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t last_access_date;
	uint16_t first_clusher_high;
	uint16_t write_time;
	uint16_t write_date;
	uint16_t first_clusher_low;
	uint32_t file_size;
};
static_assert(sizeof(fat32_directory_entry) == 32);

struct __attribute__((__packed__)) fat32_long_directory_entry
{
	uint8_t order : 6;
	bool last : 1;
	bool _unused : 1;
	uint16_t name1[5];
	uint8_t attributes;
	uint8_t type;
	uint8_t checksum;
	uint16_t name2[6];
	uint16_t _reserved;
	uint16_t name3[2];

	static constexpr uint8_t chars_offsets[] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };
	static constexpr uint8_t chars_count = sizeof(chars_offsets);
};
static_assert(sizeof(fat32_long_directory_entry) == 32);

namespace fat32_dir_attributes
{
	enum : uint8_t
	{
		read_only = 0x01,
		hidden = 0x02,
		system = 0x04,
		volume_id = 0x08,
		directory = 0x10,
		archive = 0x20,
		long_name = read_only | hidden | system | volume_id,
		long_name_mask = read_only | hidden | system | volume_id | directory | archive,
	};
};


//Yes, I'm gonna use C files through this wrapper instead of C++ fstreams
//Cry about it
struct smart_file
{
private:
	FILE* fd = nullptr;


public:
	smart_file(FILE* _fd = nullptr) : fd(_fd) {}
	smart_file(const char* name, const char* mode) 
	{
		this->open(name, mode);
	}
	smart_file(const smart_file&) = delete;
	smart_file(smart_file&& other)
	{
		this->fd = other.fd;
		other.fd = nullptr;
	}
	~smart_file()
	{
		this->close();
	}

	smart_file& operator=(const smart_file& other) = delete;
	smart_file& operator=(smart_file&& other)
	{
		this->close();
		this->fd = other.fd;
		other.fd = nullptr;
		return *this;
	}

	void open(const char* name, const char* mode) 
	{
		this->close();
		this->fd = fopen(name, mode);
	}
	void close()
	{
		if (this->fd)
			fclose(this->fd);
		this->fd = nullptr;
	}

	operator FILE*() const
	{
		return this->fd;
	}
	explicit operator bool() const
	{
		return this->fd != nullptr;
	}
};



void read_sectors(FILE* fd, size_t sector, size_t count, void* buff)
{
	const size_t read_offset = sector * sector_size;
	const size_t read_size = count * sector_size;
	fseek(fd, read_offset, SEEK_SET);
	tassert(fread(buff, 1, read_size, fd) == read_size, 
		format("read failed: {} sectors startting from {}", count, sector));
}

int main(int argc, char** argv)
{
	bool pause = false;
	rassert(argc == 6 || (argc == 7 && (pause = strcmp(argv[6], "pause") == 0)), 1,
		"%s: error: 5 arguments required", argv[0]);
	
	string_view disk = argv[1];
	string_view mount_point = argv[2];
	string_view kernel_bin = argv[3];
	string_view pbr_bin = argv[4];
	string_view loader_bin = argv[5];
	string loop_device;
	bool mounted = false;
	bool err = false;

	smart_file lodev = nullptr;

	struct
	{
		uint32_t fat_sector_rel;
		uint32_t data_sector_rel;
		uint32_t root_cluster;
		uint32_t cluster_size_bytes;
		uint32_t total_clusters;

	} fs{};

	auto sync = [&]
	{
		invoke_shell(format("sync --file-system {}/.", mount_point));
	};
	auto attach_loop_device = [&]
	{
		loop_device = create_loop_device(disk);
		lodev = fopen(loop_device.c_str(), "rb");
		tassert(lodev, format("{}: failed to open {}", argv[0], loop_device));
		setvbuf(lodev, 0, _IONBF, 0);
	};
	auto detach_loop_device = [&]
	{
		if (!loop_device.empty())
		{
			invoke_shell(format("losetup -d {}", loop_device));
			loop_device.clear();
		}
	};
	auto mount = [&]
	{
		invoke_shell(format("mount {} {}", loop_device, mount_point));
		mounted = true;
	};
	auto umount = [&]
	{
		if (mounted)
		{
			sync();
			invoke_shell(format("umount {}", loop_device));
			mounted = false;
		}
	};

	auto cluster_to_rel_sector = [&]
	(uint32_t clus) -> uint32_t
	{
		return fs.data_sector_rel + (clus - 2) * sectors_per_cluster;
	};
	
	auto next_cluster = [&]
	(uint32_t clus) -> uint32_t
	{
		constexpr uint32_t clusters_per_fat_sector = sector_size / 4;
		const uint32_t fat_sector = clus / clusters_per_fat_sector;
		const uint32_t fat_sector_entry = clus % clusters_per_fat_sector;
		read_sectors(lodev, fs.fat_sector_rel + fat_sector, 1, fat_sector_buffer);
		const uint32_t result = ((uint32_t*)fat_sector_buffer)[fat_sector_entry];
		return (result >= 0x0FFFFFF7) ? 0 : (result & 0x0FFFFFFF);
	};
	
	auto read_cluster = [&]
	(uint32_t cluster)
	{
		read_sectors(lodev,	cluster_to_rel_sector(cluster), 
			sectors_per_cluster, cluster_buffer);
	};
	
	auto directory_descend = [&]
	(uint32_t directory_cluster, string_view subdir_name, bool& is_dir) -> uint32_t
	{
		is_dir = directory_cluster > 1;
		if (subdir_name.size() == 0)
			return directory_cluster;

		is_dir = false;

		const char* const pname_rend = subdir_name.data() - 1;
		const char* const pname_rbegin = subdir_name.data() + subdir_name.size() - 1;

		const char* pname = pname_rbegin;
		//index of last char within last long directory entry name field
		const int long_name_char_index_initial = (subdir_name.size() - 1)
			% fat32_long_directory_entry::chars_count;
		int long_name_char_index = long_name_char_index_initial;

		constexpr uint32_t entry_size = sizeof(fat32_directory_entry);
		constexpr uint32_t entries_per_cluster = cluster_size / entry_size;

		uint32_t prev_cluster = 0;
		uint32_t current_cluster = directory_cluster;
		uint32_t current_entry = 0;

		auto pcurrent_entry = [&] () -> fat32_directory_entry*
		{
			return (fat32_directory_entry*)cluster_buffer + current_entry;
		};
		auto pcurrent_lentry = [&] () -> fat32_long_directory_entry*
		{
			return (fat32_long_directory_entry*)cluster_buffer + current_entry;
		};
		auto pcurrent_lentry_char = [&]
		(size_t index) -> char
		{
			return ((char*)pcurrent_lentry())[fat32_long_directory_entry::chars_offsets[index]];
		};
		auto reset_long_entry_scan_state = [&]
		{
			long_name_char_index = long_name_char_index_initial;
			pname = pname_rbegin;
		};

		while (true)
		{
			if (current_cluster == 0)
				return 0;
			while (current_entry >= entries_per_cluster)
			{
				current_cluster = next_cluster(current_cluster);
				current_entry -= entries_per_cluster;
			}
			if (current_cluster != prev_cluster)
				read_cluster(prev_cluster = current_cluster);

			//got a total match during previous iterations
			if (pname == pname_rend)
				break;
			
			if (pcurrent_entry()->name[0] == (char)0xE5)
			{
				//Empty FAT directory entry
				++current_entry;
				long_name_char_index = long_name_char_index_initial;
				continue;
			}

			if (pcurrent_entry()->name[0] == (char)0x00)
				return 0; //Rest of directory is empty

			if ((pcurrent_entry()->attributes & fat32_dir_attributes::long_name_mask)
				!= fat32_dir_attributes::long_name)
			{
				reset_long_entry_scan_state();

				//Convert subdir_name to canonnicalized FAT name and compare with stored name

				char fat_subdir_name[11];
				const size_t dot_pos = subdir_name.find_last_of('.');
				const size_t before_dot = dot_pos;
				const size_t after_dot = subdir_name.size() - dot_pos - 1;
				if (dot_pos == string::npos ||
					before_dot > 8 ||
					after_dot > 3 )
				{
					++current_entry;
					continue;
				}

				memset(fat_subdir_name, ' ', sizeof(fat_subdir_name));
				memcpy(fat_subdir_name, subdir_name.data(), dot_pos);
				memcpy(fat_subdir_name + 8, subdir_name.data() + dot_pos + 1, after_dot);

				for (size_t i = 0; i < 11; ++i)
					fat_subdir_name[i] = toupper(fat_subdir_name[i]);

				if (memcmp(fat_subdir_name, pcurrent_entry()->shortname, 11) == 0)
					break;

				++current_entry;
				continue;
			}

			//assume long directory
			//reverse compare subdir_name to dir[current_entry].name

			//check if the string is actually NULL terminated 
			//	(or ends perfectly at the end of entry)
			//and we are not comparing mid-string
			if (long_name_char_index == fat32_long_directory_entry::chars_count - 1 ||
				pcurrent_lentry_char(long_name_char_index + 1) == (char)0x00
				)
			{
				while (long_name_char_index >= 0)
				{
					if ((uint8_t)*pname == pcurrent_lentry_char(long_name_char_index))
					{
						--pname;
						--long_name_char_index;
					}
					else
						break;
				}
			}

			if (long_name_char_index >= 0)
			{
				//mismatch encountered, full reset
				reset_long_entry_scan_state();

				//skip all subsequent long name entries in current series
				current_entry += pcurrent_lentry()->order + 1;
			}
			else
			{
				//match next long name directory entry from its last char
				long_name_char_index = fat32_long_directory_entry::chars_count - 1;
				++current_entry;
			}
		}

		const uint32_t cluster_number_high = pcurrent_entry()->first_clusher_high;
		const uint32_t cluster_number_low = pcurrent_entry()->first_clusher_low;
		is_dir = pcurrent_entry()->attributes & fat32_dir_attributes::directory;
		return (cluster_number_high << 16) | cluster_number_low;
	};

	auto get_file_first_cluster = [&]
	(string_view path) -> uint32_t
	{
		uint32_t cluster = fs.root_cluster;
		const size_t path_end = path.size();
		size_t name_start = 0;

		while (name_start < path_end)
		{
			if (path[name_start] == '/')
			{
				name_start++;
				continue;
			}
			size_t name_end = path.find('/', name_start);
			if (name_end == string::npos)
				name_end = path_end;
			auto searchee = path.substr(name_start, name_end - name_start);
			
			bool is_dir;
			cluster = directory_descend(cluster, searchee, is_dir);

			if (cluster == 0)
				throw format("When looking for \"{}\": \"{}\" was not found", path, searchee);
			if (!is_dir && name_end != path_end)
				throw format("When looking for \"{}\": \"{}\" is a file and not a directory", path, searchee);
			name_start = name_end + 1;
		}
		return cluster;
	};

	auto generate_listing = [&]
	(string_view src, string_view dst) -> uint32_t
	{
		sync();

		smart_file fd(dst.data(), "w+b");
		tassert(fd, format("Failed to open {}", dst));
		setvbuf(fd, nullptr, _IONBF, 0);

		uint32_t file_size = (uint32_t)filesystem::file_size(src);
		src = discard_prefix(src, mount_point);

		struct
		{
			uint32_t sectors[101];
			uint8_t counts[101];
			uint8_t _pad[3];
			uint32_t next_sector;
		} sector_listing_element{};
		static_assert(sizeof(sector_listing_element) == 512);

		uint8_t element_entry_index = 0;
		uint32_t elements_count = 0;

		auto append = [&]
		(uint32_t cluster, uint32_t size)
		{
			static_assert(sectors_per_cluster < 128);

			size = min<uint32_t>(size, cluster_size);
			const uint32_t new_sectors = (size + (sector_size - 1)) / sector_size;
			const uint32_t current_sector = cluster_to_rel_sector(cluster);

			do //try chain joining
			{
				if (element_entry_index == 0)
					break;
				const auto prev_sector = sector_listing_element.sectors[element_entry_index - 1];
				const int prev_count = sector_listing_element.counts[element_entry_index - 1];
				const auto prev_end = prev_sector + prev_count;
				if (current_sector != prev_end)
					break;
				const int new_count = prev_count + new_sectors;
				if (new_count >= 128)
					continue;
				sector_listing_element.counts[element_entry_index - 1] = (uint8_t)new_count;
				return;
			} while (false);

			if (element_entry_index == 101)
			{
				++elements_count;
				fwrite(&sector_listing_element, 1, sizeof(sector_listing_element), fd);
				memset(&sector_listing_element, 0, sizeof(sector_listing_element));
				element_entry_index = 0;
			}
			sector_listing_element.sectors[element_entry_index] = current_sector;
			sector_listing_element.counts[element_entry_index] = new_sectors;
			++element_entry_index;
		};

		uint32_t cluster = get_file_first_cluster(src);
		while (cluster)
		{
			append(cluster, file_size);
			cluster = next_cluster(cluster);
			file_size -= cluster_size;
		}

		if (element_entry_index != 0)
		{
			++elements_count;
			fwrite(&sector_listing_element, 1, sizeof(sector_listing_element), fd);
		}
		
		//go through file and refill next sector data
		fflush(fd);
		sync();
		rewind(fd);

		uint32_t current_cluster;
		uint32_t current_sector;
		uint32_t current_sector_last;

		auto refill_location_vars = [&]
		(uint32_t cluster)
		{
			current_cluster = cluster;
			current_sector = cluster_to_rel_sector(current_cluster);
			current_sector_last = current_sector + sectors_per_cluster - 1;
		};
		dst = discard_prefix(dst, mount_point);
		refill_location_vars(get_file_first_cluster(dst));

		const uint32_t first_sector = current_sector;

		while (true)
		{
			if (--elements_count == 0)
				break;

			const size_t read = fread(&sector_listing_element, 1, sizeof(sector_listing_element), fd);

			if (read != sizeof(sector_listing_element))
				break;

			if (current_sector == current_sector_last)
				refill_location_vars(next_cluster(current_cluster));
			else
				++current_sector;

			sector_listing_element.next_sector = current_sector;
			fseek(fd, -(long)sizeof(sector_listing_element), SEEK_CUR);
			fwrite(&sector_listing_element, 1, sizeof(sector_listing_element), fd);
		}

		fd.close();
		sync();

		return first_sector;
	};

	auto generate_listing_for = [&]
	(string_view file)
	{
		return generate_listing(
			format("{}", file),
			format("{}.lst", file)
		);
	};


	auto generate_stos_file_listing = [&]
	(string_view binary)
	{
		return generate_listing_for(
			format("{}/StOS/{}", mount_point, 
			filename(binary))
		);
	};

	auto patch_file = []
	(string_view filename, size_t offset, const void* data, size_t size)
	{
		smart_file fd = fopen(filename.data(), "r+b");
		tassert(fd, format("Failed to open\"{}\"", filename));
		
		fseek(fd, offset, SEEK_SET);
		tassert(size == fwrite(data, 1, size, fd), format("Failed to patch \"{}\"", filename));
	};

	auto read_fat_bpb = [&]
	{
		fat32_bpb_sector bpb{};
		read_sectors(lodev, 0, 1, &bpb);
		tassert(bpb.AA55 == 0xAA55, "Invalid FAT bpb");

		fs.root_cluster = bpb.root_cluster & 0x0FFFFFFF;

		fs.cluster_size_bytes = bpb.bytes_per_sector * (uint32_t)bpb.sectors_per_cluster;

		uint32_t fat_sectors = bpb.fat_size_sectors16;
		if (fat_sectors == 0)
			fat_sectors = bpb.fat_size_sectors32;

		fs.fat_sector_rel = bpb.reserved_sectors_count;
		if (bpb.mirroring_disabled)
			fs.fat_sector_rel += fat_sectors * sector_size * bpb.active_fat;

		fs.data_sector_rel = bpb.reserved_sectors_count + fat_sectors * bpb.number_of_fats;

		uint32_t total_sectors = bpb.total_sectors16;
		if (total_sectors == 0)
			total_sectors = bpb.total_sectors32;
		
		fs.total_clusters = total_sectors / sectors_per_cluster;
	};


	try
	{
		attach_loop_device();
		invoke_shell(format(
			"mkfs.fat -F 32 -s {} -S {} {} >/dev/null",
			sectors_per_cluster, sector_size, loop_device
		));
		
		read_fat_bpb();

		invoke_shell(format("mkdir -p {}", mount_point));

		mount();
		invoke_shell(format("mkdir {}/StOS", mount_point));

		invoke_shell(format("cp {} {}/StOS", kernel_bin, mount_point));
		const uint32_t kernel_listing_sector = generate_stos_file_listing(kernel_bin);
		patch_file(loader_bin, 4, &kernel_listing_sector, sizeof(kernel_listing_sector));

		invoke_shell(format("cp {} {}/StOS", loader_bin, mount_point));
		const uint32_t loader_listing_sector = generate_stos_file_listing(loader_bin);
		patch_file(pbr_bin, 108, &loader_listing_sector, sizeof(loader_listing_sector));
	}
	catch (const std::string& s)
	{
		if (!s.empty())
			cerr << s << '\n';
		err = true;
	}
	catch (const std::exception& excp)
	{
		cerr << excp.what() << '\n';
		err = true;
	}

	if (pause)
	{
		cout << "PAUSE";
		cin.get();
	}
	
	lodev.close();
	umount();
	detach_loop_device();

	return (int)err;
}
