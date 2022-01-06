#include "doctest/doctest.h"
#include "address.h"

TEST_CASE("Address")
{
    SUBCASE("Constructor")
    {
        constexpr Address ptr_1 = 0x1F2A;
        CHECK_EQ(ptr_1.get().to_int(), 0x1F2A);

        constexpr Word w = 0xDEDA;
        constexpr auto ptr_2 = Address(w);

        CHECK_EQ(ptr_2.get(), w);
        REQUIRE_EQ(ptr_2.get().to_int(), 0xDEDA);

        constexpr Address ptr_3 = ptr_2;
        CHECK_EQ(ptr_2, ptr_3);
        REQUIRE_EQ(ptr_2.get().to_int(), 0xDEDA);
    }

    SUBCASE("Addition")
    {
        Address ptr = 0x163;
        ++ptr;
        CHECK_EQ(ptr.get().to_int(), 0x164);
        CHECK_EQ((ptr++).get().to_int(), 0x164);
        CHECK_EQ(ptr.get().to_int(), 0x165);

        ptr += 5;

        CHECK_EQ(ptr.get().to_int(), 0x16A);
    }
}