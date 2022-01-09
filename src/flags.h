#include <cstdint>
#include <concepts>

class Flags
{
public:
    using flag_storage_t = std::uint8_t;

    enum flags_t : flag_storage_t
    {
        NONE            = 0b00000001,
        HALTED          = 0b00000010,
        ERROR           = 0b00000100,
        BAD_INPUT       = 0b00001000,
        STACK_UNDERFLOW = 0b00010000
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

private:
    flag_storage_t m_flags;
};