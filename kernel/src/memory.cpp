
#include <stdint.h>
#include "kernel/aux/utility.hpp"
#include "kernel/aux/memset.hpp"
#include "kernel/memory.hpp"


#define PME_PHYS_ADDR_MASK 	0x0000FFFFFFFFF000u
#define PME_FLAGS_MASK		0x800000000000011Eu

#define PME_PRESENT_BIT (uint64_t(1) << 0)

#define PME_HUGE_PAGE_BIT (uint64_t(1) << 7)


#define MMAP_FLAGS_MASK	(0 \
	| MMAP_WRITABLE \
	| MMAP_USER_ACCESSABLE \
	| MMAP_WRITETHROUGH \
	| MMAP_UNCACHABLE \
	| MMAP_GLOBAL \
	| MMAP_UNEXECUTABLE \
	| MMAP_ALLOW_ADDRESS_OVERRIDE \
	| 0)





struct _paging_memory_manager_t
{
private:
	constexpr static uint64_t end = 0x80000;
	
	uint64_t _allocate_basic();

public:
	uint64_t allocate();
	void deallocate(uint64_t page);

} static paging_memory_manager;

uint64_t _paging_memory_manager_top;

uint64_t _paging_memory_manager_t::allocate()
{
	//Customization point: after proper memory allocators 
	//initialization is done, replace this call with an 
	//indirect call to a replacable allocation routine
	return this->_allocate_basic();
}
uint64_t _paging_memory_manager_t::_allocate_basic()
{
	if (_paging_memory_manager_top != this->end) [[likely]]
	{
		uint64_t addr = _paging_memory_manager_top;
		_paging_memory_manager_top += 4096;		
		memset((void*)addr, 0, 4096);
		return addr;
	}
	//TODO: panic
	return 0;
}



template<class Tr>
concept pml1_traverser = requires(Tr tr)
{
	has_member_function(Tr, pml1e_callback, mmap_result::type, uint64_t&);
};
template<class Tr>
concept pml2_traverser = requires(Tr tr)
{
	has_member_function(Tr, pml2e_callback, mmap_result::type, uint64_t&);
};
template<class Tr>
concept pml3_traverser = requires(Tr tr)
{
	has_member_function(Tr, pml3e_callback, mmap_result::type, uint64_t&);
};
template<class Tr>
concept pml4_traverser = requires(Tr tr)
{
	has_member_function(Tr, pml4e_callback, mmap_result::type, uint64_t&);
};

template<class Tr>
concept pml1_or_below_traverser = pml1_traverser<Tr>;
template<class Tr>
concept pml2_or_below_traverser = pml2_traverser<Tr> || pml1_or_below_traverser<Tr>;
template<class Tr>
concept pml3_or_below_traverser = pml3_traverser<Tr> || pml2_or_below_traverser<Tr>;
template<class Tr>
concept pml4_or_below_traverser = pml4_traverser<Tr> || pml3_or_below_traverser<Tr>;

template<size_t N, class Tr>
struct nth_or_below_pm_traverser
{
	static constexpr bool value = nth_value_v<bool, N
	, false //level 0 being always false simplifies traversion logic
	, pml1_or_below_traverser<Tr>
	, pml2_or_below_traverser<Tr>
	, pml3_or_below_traverser<Tr>
	, pml4_or_below_traverser<Tr>
	>;
};
template<size_t N, class Tr>
static constexpr bool nth_or_below_pm_traverser_v = nth_or_below_pm_traverser<N, Tr>::value;


template<size_t N, class Tr>
struct is_pmlN_traverser
{
	static constexpr bool value = nth_value_v<bool, N
	, false
	, pml1_traverser<Tr>
	, pml2_traverser<Tr>
	, pml3_traverser<Tr>
	, pml4_traverser<Tr>
	>;
};
template<size_t N, class Tr>
constexpr bool is_pmlN_traverser_v = is_pmlN_traverser<N, Tr>::value;


struct address_range
{
	uint64_t address;
	uint32_t pages;
};



template<size_t page_map_level, class Traverser>
mmap_result::type traverse_page_map1(uint64_t, Traverser&, address_range&);


template<size_t page_map_level, class Traverser>
mmap_result::type traverse_page_map_entry(Traverser& tr, uint64_t& pme)
{
	if constexpr (is_pmlN_traverser_v<page_map_level, Traverser>)
	{
		if constexpr (page_map_level == 1)
			return tr.pml1e_callback(pme);
		else if constexpr (page_map_level == 2)
			return tr.pml2e_callback(pme);
		else if constexpr (page_map_level == 3)
			return tr.pml3e_callback(pme);
		else if constexpr (page_map_level == 4)
			return tr.pml4e_callback(pme);
		else
			ct_unreachable("Unreachable");
	}
	else
		return mmap_result::ok;
}

