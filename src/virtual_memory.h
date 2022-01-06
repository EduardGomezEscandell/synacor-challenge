#pragma once

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <array>
#include <ostream>
#include <sys/types.h>

#include "word.h"
#include "address.h"

class Memory
{
    static constexpr std::uint_least16_t address_space = 0x8000;

    constexpr void AssertValidAddress(
        [[maybe_unused]] const raw_word_t raw_ptr) const noexcept
    {
#ifndef DNDEBUG
        if(raw_ptr < address_space) return;

        std::cerr << "Invalid access to address " << raw_ptr << std::endl;
        exit(EXIT_FAILURE);
#endif
    }

    constexpr Word& dereference(const raw_word_t raw_ptr)
    {
        AssertValidAddress(raw_ptr);
        return m_data[raw_ptr];
    }

    constexpr Word const& dereference(const raw_word_t raw_ptr) const
    {
        AssertValidAddress(raw_ptr);
        return m_data[raw_ptr];
    }

    constexpr Word& dereference(const Address ptr)
    {
        return dereference(ptr.get().to_int());
    }

    constexpr Word const& dereference(const Address ptr) const
    {
        return dereference(ptr.get().to_int());
    }


public:
    using program_file_t = std::ifstream;

    constexpr Memory()
    {
        std::fill(m_data.begin(), m_data.end(), static_cast<raw_word_t>(0));
    }

    void load(program_file_t& source)
    {
        Address ptr;
        raw_byte_t lo;
        raw_byte_t hi;
        char val;

        while(true)
        {
            val = source.get();
            lo = *reinterpret_cast<raw_byte_t*>(&val);

            val = source.get();
            hi = *reinterpret_cast<raw_byte_t*>(&val);

            if(source.eof()) break;
            
            dereference(ptr++).set_raw(lo, hi);
        }
    };

    constexpr Word& operator[](auto const& ptr)
    {
        return dereference(ptr);
    }

    void hex_dump(const size_t row_begin, const size_t row_end) const
    {
        constexpr size_t row_size = 0x08;

        for(size_t i = row_begin*row_size; i != row_end*row_size; i += row_size)
        {
            std::cout << std::hex << std::setfill('0') << std::setw(4) << i;
            std::cout << ':';

            for(size_t j=0; j < row_size; ++j)
            {
                std::cout << ' ';
                std::cout << std::hex << std::setfill('0') << std::setw(4)
                          << m_data[i + j].get_raw() + 0u;
            }

            std::cout << '\n';
        }
    }

private:
    std::array<Word, address_space> m_data;

    static constexpr Address first = 0;
    static constexpr Address last = address_space-1;
};

inline const std::ostream& operator<<(std::ostream& os, Address const& address)
{
    return os << address.get();
}