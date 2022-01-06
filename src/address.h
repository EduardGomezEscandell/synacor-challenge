#pragma once

#include "word.h"

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

    constexpr Address(auto in = 0) noexcept
    {
        m_internal = in;
    }

    constexpr Address(Address const& ptr) noexcept = default;

    constexpr auto& get() noexcept { return m_internal; }
    constexpr auto const& get() const noexcept { return m_internal; }

    constexpr auto operator++() noexcept
    {
    }

    constexpr auto operator++(int) noexcept
    {
        Address const ret = *this;

        ++(*this);

        return ret;
    }

    constexpr std::strong_ordering operator<=>(Address const& other) const noexcept
    {
        return this->m_internal <=> other.m_internal;
    }

private:
    Word m_internal = 0;
};
