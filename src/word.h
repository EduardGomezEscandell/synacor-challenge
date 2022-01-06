#pragma once

#include <compare>
#include <cstdint>
#include <ostream>
#include <concepts>

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

        constexpr std::strong_ordering operator<=>(std::integral auto other)
        {
            return this->get() <=> (other & 0x00FF);
        }

        constexpr bool operator<(HalfWordProxy<B> const& other) const noexcept = default;
        constexpr bool operator<=(HalfWordProxy<B> const& other) const noexcept = default;
        constexpr bool operator==(HalfWordProxy<B> const& other) const noexcept = default;
        constexpr bool operator!=(HalfWordProxy<B> const& other) const noexcept = default;
        constexpr bool operator>=(HalfWordProxy<B> const& other) const noexcept = default;
        constexpr bool operator>(HalfWordProxy<B> const& other) const noexcept = default;

        constexpr auto& operator+=(half_word_t jump) noexcept
        {
            this->set(this->get() + jump);
            return *this;
        }

        constexpr auto& operator++() noexcept
        {
            return *this += 1;
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

        constexpr auto operator=(std::integral auto value) noexcept
        {
            return set(value & 0x00FF);
        }

    private:
        Word& m_base;
    };

    word_t raw = 0;

    constexpr Word() = default;
    constexpr Word(Word const& word) = default;
    constexpr Word(half_word_t in) noexcept;
    constexpr Word(std::integral auto in) noexcept;

    constexpr Word& operator=(Word const& word) = default;

    constexpr HalfWordProxy<LO> lo() noexcept { return *this;};
    constexpr HalfWordProxy<HI> hi() noexcept { return *this;};

    constexpr half_word_t get_lo() const noexcept;
    constexpr half_word_t get_hi() const noexcept;

    constexpr Word& operator++() noexcept;
    constexpr Word operator++(int) noexcept;
    constexpr Word& operator+=(word_t jump) noexcept;
    constexpr Word operator+(Word const& other) const noexcept;

    constexpr std::strong_ordering operator<=>(Word const& other) const noexcept;
    
    constexpr bool operator<(Word const& other) const noexcept = default;
    constexpr bool operator<=(Word const& other) const noexcept = default;
    constexpr bool operator==(Word const& other) const noexcept = default;
    constexpr bool operator!=(Word const& other) const noexcept = default;
    constexpr bool operator>=(Word const& other) const noexcept = default;
    constexpr bool operator>(Word const& other) const noexcept = default;

    constexpr Word flip() const noexcept;

    constexpr word_t to_int() const noexcept
    {
        return get_hi() << 8 | get_lo();
    }

};

template<>
constexpr void Word::HalfWordProxy<Word::HI>::set(half_word_t value) noexcept
{
    m_base.raw = m_base.raw | 0x00FF;
    word_t const augmented_value = 0xFF00 | value;
    m_base.raw = m_base.raw & augmented_value;
}

template<>
constexpr void Word::HalfWordProxy<Word::LO>::set(half_word_t value) noexcept
{
    m_base.raw = m_base.raw | 0xFF00;
    word_t const augmented_value = (static_cast<word_t>(value) << 8) | 0x00FF;
    m_base.raw = m_base.raw & augmented_value;
}

template<>
constexpr Word::half_word_t Word::HalfWordProxy<Word::HI>::get() const noexcept
{
    return m_base.raw & 0x00FF;
}

template<>
constexpr Word::half_word_t Word::HalfWordProxy<Word::LO>::get() const noexcept
{
    return m_base.raw >> 8;
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

constexpr Word::Word(std::integral  auto in) noexcept
{
    lo() = (in & 0x00FF);
    hi() = (in >> 8) & 0x00FF;
}

constexpr Word& Word::operator+=(const word_t jump) noexcept
{
    const half_word_t jump_lo = jump & 0x00FF;
    const half_word_t jump_hi = jump >> 8;

    if((lo() <=> (0x00FF-jump_lo)) == std::strong_ordering::greater)
    {
        hi() += 1;
    }

    lo() += jump_lo;
    hi() += jump_hi;

    return *this;
}

constexpr Word& Word::operator++() noexcept
{
    return (*this) += 1u;
}

constexpr Word Word::operator++(int) noexcept
{
    const Word copy = *this;
    ++(*this);
    return copy;
}

constexpr Word Word::operator+(Word const& other) const noexcept
{
    return Word(*this) += other.to_int();
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

inline std::ostream& operator<<(std::ostream& os, Word const& in)
{
    const char ascii = static_cast<char>(in.to_int());
    return os << ascii;
}