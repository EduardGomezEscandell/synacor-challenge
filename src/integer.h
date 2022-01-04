#pragma once

#include <compare>
#include <cstdint>
#include <ostream>
#include <system_error>
#include <type_traits>

class Word 
{
public:
    using half_word_t = std::uint8_t;
    using word_t = std::uint16_t;
    enum byte{HI, LO};
    
    /**
     * Class to pretend to pass references to the high or low byte
     */
    template<byte B>
    class HalfWordProxy
    {
    
    public:
        constexpr HalfWordProxy(Word& base)
            : m_base(base)
        {}

        constexpr half_word_t get() const noexcept;
        constexpr void set(half_word_t) noexcept;

        template<byte Tb>
        constexpr std::strong_ordering operator<=>(HalfWordProxy<Tb> const& other)
        {
            return this->get() <=> other.get();
        }

        constexpr std::strong_ordering operator<=>(half_word_t other)
        {
            return this->get() <=> other;
        }

        constexpr std::strong_ordering operator<=>(int other)
        {
            return this->get() <=> (other & 0xF);
        }

        constexpr auto operator++() noexcept
        {
            this->set(this->get() + 1);
            return *this;
        }

        constexpr auto operator++(int) noexcept
        {
            auto copy = *this;
            ++(*this);
            return copy;
        }

        template<byte Tb>
        constexpr auto operator=(HalfWordProxy<Tb> value) noexcept
        {
            return set(value.get());
        }

        constexpr auto operator=(half_word_t value) noexcept
        {
            return set(value);
        }

        constexpr auto operator=(int value) noexcept
        {
            return set(value & 0xF);
        }

    private:
        Word& m_base;
    };

    word_t raw = 0;

    constexpr Word() = default;
    constexpr Word(half_word_t in) noexcept;
    constexpr Word(auto in) noexcept;

    constexpr HalfWordProxy<LO> lo() noexcept { return *this;};
    constexpr HalfWordProxy<LO> hi() noexcept { return *this;};

    constexpr half_word_t get_lo() const noexcept;
    constexpr half_word_t get_hi() const noexcept;

    constexpr Word operator++() noexcept;
    constexpr std::strong_ordering operator<=>(Word const& other) const noexcept;

    constexpr Word flip() const noexcept;

};

template<>
constexpr void Word::HalfWordProxy<Word::HI>::set(half_word_t value) noexcept
{
    m_base.raw = m_base.raw | 0x0F;
    word_t const augmented_value = 0xF0 | value;
    m_base.raw = m_base.raw & augmented_value;
}

template<>
constexpr void Word::HalfWordProxy<Word::LO>::set(half_word_t value) noexcept
{
    m_base.raw = m_base.raw | 0xF0;
    word_t const augmented_value = (static_cast<word_t>(value) << 8) | 0x0F;
    m_base.raw = m_base.raw & augmented_value;
}

template<>
constexpr Word::half_word_t Word::HalfWordProxy<Word::HI>::get() const noexcept
{
    return m_base.raw & 0x0F;
}

template<>
constexpr Word::half_word_t Word::HalfWordProxy<Word::LO>::get() const noexcept
{
    return (m_base.raw & 0xF0) >> 8;
}

constexpr Word::half_word_t Word::get_lo() const noexcept
{
    // TODO: Create ConstProxy instead of this ugly fix
    return HalfWordProxy<Word::LO>(*const_cast<Word*>(this)).get();
}

constexpr Word::half_word_t Word::get_hi() const noexcept
{
    // TODO: Create ConstProxy instead of this ugly fix
    return HalfWordProxy<Word::HI>(*const_cast<Word*>(this)).get();
}


constexpr Word::Word(Word::half_word_t in) noexcept
{
    lo() = in;
}

constexpr Word::Word(auto in) noexcept
{
    lo() = (in & 0x0F);
}

constexpr Word Word::operator++() noexcept
{
    if((lo() <=> 0xF) == std::strong_ordering::equivalent)
    {
        ++hi();
        lo() = 0;
    }
    else
    {
        ++lo();
    }

    return *this;
}


constexpr std::strong_ordering Word::operator<=>(Word const& other) const noexcept
{
    if(this->get_hi() < other.get_hi()) return std::strong_ordering::less;
    if(this->get_hi() > other.get_hi()) return std::strong_ordering::greater;

    if(this->get_lo() < other.get_lo()) return std::strong_ordering::less;
    if(this->get_lo() > other.get_lo()) return std::strong_ordering::greater;

    return std::strong_ordering::equivalent;
}

constexpr Word Word::flip() const noexcept
{
    Word out;
    out.lo() = this->get_hi();
    out.hi() = this->get_lo();
    return out;
}

std::ostream& operator<<(std::ostream& os, Word const& in)
{
    const char ascii = static_cast<char>(in.flip().raw);
    return os << ascii;
}


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

    constexpr Address(Address const& ptr) noexcept
    {
        m_internal.raw = ptr.m_internal.raw;
    }

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
