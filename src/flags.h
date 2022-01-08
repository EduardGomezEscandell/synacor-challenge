#include <cstdint>
class Flags
{
using flag_storage_t = std::int8_t;

public:
	enum flags_t : flag_storage_t
	{
		NONE   = 0x00,
		HALTED = 0x01,
		ERROR  = 0X02,
		BAD_INPUT = 0x04
	};

	constexpr Flags(flags_t state = NONE)
		: m_flags(state)
	{};

	constexpr void Set(flags_t const flag) noexcept
	{
		m_flags = m_flags | flag;
	}

	constexpr void UnSet(flags_t const flag) noexcept
	{
		m_flags = m_flags & (~flag);
	}

	constexpr bool Is(flags_t const flag) const noexcept
	{
		return (m_flags & flag) != 0;
	}

private:
	flag_storage_t m_flags;
};