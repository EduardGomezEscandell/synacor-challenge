#pragma once

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <array>
#include <ostream>

#include "word.h"
#include "address.h"

class Memory
{
    static constexpr std::uint_least16_t address_space = 0x8000;

    constexpr void AssertValidAddress(
        [[maybe_unused]] const raw_word_t raw_ptr) const noexcept
    {
#ifndef DNDEBUG
        if(raw_ptr > 0 && raw_ptr < address_space) return;

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
        return dereference(ptr.get().raw());
    }

    constexpr Word const& dereference(const Address ptr) const
    {
        return dereference(ptr.get().raw());
    }


public:
    constexpr Memory()
    {
        std::fill(m_data.begin(), m_data.end(), static_cast<raw_word_t>(0));
    }

    void load(std::basic_istream<raw_word_t>& in)
    {
        Address ptr;

        while(!in.eof())
        {
            dereference(ptr++) = in.get();
        }
    };

    constexpr Word& operator[](const Address ptr)
    {
        return dereference(ptr);
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