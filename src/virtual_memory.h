#pragma once

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <array>
#include <ostream>
#include <sys/types.h>

#include "word.h"
#include "instruction.h"
#include "address.h"

class Memory
{
    static constexpr raw_word_t address_space = Word::max_word;

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

    void load(program_file_t& source, Address& load_ptr)
    {
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
            
            dereference(load_ptr++).set_raw(lo, hi);
        }
    };

    constexpr Word& operator[](auto const& ptr)
    {
        return dereference(ptr);
    }

    constexpr Word const& operator[](auto const& ptr) const
    {
        return dereference(ptr);
    }

    void hex_dump(const std::size_t row_begin, const std::size_t row_end, const std::size_t highlight = address_space+1) const
    {
        constexpr std::size_t row_size = 0x08;

        for(std::size_t i = row_begin*row_size; i != row_end*row_size; i += row_size)
        {
            std::cout << std::hex << std::setfill('0') << std::setw(4) << i;
            std::cout << ':';

            for(std::size_t j=0; j < row_size; ++j)
            {
                const auto address = i + j;
                
                if(address == highlight)    std::cout << '[';
                else                        std::cout << ' ';

                std::cout << m_data[address].hex_dump();

                if(address == highlight)    std::cout << ']';
                else                        std::cout << ' ';

            }

            std::cout << " | ";

            for(std::size_t j=0; j < row_size; ++j)
            {
                const auto lo = m_data[i+j].lo();
                if(lo > 32)
                    std::cout << static_cast<char>(lo);
                else
                    std::cout << '.';
            }

            std::cout << '\n';
        }
        std::cout << '\n';
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