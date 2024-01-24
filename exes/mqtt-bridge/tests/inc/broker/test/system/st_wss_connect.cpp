// Copyright Takatoshi Kondo 2023
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "../common/test_main.hpp"
#include "../common/global_fixture.hpp"
#include "broker_runner.hpp"
#include "coro_base.hpp"

#include <async_mqtt/all.hpp>
#include <boost/asio/yield.hpp>

BOOST_AUTO_TEST_SUITE(st_connect)

namespace am = async_mqtt;
namespace as = boost::asio;

BOOST_AUTO_TEST_CASE(cb) {
    broker_runner br;
    as::io_context ioc;
    as::ip::address address = boost::asio::ip::make_address("127.0.0.1");
    as::ip::tcp::endpoint endpoint{address, 10443};

    am::tls::context ctx{am::tls::context::tlsv12};
    ctx.set_verify_mode(am::tls::verify_peer);
    ctx.load_verify_file("cacert.pem");

    using ep_t = am::endpoint<am::role::client, am::protocol::wss>;
    auto amep = ep_t::create(
        am::protocol_version::v3_1_1,
        ioc.get_executor(),
        ctx
    );

    amep->lowest_layer().async_connect(
        endpoint,
        [&](am::error_code const& ec) {
            BOOST_TEST(ec == am::error_code{});
            amep->next_layer().next_layer().async_handshake(
                am::tls::stream_base::client,
                [&](am::error_code const& ec) {
                    BOOST_TEST(ec == am::error_code{});
                    amep->next_layer().async_handshake(
                        "127.0.0.1",
                        "/",
                        [&](am::error_code const& ec) {
                            BOOST_TEST(ec == am::error_code{});
                            amep->send(
                                am::v3_1_1::connect_packet{
                                    true,   // clean_session
                                    0x1234, // keep_alive
                                    am::allocate_buffer("cid1"),
                                    am::nullopt, // will
                                    am::allocate_buffer("u1"),
                                    am::allocate_buffer("passforu1")
                                },
                                [&](am::system_error const& se) {
                                    BOOST_TEST(!se);
                                    amep->recv(
                                        [&](am::packet_variant pv) {
                                            pv.visit(
                                                am::overload {
                                                    [&](am::v3_1_1::connack_packet const& p) {
                                                        BOOST_TEST(!p.session_present());
                                                    },
                                                    [](auto const&) {
                                                        BOOST_TEST(false);
                                                    }
                                                }
                                            );
                                            amep->close([]{});
                                        }
                                    );
                                }
                            );
                        }
                    );
                }
            );
        }
    );

    ioc.run();
}

BOOST_AUTO_TEST_CASE(fut) {
    broker_runner br;
    as::io_context ioc;
    as::ip::address address = boost::asio::ip::make_address("127.0.0.1");
    as::ip::tcp::endpoint endpoint{address, 10443};

    am::tls::context ctx{am::tls::context::tlsv12};
    ctx.set_verify_mode(am::tls::verify_peer);
    ctx.load_verify_file("cacert.pem");

    using ep_t = am::endpoint<am::role::client, am::protocol::wss>;
    auto amep = ep_t::create(
        am::protocol_version::v3_1_1,
        ioc.get_executor(),
        ctx
    );

    auto guard = as::make_work_guard(ioc.get_executor());
    std::thread th {
        [&] {
            ioc.run();
        }
    };
    auto on_finish = am::unique_scope_guard(
        [&] {
            guard.reset();
            th.join();
        }
    );

    {
        auto fut = amep->lowest_layer().async_connect(
            endpoint,
            as::use_future
        );
        try {
            fut.get();
        }
        catch (am::error_code const&) {
            BOOST_TEST(false);
        }
    }
    {
        auto fut = amep->next_layer().next_layer().async_handshake(
            am::tls::stream_base::client,
            as::use_future
        );
        try {
            fut.get();
        }
        catch (am::error_code const&) {
            BOOST_TEST(false);
        }
    }
    {
        auto fut = amep->next_layer().async_handshake(
            "127.0.0.1",
            "/",
            as::use_future
        );
        try {
            fut.get();
        }
        catch (am::error_code const&) {
            BOOST_TEST(false);
        }
    }
    {
        auto fut =
            amep->send(
                am::v3_1_1::connect_packet{
                    true,   // clean_session
                    0x1234, // keep_alive
                    am::allocate_buffer("cid1"),
                    am::nullopt, // will
                    am::allocate_buffer("u1"),
                    am::allocate_buffer("passforu1")
                },
                as::use_future
            );
        am::system_error se = fut.get();
        BOOST_TEST(!se);
    }
    {
        auto fut =
            amep->recv(as::use_future);
        auto pv = fut.get();
        pv.visit(
            am::overload {
                [&](am::v3_1_1::connack_packet const& p) {
                    BOOST_TEST(!p.session_present());
                },
                [](auto const&) {
                    BOOST_TEST(false);
                }
            }
        );
    }
    {
        auto fut = amep->close(as::use_future);
        fut.get();
    }
}

BOOST_AUTO_TEST_CASE(coro) {
    broker_runner br;
    as::io_context ioc;
    am::tls::context ctx{am::tls::context::tlsv12};
    ctx.set_verify_mode(am::tls::verify_peer);
    ctx.load_verify_file("cacert.pem");

    using ep_t = am::endpoint<am::role::client, am::protocol::wss>;
    auto amep = ep_t::create(
        am::protocol_version::v3_1_1,
        ioc.get_executor(),
        ctx
    );

    struct tc : coro_base<ep_t> {
        using coro_base<ep_t>::coro_base;
    private:
        void proc(
            am::optional<am::error_code> ec,
            am::optional<am::system_error> se,
            am::optional<am::packet_variant> pv,
            am::optional<packet_id_t> /*pid*/
        ) override {
            reenter(this) {
                yield ep().lowest_layer().async_connect(
                    dest(),
                    *this
                );
                BOOST_TEST(*ec == am::error_code{});
                yield ep().next_layer().next_layer().async_handshake(
                    am::tls::stream_base::client,
                    *this
                );
                BOOST_TEST(*ec == am::error_code{});
                yield ep().next_layer().async_handshake(
                    "127.0.0.1",
                    "/",
                    *this
                );
                BOOST_TEST(*ec == am::error_code{});
                yield ep().send(
                    am::v3_1_1::connect_packet{
                        true,   // clean_session
                        0x1234, // keep_alive
                        am::allocate_buffer("cid1"),
                        am::nullopt, // will
                        am::allocate_buffer("u1"),
                        am::allocate_buffer("passforu1")
                    },
                    *this
                );
                BOOST_TEST(!*se);
                yield ep().recv(*this);
                pv->visit(
                    am::overload {
                        [&](am::v3_1_1::connack_packet const& p) {
                            BOOST_TEST(!p.session_present());
                        },
                        [](auto const&) {
                            BOOST_TEST(false);
                        }
                   }
                );
                yield ep().close(*this);
                yield set_finish();
            }
        }
    };

    tc t{*amep, "127.0.0.1", 10443};
    t();
    ioc.run();
    BOOST_TEST(t.finish());
}

BOOST_AUTO_TEST_SUITE_END()

#include <boost/asio/unyield.hpp>
