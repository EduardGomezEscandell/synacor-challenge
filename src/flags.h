#pragma once

#include <cstdint>
#include <concepts>

struct Flags
{
    using flag_storage_t = std::uint8_t;

    enum flags_t : flag_storage_t
    {
        NONE             = 0b00000000,  // No flags
        HALTED           = 0b00000001,  // Prgroma must halt
        ERROR            = 0b00000010,  // There is an error
        BAD_INTEGER      = 0b00000100,  // Integer larger than max_word
        STACK_UNDERFLOW  = 0b00001000,  // Attempted to pop empty stack
        WRITE_ON_LITERAL = 0b00010000,  // Attempted to write on a literal (example: SET 23 15 ; expected register, got 23)
    };

    constexpr Flags(flag_storage_t state = NONE)
        : m_flags(state)
    {};

    constexpr void Set(flag_storage_t const flag) noexcept
    {
        m_flags = m_flags | flag;
    }

    constexpr void UnSet(flag_storage_t const flag) noexcept
    {
        m_flags = m_flags & (~flag);
    }

    constexpr bool Is(flag_storage_t const flag) const noexcept
    {
        return (m_flags & flag) != 0;
    }

    flag_storage_t m_flags;
};