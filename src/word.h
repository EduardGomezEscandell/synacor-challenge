#pragma once

#include <compare>
#include <cstdint>
#include <ostream>
#include <concepts>


using raw_byte_t = std::uint8_t;
using raw_word_t = std::uint16_t;

class Word
{
public:
    enum byte{LO, HI};

    raw_byte_t data[2] = {0, 0};

    constexpr Word() = default;
    constexpr Word(Word const& word) = default;
    constexpr Word(raw_byte_t in) noexcept;
    constexpr Word(std::integral auto in) noexcept;

    constexpr Word& operator=(Word const& word) = default;

    constexpr raw_byte_t& lo() noexcept { return data[LO];};
    constexpr raw_byte_t& hi() noexcept { return data[HI];};

    constexpr raw_byte_t lo() const noexcept { return data[LO];};
    constexpr raw_byte_t hi() const noexcept { return data[HI];};

    constexpr Word& operator++() noexcept;
    constexpr Word operator++(int) noexcept;
    constexpr Word& operator+=(raw_word_t jump) noexcept;
    constexpr Word operator+(Word const& other) const noexcept;

    constexpr std::strong_ordering operator<=>(Word const& other) const noexcept;
    
    constexpr bool operator<(Word const& other) const noexcept = default;
    constexpr bool operator<=(Word const& other) const noexcept = default;
    constexpr bool operator==(Word const& other) const noexcept = default;
    constexpr bool operator!=(Word const& other) const noexcept = default;
    constexpr bool operator>=(Word const& other) const noexcept = default;
    constexpr bool operator>(Word const& other) const noexcept = default;

    constexpr Word flip() const noexcept;

    constexpr raw_word_t get_raw() const noexcept
    {
        return (static_cast<raw_word_t>(lo()) << 8) | hi();
    }

    constexpr void set_raw(raw_word_t inp) noexcept
    {
        data[LO] = inp & 0x00FF;
        data[HI] = inp >> 8;
    }
    
    constexpr void set_raw(raw_byte_t lo, raw_byte_t hi) noexcept
    {
        data[LO] = lo;
        data[HI] = hi;
    }

    constexpr raw_word_t to_int() const noexcept
    {
        return (static_cast<raw_word_t>(hi()) << 8) | lo();
    }

};

constexpr Word::Word(raw_byte_t in) noexcept
{
    lo() = in;
}

constexpr Word::Word(std::integral auto in) noexcept
{
    lo() = (in & 0x00FF);
    hi() = (in >> 8) & 0x00FF;
}

constexpr Word& Word::operator+=(const raw_word_t jump) noexcept
{
    const raw_byte_t jump_lo = jump & 0x00FF;
    const raw_byte_t jump_hi = jump >> 8;

    if(lo() > (0x00FF-jump_lo))
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
    if(this->hi() < other.hi()) return std::strong_ordering::less;
    if(this->hi() > other.hi()) return std::strong_ordering::greater;

    if(this->lo() < other.lo()) return std::strong_ordering::less;
    if(this->lo() > other.lo()) return std::strong_ordering::greater;

    return std::strong_ordering::equivalent;
}

constexpr Word Word::flip() const noexcept
{
    Word out;
    out.lo() = this->hi();
    out.hi() = this->lo();
    return out;
}

inline std::ostream& operator<<(std::ostream& os, Word const& in)
{
    const char ascii = static_cast<char>(in.get_raw());
    return os << ascii;
}