#pragma once

#include <compare>
#include <cstdint>
#include <concepts>

#include <iostream>
#include <iomanip>
#include <sstream>


using raw_byte_t = std::uint8_t;
using raw_word_t = std::uint16_t;

class Word
{
public:
    enum byte{LO, HI};
    static constexpr raw_word_t max_word = 0x8000;

    raw_byte_t data[2] = {0, 0};

    constexpr Word() = default;
    constexpr Word(const bool in) noexcept {lo() = in; }
    constexpr Word(Word const& word) = default;

    constexpr Word(raw_byte_t in) noexcept { lo() = in; }

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

    constexpr Word& operator--() noexcept;
    constexpr Word operator--(int) noexcept;
    constexpr Word& operator-=(raw_word_t jump) noexcept;
    constexpr Word operator-(Word const& other) const noexcept;

    constexpr Word& operator*=(Word const& other) noexcept;
    constexpr Word operator*(Word const& other) const noexcept;

    constexpr Word& operator%=(Word const& other) noexcept;
    constexpr Word operator%(Word const& other) const noexcept;

    constexpr Word& operator&=(Word const& other) noexcept;
    constexpr Word operator&(Word const& other) const noexcept;
    
    constexpr Word& operator|=(Word const& other) noexcept;
    constexpr Word operator|(Word const& other) const noexcept;

    constexpr Word operator~() const noexcept;

    constexpr std::strong_ordering operator<=>(Word const& other) const noexcept;
    
    constexpr bool operator<(Word const& other) const noexcept = default;
    constexpr bool operator<=(Word const& other) const noexcept = default;
    constexpr bool operator==(Word const& other) const noexcept = default;
    constexpr bool operator!=(Word const& other) const noexcept = default;
    constexpr bool operator>=(Word const& other) const noexcept = default;
    constexpr bool operator>(Word const& other) const noexcept = default;

    constexpr bool is_zero() const noexcept
    {
        return hi() == 0 && lo() == 0;
    }

    constexpr Word flip() const noexcept;

    constexpr raw_word_t get_raw() const noexcept
    {
        return (static_cast<raw_word_t>(lo()) << 8) | hi();
    }

    constexpr Word& set_raw(raw_word_t inp) noexcept
    {
        data[LO] = inp & 0x00FF;
        data[HI] = inp >> 8;
        return *this;
    }
    
    constexpr Word& set_raw(raw_byte_t lo, raw_byte_t hi) noexcept
    {
        data[LO] = lo;
        data[HI] = hi;
        return *this;
    }

    constexpr raw_word_t to_int() const noexcept
    {
        return (static_cast<raw_word_t>(hi()) << 8) | lo();
    }

    std::string hex_dump() const
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(4) << get_raw() + 0u;
        return ss.str();
    }

};



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

    hi() %= (max_word>>8);

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

constexpr Word& Word::operator-=(const raw_word_t jump) noexcept
{
    const raw_byte_t jump_lo = jump & 0x00FF;
    const raw_byte_t jump_hi = jump >> 8;

    if(lo() < jump_lo)
    {
        --hi();
    }

    lo() -= jump_lo;
    hi() -= jump_hi;

    hi() %= (max_word>>8);

    return *this;
}

constexpr Word& Word::operator--() noexcept
{
    return (*this) -= 1u;
}

constexpr Word Word::operator--(int) noexcept
{
    const Word copy = *this;
    --(*this);
    return copy;
}

constexpr Word Word::operator-(Word const& other) const noexcept
{
    return Word(*this) -= other.to_int();
}

constexpr Word& Word::operator*=(Word const& other) noexcept
{
    return *this = (this->to_int() * other.to_int()) % max_word;
}

constexpr Word Word::operator*(Word const& other) const noexcept
{
    return Word(*this) *= other;
}

constexpr Word& Word::operator%=(Word const& other) noexcept
{
    return *this = (this->to_int() % other.to_int());
}

constexpr Word Word::operator%(Word const& other) const noexcept
{
    return Word(*this) %= other;
}


constexpr Word& Word::operator&=(Word const& other) noexcept
{
    this->data[LO] &= other.data[LO];
    this->data[HI] &= other.data[HI];
    return *this;
}

constexpr Word Word::operator&(Word const& other) const noexcept
{
    return Word(*this) &= other;
}

constexpr Word& Word::operator|=(Word const& other) noexcept
{
    this->data[LO] |= other.data[LO];
    this->data[HI] |= other.data[HI];
    return *this;
}

constexpr Word Word::operator|(Word const& other) const noexcept
{
    return Word(*this) |= other;
}

constexpr Word Word::operator~() const noexcept
{
    return Word((~ this->to_int()) & (max_word-1));
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