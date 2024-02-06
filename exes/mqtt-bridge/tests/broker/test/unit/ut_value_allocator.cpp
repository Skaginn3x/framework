// Copyright Takatoshi Kondo 2020
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "../common/test_main.hpp"
#include "../common/global_fixture.hpp"

#include <async_mqtt/util/value_allocator.hpp>

BOOST_AUTO_TEST_SUITE(ut_value_allocator)

namespace am = async_mqtt;

BOOST_AUTO_TEST_CASE( one ) {
    am::value_allocator<std::size_t> a{0, 0};
    BOOST_TEST(a.interval_count() == 1);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 0);
        BOOST_TEST(a.interval_count() == 0);
    }
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(!value_opt);
    }
    a.deallocate(0);
    BOOST_TEST(a.interval_count() == 1);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 0);
        BOOST_TEST(a.interval_count() == 0);
    }
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(!value_opt);
    }
    BOOST_TEST(a.use(0) == false);
    BOOST_TEST(a.use(1) == false);
    a.deallocate(0);
    BOOST_TEST(a.interval_count() == 1);
    BOOST_TEST(a.use(0) == true);
    BOOST_TEST(a.interval_count() == 0);
    BOOST_TEST(a.use(1) == false);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(!value_opt);
    }
    a.deallocate(0);
    BOOST_TEST(a.interval_count() == 1);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 0);
        BOOST_TEST(a.interval_count() == 0);
    }
}

BOOST_AUTO_TEST_CASE( offset ) {
    am::value_allocator<std::size_t> a{5, 5};
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 5);
    }
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(!value_opt);
    }
    a.deallocate(5);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 5);
    }
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(!value_opt);
    }
    BOOST_TEST(a.use(5) == false);
    BOOST_TEST(a.use(1) == false);
    a.deallocate(5);
    BOOST_TEST(a.use(5) == true);
    BOOST_TEST(a.use(1) == false);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(!value_opt);
    }
    a.deallocate(5);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 5);
    }
}

BOOST_AUTO_TEST_CASE( allocate ) {
    am::value_allocator<std::size_t> a{0, 4};
    BOOST_TEST(a.interval_count() == 1);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 0);
        BOOST_TEST(a.interval_count() == 1);
    }
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 1);
        BOOST_TEST(a.interval_count() == 1);
    }
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 2);
        BOOST_TEST(a.interval_count() == 1);
    }
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 3);
        BOOST_TEST(a.interval_count() == 1);
    }
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 4);
        BOOST_TEST(a.interval_count() == 0);
    }
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(!value_opt);
    }
    a.deallocate(2);
    BOOST_TEST(a.interval_count() == 1);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 2);
        BOOST_TEST(a.interval_count() == 0);
    }
}

BOOST_AUTO_TEST_CASE( use ) {
    am::value_allocator<std::size_t> a{0, 4};
    BOOST_TEST(a.interval_count() == 1);
    BOOST_TEST(a.use(1) == true);
    BOOST_TEST(a.interval_count() == 2);
    BOOST_TEST(a.use(3) == true);
    BOOST_TEST(a.interval_count() == 3);
    BOOST_TEST(a.use(2) == true);
    BOOST_TEST(a.interval_count() == 2);
    BOOST_TEST(a.use(0) == true);
    BOOST_TEST(a.interval_count() == 1);
    BOOST_TEST(a.use(4) == true);
    BOOST_TEST(a.interval_count() == 0);
    BOOST_TEST(a.use(0) == false);
    BOOST_TEST(a.use(1) == false);
    BOOST_TEST(a.use(2) == false);
    BOOST_TEST(a.use(3) == false);
    BOOST_TEST(a.use(4) == false);
    a.deallocate(2);
    BOOST_TEST(a.interval_count() == 1);
    BOOST_TEST(a.use(2) == true);
    BOOST_TEST(a.interval_count() == 0);
}

BOOST_AUTO_TEST_CASE( clear ) {
    am::value_allocator<std::size_t> a{0, 4};
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 0);
        BOOST_TEST(a.interval_count() == 1);
    }
    BOOST_TEST(a.use(1) == true);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 2);
        BOOST_TEST(a.interval_count() == 1);
    }
    BOOST_TEST(a.use(3) == true);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 4);
        BOOST_TEST(a.interval_count() == 0);
    }

    a.clear();
    BOOST_TEST(a.interval_count() == 1);

    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 0);
        BOOST_TEST(a.interval_count() == 1);
    }
    BOOST_TEST(a.use(1) == true);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 2);
        BOOST_TEST(a.interval_count() == 1);
    }
    BOOST_TEST(a.use(3) == true);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == 4);
        BOOST_TEST(a.interval_count() == 0);
    }
}

BOOST_AUTO_TEST_CASE( interval_management ) {
    am::value_allocator<std::size_t> a{0, 4};
    BOOST_TEST(a.use(0) == true);
    BOOST_TEST(a.use(1) == true);
    BOOST_TEST(a.use(2) == true);
    BOOST_TEST(a.use(3) == true);
    BOOST_TEST(a.use(4) == true);

    {
        auto ca = a;
        // fully allocated
        ca.deallocate(0);
        BOOST_TEST(ca.interval_count() == 1);
        // .....v no concat
        ca.deallocate(4);
        BOOST_TEST(ca.interval_count() == 2);
        // ..v.. no concat
        ca.deallocate(2);
        BOOST_TEST(ca.interval_count() == 3);
        // ..v.. concat both
        ca.deallocate(1);
        BOOST_TEST(ca.interval_count() == 2);
        // ..v.. concat both
        ca.deallocate(3);
        BOOST_TEST(ca.interval_count() == 1);
    }
    {
        auto ca = a;
        // fully allocated
        ca.deallocate(3);
        BOOST_TEST(ca.interval_count() == 1);
        // v.... no concat
        ca.deallocate(0);
        BOOST_TEST(ca.interval_count() == 2);
        // ....v concat left
        ca.deallocate(4);
        BOOST_TEST(ca.interval_count() == 2);
    }
    {
        auto ca = a;
        // fully allocated
        ca.deallocate(3);
        BOOST_TEST(ca.interval_count() == 1);
        // v.... concat right
        ca.deallocate(2);
        BOOST_TEST(ca.interval_count() == 1);
    }
    {
        auto ca = a;
        // fully allocated
        ca.deallocate(0);
        BOOST_TEST(ca.interval_count() == 1);
        // ....v no concat
        ca.deallocate(4);
        BOOST_TEST(ca.interval_count() == 2);
        // ..v.. concat right
        ca.deallocate(3);
        BOOST_TEST(ca.interval_count() == 2);
        // ..v.. concat left
        ca.deallocate(1);
        BOOST_TEST(ca.interval_count() == 2);
    }
    {
        auto ca = a;
        ca.deallocate(2);
        BOOST_TEST(ca.interval_count() == 1);
        // v.... concat left
        ca.deallocate(1);
        BOOST_TEST(ca.interval_count() == 1);
    }
}

BOOST_AUTO_TEST_CASE( signed_value ) {
    am::value_allocator<int> a{-2, 3};
    BOOST_TEST(a.interval_count() == 1);
    BOOST_TEST(a.use(2) == true);
    BOOST_TEST(a.interval_count() == 2);
    {
        auto value_opt = a.allocate();
        BOOST_CHECK(value_opt);
        BOOST_TEST(*value_opt == -2);
        BOOST_TEST(a.interval_count() == 2);
    }
    BOOST_TEST(a.use(0) == true);
    BOOST_TEST(a.interval_count() == 3);
}

BOOST_AUTO_TEST_SUITE_END()
