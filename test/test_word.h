#include "doctest/doctest.h"
#include "word.h"

TEST_CASE("Word -- bitewise")
{
    SUBCASE("Low byte")
    {
        Word w;

        w.lo() = 0x35;

        CHECK_EQ(w.lo(), 0x35);
        CHECK_EQ(w.get_raw(), 0x3500);
        CHECK_EQ(w.to_int(), 0x0035);
    }

    SUBCASE("High byte")
    {
        Word w;
        w.hi() = 0x27;

        CHECK_EQ(w.hi(), 0x27);
        CHECK_EQ(w.get_raw(), 0x0027);
        CHECK_EQ(w.to_int(), 0x2700);
    }

    SUBCASE("Both bytes")
    {
        Word w;
        w.hi() = 0x13;
        w.lo() = 0x47;

        CHECK_EQ(w.hi(), 0x13);
        CHECK_EQ(w.lo(), 0x47);
        CHECK_EQ(w.get_raw(), 0x4713);
        CHECK_EQ(w.to_int(), 0x1347);
    }
    
}

TEST_CASE("Word -- constructors")
{
    SUBCASE("Default")
    {
        constexpr Word w;

        static_assert(w.get_raw() == 0, "Test failed at compile-time!");

        CHECK_EQ(w.get_raw(), 0);
    }

    SUBCASE("From integer")
    {
        constexpr Word w = 0xABCD;

        static_assert(w.to_int() == 0xABCD);

        CHECK_EQ(w.to_int(), 0xABCD);
    }

    SUBCASE("From other word")
    {
        constexpr Word w1 = 0xBEEF;
        constexpr Word w2 = w1;

        static_assert(w2.to_int() == w1.to_int());
        static_assert(w1.to_int() == 0xBEEF);
        static_assert(w2.to_int() == 0xBEEF);

        CHECK_EQ(w1.to_int(), 0xBEEF);
        CHECK_EQ(w1.get_raw(), 0xEFBE);
        
        CHECK_EQ(w2.to_int(), 0xBEEF);
        CHECK_EQ(w2.get_raw(), 0xEFBE);
    }
}

TEST_CASE("Word -- comparisons")
{
    constexpr Word A = 0x0135;
    constexpr Word B = 0x03D8;
    constexpr Word C = 0x03D8;
    constexpr Word D = 0x0F11;

    SUBCASE("Equality") {
        REQUIRE(B == C);
        REQUIRE(!(B != C));
    }

    SUBCASE("Inequality") {
        REQUIRE(A != B);
        REQUIRE(!(A == B));
    }

    SUBCASE("LT") {
        REQUIRE(A < B);
        REQUIRE(A <= B);
        REQUIRE(B <= C);
        REQUIRE(C <= B);
        REQUIRE(!(D < C));
    }

    SUBCASE("GT") {
        REQUIRE(B > A);
        REQUIRE(B >= A);
        REQUIRE(C >= B);
        REQUIRE(B >= C);
        REQUIRE(!(C > D));
    }
}

TEST_CASE("Word -- addition and subtraction")
{
    SUBCASE("Scalar increment")
    {
        Word w = 0x123;
        ++w;
        CHECK_EQ(w.to_int(), 0x124);
        CHECK_EQ((w++).to_int(), 0x124);
        CHECK_EQ(w.to_int(), 0x125);
    }

    SUBCASE("Scalar increment with overflow")
    {
        Word w = 0x12FF;
        ++w;
        CHECK_EQ(w.to_int(), 0x1300);
    }

    SUBCASE("Scalar addition")
    {
        Word w = 0x123;
        w += 0x101;
        CHECK_EQ(w.to_int(), 0x224);
    }

    SUBCASE("Scalar addition with overflow")
    {
        Word w = 0x12FE;
        w += 0x130;
        CHECK_EQ(w.to_int(), 0x142E);
    }

    SUBCASE("Word addition")
    {
        constexpr Word w = 0x1234;
        constexpr Word x = 0x4321;

        CHECK_EQ(w + x, 0x5555);
        CHECK_EQ(w, 0x1234);
        CHECK_EQ(x, 0x4321);
    }

    SUBCASE("Word addition with overflow")
    {
        constexpr Word w = 0x1234;
        constexpr Word x = 0x01FF;

        CHECK_EQ(w + x, 0x1433);
    }

    SUBCASE("Scalar decrement")
    {
        Word w = 0x123;
        --w;
        CHECK_EQ(w.to_int(), 0x122);
        CHECK_EQ((w--).to_int(), 0x122);
        CHECK_EQ(w.to_int(), 0x121);
    }

    SUBCASE("Scalar decrement with underflow")
    {
        Word w = 0x0000;
        --w;
        CHECK_EQ(w.to_int(), 0x7FFF);
    }

    SUBCASE("Scalar subtraction")
    {
        Word w = 0x123;
        w -= 0x101;
        CHECK_EQ(w.to_int(), 0x022);
    }

    SUBCASE("Scalar subtraction with underflow")
    {
        Word w = 0x1200;
        w -= 0x0002;
        CHECK_EQ(w.to_int(), 0x11FE);
    }

    SUBCASE("Word subtraction")
    {
        constexpr Word w = 0x4321;
        constexpr Word x = 0x1111;

        CHECK_EQ(w - x, 0x3210);
        CHECK_EQ(w, 0x4321);
        CHECK_EQ(x, 0x1111);
    }

    SUBCASE("Word subtraction with underflow")
    {
        constexpr Word w = 0x4321;
        constexpr Word x = 0x01FF;

        CHECK_EQ(w - x, 0x4122);
    }
}

TEST_CASE("Word -- product and modulo")
{
    SUBCASE("Product")
    {
        constexpr Word a = 0x0538;
        constexpr Word b = 0x0018;

        CHECK_EQ((a*b).to_int(), 0x7d40);
    }

    SUBCASE("Product with overflow")
    {
        constexpr Word a = 0x0538;
        constexpr Word b = 0x0318;

        CHECK_EQ((a*b).to_int(), 0x2540);
    }

    SUBCASE("Modulo")
    {
        constexpr Word a = 0x0538;
        constexpr Word b = 0x0318;

        CHECK_EQ((a%b).to_int(), 0x0220);
    }
}

TEST_CASE("Word -- bitwise arithmetic")
{
    SUBCASE("Flip")
    {
        Word w = 0xABCD;
        CHECK_EQ(w.flip().to_int(), 0xCDAB);
        CHECK_EQ(w.to_int(), 0xABCD);
    }

    SUBCASE("And")
    {
        constexpr Word w = 0xFF00;
        constexpr Word x = 0x1359;

        CHECK_EQ((w & x).to_int(), 0x1300);
    }

    SUBCASE("Or")
    {
        constexpr Word w = 0xFF00;
        constexpr Word x = 0x1359;

        CHECK_EQ((w | x).to_int(), 0xFF59);
    }

    SUBCASE("Not")
    {
        constexpr Word w = 0x0000;

        CHECK_EQ((~w).to_int(), 0x7FFF); // Remember: First bit is dead
    }
}