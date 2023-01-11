
#ifndef _FARPTR_HPP_
#define _FARPTR_HPP_

#include <stdint.h>
#include <stddef.h>
#include "loader.hpp"

extern "C" uint32_t far_read(uint16_t segment, uint16_t offset);
extern "C" void far_write(uint16_t segment, uint16_t offset, uint32_t value);

template<class T>
class FarPtr;

template<class T>
class FarRef
{
	friend class FarPtr<T>;

	uint16_t m_offset;
	uint16_t m_segment;

	FarRef(uint16_t off, uint16_t seg)
		: m_offset(off), m_segment(seg)
	{}

public:
	FarRef<T>& operator=(T x)
	{
		uint32_t buff = far_read(this->m_segment, this->m_offset);
		memcpy(&buff, &x, sizeof(T));
		far_write(this->m_segment, this->m_offset, buff);
		return *this;
	}
	operator T() const
	{
		uint32_t buff = far_read(this->m_segment, this->m_offset);
		return *(T*)&buff;
	}

	FarPtr<T> operator&() const noexcept;
};

template<class T>
class __attribute__((packed)) FarPtr
{
	static_assert(sizeof(T) == 1
		|| sizeof(T) == 2
		|| sizeof(T) == 4
	, "");
	uint16_t m_offset;
	uint16_t m_segment;

	using my_t = FarPtr<T>;

public:
	FarPtr()
		: m_offset(0), m_segment(0)
	{}
	FarPtr(uint16_t segment, uint16_t offset)
		: m_offset(offset), m_segment(segment)
	{}

	static FarPtr<T> convert(T* ptr)
	{
		FarPtr<T> result;
		uint32_t p = (uint32_t)ptr;
		result.m_segment = (uint16_t)(p / 16);
		result.m_offset = (uint16_t)(p % 16);
		return result;
	}

	FarRef<T> operator*() const noexcept
	{
		return { this->m_offset, this->m_segment };
	}
	FarRef<T> operator[](ptrdiff_t n) const noexcept
	{
		return { this->m_offset + n * sizeof(T), this->m_segment };
	}
	my_t& operator++()
	{
		this->m_offset += sizeof(T);
		return *this;
	}
	my_t& operator--()
	{
		this->m_offset -= sizeof(T);
		return *this;
	}
	my_t operator++(int)
	{
		auto copy = *this;
		++(*this);
		return copy;
	}
	my_t operator--(int)
	{
		auto copy = *this;
		--(*this);
		return copy;
	}
	my_t& operator+=(ptrdiff_t n)
	{
		this->m_offset += n * sizeof(T);
		return *this;
	}
	my_t& operator-=(ptrdiff_t n)
	{
		this->m_offset -= n * sizeof(T);
		return *this;
	}
};

template <class T>
inline FarPtr<T> FarRef<T>::operator&() const noexcept
{
	return FarPtr<T>(this->m_segment, this->m_offset);
}

#endif //!_FARPTR_HPP_
