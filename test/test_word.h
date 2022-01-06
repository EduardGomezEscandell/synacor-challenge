#include "doctest/doctest.h"

#include "word.h"

TEST_CASE("Word -- constructors")
{
	SUBCASE("default")
	{
		Word w;

		CHECK_EQ(w.to_int(), 0);
	}
}