template<size_t N, class Traverser>
mmap_result::type traverse_pmlNe(Traverser& tr, uint64_t& pme)
{
	if constexpr (is_pmlN_traverser_v<N, Traverser>)
	{
		if constexpr (N == 1)
			return tr.pml1e_callback(pme);
		else if constexpr (N == 2)
			return tr.pml2e_callback(pme);
		else if constexpr (N == 3)
			return tr.pml3e_callback(pme);
		else if constexpr (N == 4)
			return tr.pml4e_callback(pme);
		else
			ct_unreachable("Unreachable");
	}
	else
		return mmap_result::ok;
}

template<size_t page_map_level, class Traverser>
mmap_result::type traverse_page_map(uint64_t base, Traverser& tr, address_range& range)
{
	static_assert(page_map_level >= 0 && page_map_level <= 4);
	if constexpr (page_map_level == 4)
		static_assert(pml4_or_below_traverser<Traverser>);

	if constexpr (!nth_or_below_pm_traverser_v<page_map_level, Traverser>)
		return mmap_result::ok;
	else
		return traverse_page_map1<page_map_level>(base, tr, range);
}

template<size_t page_map_level, class Traverser>
mmap_result::type traverse_page_map1(uint64_t pm_base, Traverser& tr, address_range& range)
{
	auto& virtual_addr = range.address;
	auto& pages_left = range.pages;

	uint64_t* pm = (uint64_t*)pm_base;

	static constexpr int address_shift = 12 + 9 * (page_map_level - 1);
	static constexpr uint64_t pages_per_entry = ct_pow<uint64_t>(512, page_map_level - 1);

	uint64_t index = (virtual_addr >> address_shift) & 511;
	const uint64_t max_possible_current_entries_count = (pages_left + pages_per_entry - 1) / pages_per_entry;
	const uint64_t index_end = min<uint64_t>(512, index + max_possible_current_entries_count);
	/*
	const uint64_t current_entries_count = index_end - index;
	uint64_t current_pages_count;
	if constexpr (pages_per_entry == 1)
		current_pages_count = current_entries_count;
	else
		current_pages_count = min<uint64_t>(pages_left, pages_per_entry * current_entries_count);
	*/
	
	while (index != index_end)
	{
		uint64_t& pme = pm[index];
		mmap_result::type subcall_result;
		
		subcall_result = traverse_page_map_entry<page_map_level>(tr, pme);
		if (subcall_result != mmap_result::ok)
			return subcall_result;
		
		const bool current_entry_present = pme & PME_PRESENT_BIT;
		const bool current_entry_is_page = pme & PME_HUGE_PAGE_BIT;
		if (!current_entry_present || current_entry_is_page)
			goto skip_descending_subcall;
		//happy goto appreciation day

		subcall_result = traverse_page_map<page_map_level - 1>(pme & PME_PHYS_ADDR_MASK, tr, range);
		if (subcall_result != mmap_result::ok)
			return subcall_result;
		
//using goto may help in future in case if virtual address advance and 
//pages count desrease is to be moved into the loop
skip_descending_subcall:
		++index;
		if constexpr (!nth_or_below_pm_traverser_v<page_map_level - 1, Traverser>)
		{
			pages_left -= pages_per_entry; 
			// ^^^^^ underflow is ok cuz it only happens on very last iteration
			virtual_addr += 4096 * pages_per_entry;
		}
	}

	return mmap_result::ok;
}


mmap_result::type update_virtual_range_flags(
	uint64_t virtual_address,
	uint32_t pages,
	uint64_t flags
)
{
	static constexpr uint64_t kept_bits_mask = PME_PHYS_ADDR_MASK
		| PME_PRESENT_BIT
		| PME_HUGE_PAGE_BIT
		;

	struct
	{
		uint64_t flags;

		mmap_result::type pml1e_callback(uint64_t& x)
		{
			x = (x & kept_bits_mask) | flags;
			return mmap_result::ok;
		}
		mmap_result::type pml2e_callback(uint64_t& x)
		{
			if (x & PME_HUGE_PAGE_BIT)
				return this->pml1e_callback(x);
			else
				return mmap_result::ok;
		}
	} flags_updater{ flags & PME_FLAGS_MASK };

	address_range range;
	range.address = virtual_address;
	range.pages = pages;

	return traverse_page_map<4>(0x30000, flags_updater, range);
}

static void f()
{	
	(void)paging_memory_manager; //TODO: remove
}

