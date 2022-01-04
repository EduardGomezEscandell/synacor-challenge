#include <cstdint>

class Flags
{
using flag_storage_t = uint_least8_t;

public:
	enum flags_t
	{
		NONE   = 0x00,
		HALTED = 0x01,
		PRINT  = 0X02,
		ALL = ~static_cast<flag_storage_t>(0)
	};

	Flags(flags_t state = NONE)
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