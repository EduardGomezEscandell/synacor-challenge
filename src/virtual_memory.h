#pragma once

#include <cstddef>
#include <cstdint>

#include <fstream>
#include <array>

#include "integer.h"

class Memory
{
    static constexpr std::uint_least16_t size = 0xEF;

    constexpr Word& dereference(Word::word_t raw_ptr)
    {
        return m_data[raw_ptr];
    }

    constexpr Word& dereference(Address const& ptr)
    {
        return dereference(ptr.get().flip().raw);
    }

    constexpr Word const& dereference(Word::word_t raw_ptr) const
    {
        return m_data[raw_ptr];
    }

    constexpr Word const& dereference(Address const& ptr) const
    {
        return dereference(ptr.get().flip().raw);
    }


public:
    constexpr Memory()
    {
        std::fill(m_data.begin(), m_data.end(), static_cast<Word::word_t>(0));
    }

    void from_file(std::fstream& in)
    {
        Address ptr;

        while(!in.eof())
        {
            dereference(ptr).lo() = in.eof() ? 0 : in.get();
            dereference(ptr).hi() = in.eof() ? 0 : in.get();
            ++ptr;
        }
    };


    constexpr Word& operator[](Word ptr)
    {
        return dereference(ptr);
    }


private:
    std::array<Word, size> m_data;

    static constexpr Address first = 0;
    static constexpr Address last = size-1;
};