/*

template<class Range>
concept address_range = requires()
{
	has_member_function(Range, get, void, uint64_t virtual_addr, uint64_t& physical, uint32_t& pages);
	has_member_function(Range, advance, void, uint32_t pages);
};



struct mapping_params_t
{
	uint64_t virtual_addr;
	uint64_t flags;
	uint32_t pages;
	bool override_allowed;
};

template<address_range Range>
struct pm_descend_params_t
{
	using mapping_function_ptr = 
		mmap_result::type (*)(
			uint64_t pm_addr,
			mapping_params_t& params,
			Range&,
			const pm_descend_params_t*
		);
	
	pm_descend_params_t<Range>* next_params = nullptr;
	mapping_function_ptr next_function;
	uint64_t pages_per_pm;
	uint8_t pages_per_pm_log2;
	uint8_t address_shift;
};

uint64_t get_pme(uint64_t& pme)
{
	if ((pme & PME_PRESENT_BIT) == 0)
	{
		uint64_t new_addr = paging_memory_manager.allocate();
		pme = new_addr | MMAP_WRITABLE;
		//non-restrictive flags are put on all page map structures
		//flags only applied to pages because of ease of implementing
	}
	return pme & PME_PHYS_ADDR_MASK;
}



//TODO: if range returns nullptr, mark entry not present and release page if was present

template<address_range Range>
mmap_result::type remap_range_pml1(
	uint64_t pml1_ptr_raw,
	mapping_params_t& params,
	Range& phys_range
	)
{
	auto& virtual_ = params.virtual_addr;
	auto& current_pages_left = params.pages;

	uint32_t index = (virtual_ >> 12) & 511;
	const uint32_t index_end = (uint32_t)min<uint64_t>((uint64_t)index + params.pages, 512);

	uint64_t* const pml1 = (uint64_t*)pml1_ptr_raw;

	uint64_t current_phys_addr = 0;
	uint32_t current_pages = 0;
	uint64_t present_bit = 0;

	while (index != index_end)
	{
		const uint64_t old_addr = pml1[index] & PME_PHYS_ADDR_MASK;
		if (current_pages == 0) [[unlikely]]
		{
			current_phys_addr = old_addr;
			current_pages = current_pages_left;
			phys_range.get(virtual_, current_phys_addr, current_pages);
			if (current_pages == 0)
				return mmap_result::insufficient_physical_pages;
			current_pages_left -= current_pages;
			virtual_ += 4096 * current_pages;
			present_bit = (current_phys_addr != 0) ? PME_PRESENT_BIT | 0;
		}
		if (!params.override_allowed && old_addr != current_phys_addr && old_addr != 0)
			return mmap_result::illegal_override;
		pml1[index++] = current_phys_addr | params.flags | present_bit;
		phys_range.advance(1);
		current_phys_addr += 4096;
	}
	return mmap_result::ok;
}

template<address_range Range>
mmap_result::type remap_range_pml2(
	uint64_t pml2_ptr_raw,
	mapping_params_t& params,
	Range& phys_range,
	const pm_descend_params_t<Range>*
	)
{
	auto& virtual_ = params.virtual_addr;
	auto& current_pages_left = params.pages;

	const uint32_t page_tables = (params.pages + 511) / 512;
	uint32_t index = (virtual_ >> 21) & 511;
	const uint32_t index_end = min<uint32_t>(index + page_tables, 512);

	uint64_t* const pml2 = (uint64_t*)pml2_ptr_raw;

	uint64_t current_phys_addr = 0;
	uint32_t current_pages = 0;
	uint64_t present_bit;

	auto process_page_table = [&]
	{
		const uint64_t pml1_addr = get_pme(pml2[index]);

		const auto pages_prev = params.pages;
		const auto call_result = remap_range_pml1(pml1_addr, params, phys_range);

		const auto pages_diff = pages_prev - params.pages;
		current_pages -= pages_diff;
		current_phys_addr += 4096 * pages_diff;
		
		return call_result;
	};

	for (; index != index_end; ++index)
	{
		const uint64_t old_entry_value = pml2[index];
		const bool page_table_present =
			(old_entry_value & PME_PRESENT_BIT) && 
			(old_entry_value & PML2E_2MiB_BIT) == 0;
		if (current_pages_left < 512 || page_table_present)
		{
			auto subcall_result = process_page_table();
			if (subcall_result != mmap_result::ok)
				return subcall_result;
			continue;
		}

		const uint64_t old_addr = pml2[index] & PME_PHYS_ADDR_MASK;
		if (current_pages == 0)
		{
			current_phys_addr = old_addr;
			current_pages = current_pages_left;
			phys_range.get(virtual_, current_phys_addr, current_pages);
			if (current_pages == 0)
				return mmap_result::insufficient_physical_pages;
			current_pages_left -= current_pages;
			present_bit = (current_phys_addr != 0) ? PME_PRESENT_BIT | 0;
		}

		const bool phys_addr_2MiB_aligned = 
			(current_phys_addr & PME_PHYS_ADDR_MASK & 0x1FFFFF) == 0;
		//2MB pages used for performance if possible
		if (phys_addr_2MiB_aligned && current_pages >= 512)
		{
			const bool _2MiB_page_present = 
				(old_entry_value & PME_PRESENT_BIT) &&
				(old_entry_value & PML2E_2MiB_BIT);

			if (!params.override_allowed &&
				old_addr != current_phys_addr &&
				old_addr != 0 &&
				_2MiB_page_present)
				return mmap_result::illegal_override;
			pml2[index] = current_phys_addr | PML2E_2MiB_BIT | present_bit;
			current_pages -= 512;
			params.pages -= 512;
			phys_range.advance(512);
			current_phys_addr += 4096 * 512;
			virtual_ += 4096 * 512;
		}
		else
		{
			auto subcall_result = process_page_table();
			if (subcall_result != mmap_result::ok)
				return subcall_result;
		}
	}
	return mmap_result::ok;
}

template<address_range Range>
mmap_result::type remap_range_pmlX(
	uint64_t pm_ptr_raw,
	mapping_params_t& mapping_params,
	Range& phys_range,
	const pm_descend_params_t<Range>* descend_params
	)
{
	auto& virtual_ = mapping_params.virtual_addr;
	auto& pages = mapping_params.pages;

	uint32_t index = (virtual_ >> descend_params->address_shift) & 511;
	const uint32_t entries = (uint32_t)(
		(pages + descend_params->pages_per_pm - 1) >> descend_params->pages_per_pm_log2);
	const uint32_t index_end = min<uint32_t>(index + entries, 512);

	auto* const pm = (uint64_t*)pm_ptr_raw;

	while (index != index_end)
	{
		const uint64_t next_pm = get_pme(pm[index]);
		auto subcall_result = descend_params->next_function(
			next_pm,
			mapping_params,
			phys_range,
			descend_params->next_params
		);
		if (subcall_result != mmap_result::ok)
			return subcall_result;
		++index;
	}
	return mmap_result::ok;
}

template<address_range Range>
mmap_result::type _remap_virtual_range1(
	uint64_t virtual_, uint32_t pages, 
	Range& phys_range,
	uint64_t flags
)
{
	if (virtual_ & 4095)
		return mmap_result::unaligned_base;
	if (flags & ~MMAP_FLAGS_MASK)
		return mmap_result::illegal_flags;
	
	pm_descend_params_t<Range> pml3_params;
	pml3_params.address_shift = 12 + 2 * 9;
	pml3_params.next_function = remap_range_pml2<Range>;
	pml3_params.pages_per_pm = 512 * 512 * 512;
	pml3_params.pages_per_pm_log2 = 3 * 9;
	
	pm_descend_params_t<Range> pml4_params;
	pml4_params.address_shift = 12 + 3 * 9;
	pml4_params.pages_per_pm = (uint64_t)512 * 512 * 512 * 512;
	pml4_params.pages_per_pm_log2 = 4 * 9;
	pml4_params.next_params = &pml3_params;
	pml4_params.next_function = remap_range_pmlX<Range>;

	mapping_params_t mapping_params;
	mapping_params.virtual_addr = virtual_;
	mapping_params.pages = pages;
	mapping_params.override_allowed = flags & MMAP_ALLOW_ADDRESS_OVERRIDE;
	mapping_params.flags = flags & PME_FLAGS_MASK;
	
	return remap_range_pmlX(0x30000, mapping_params, phys_range, &pml4_params);
}




mmap_result::type map_virtual_range(
	uint64_t virtual_address,
	uint64_t physical_address,
	uint32_t pages,
	uint64_t flags
)
{
	struct
	{
		uint64_t delta;
		void get(uint64_t virtual_address, uint64_t& physical_address, uint32_t&)
		{
			physical_address = virtual_address + delta;
		}
		void advance(uint32_t) {}
	} continious_range{ physical_address - virtual_address };

	return _remap_virtual_range1(virtual_address, pages, continious_range, flags);
}



mmap_result::type update_virtual_range_flags(
	uint64_t virtual_address,
	uint32_t pages,
	uint64_t flags
)
{
	struct
	{
		void get(uint64_t, uint64_t&, uint32_t&) {}
		void advance(uint32_t) {}
	} noop_mapper;

	return _remap_virtual_range1(virtual_address, pages, noop_mapper, flags);
}
*/
