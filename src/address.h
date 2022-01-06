#pragma once

#include "word.h"
#include <concepts>

/**
 * This class exists solely to prevent implicit conversions
 * between literals and adresses
 */
class Address
{
public:
    constexpr Address() noexcept = default;

    constexpr Address(Word::half_word_t in) noexcept
    {
        m_internal.lo() = in;
    }

    constexpr explicit Address(Word const& word) noexcept
    {
        m_internal = word;
    }

    constexpr Address(std::integral auto  in) noexcept
    {
        m_internal = in;
    }

    constexpr Address(Address const& ptr) noexcept = default;

    constexpr auto& get() noexcept { return m_internal; }
    constexpr auto const& get() const noexcept { return m_internal; }

    constexpr Address& operator+=(std::integral auto in) noexcept
    {
        m_internal += in;
        return *this;
    }

    constexpr Address operator+(std::integral auto in) noexcept
    {
        return Address(*this) += in;
    }
    
    constexpr Address operator++() noexcept
    {
        return (*this) += 1;
    }

    constexpr Address operator++(int) noexcept
    {
        const auto copy = *this;
        ++(*this);
        return copy;
    }

    constexpr std::strong_ordering operator<=>(Address const& other) const noexcept
    {
        return this->m_internal <=> other.m_internal;
    }

    constexpr bool operator==(Address const&) const noexcept = default;
    constexpr bool operator!=(Address const&) const noexcept = default;

private:
    Word m_internal = 0;
};
