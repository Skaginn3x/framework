#pragma once
// clang-format off
//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/asio_experiments
//

#ifndef BOOST_SAM_DETAIL_IMPL_PREDICATE_OP_MODEL_HPP
#define BOOST_SAM_DETAIL_IMPL_PREDICATE_OP_MODEL_HPP

#include <boost/sam/detail/predicate_op_model.hpp>
#include <exception>

#if defined(BOOST_SAM_STANDALONE)
#include <asio/experimental/append.hpp>
#include <asio/post.hpp>
#else
#include <boost/asio/experimental/append.hpp>
#include <boost/asio/post.hpp>
#endif

BOOST_SAM_BEGIN_NAMESPACE

namespace detail
{
template <class Executor, class Handler, class Predicate, class... Ts>
auto predicate_op_model<Executor, Handler, Predicate, void(error_code ec, Ts...)>::construct(Executor  e,
                                                                                             Handler   handler,
                                                                                             Predicate predicate)
    -> predicate_op_model *
{
  auto halloc  = net::get_associated_allocator(handler);
  auto alloc   = typename std::allocator_traits<decltype(halloc)>::template rebind_alloc<predicate_op_model>(halloc);
  using traits = std::allocator_traits<decltype(alloc)>;
  auto pmem    = traits::allocate(alloc, 1);

  try
  {
    return new (pmem) predicate_op_model(std::move(e), std::move(handler), std::move(predicate));
  }
  catch (...)
  {
    traits::deallocate(alloc, pmem, 1);
    throw;
  }
}

template <class Executor, class Handler, class Predicate, class... Ts>
auto predicate_op_model<Executor, Handler, Predicate, void(error_code ec, Ts...)>::destroy(
    predicate_op_model *self, net::associated_allocator_t<Handler> halloc) -> void
{
  auto alloc = typename std::allocator_traits<decltype(halloc)>::template rebind_alloc<predicate_op_model>(halloc);
  self->~predicate_op_model();
  auto traits = std::allocator_traits<decltype(alloc)>();
  traits.deallocate(alloc, self, 1);
}

template <class Executor, class Handler, class Predicate, class... Ts>
predicate_op_model<Executor, Handler, Predicate, void(error_code ec, Ts...)>::predicate_op_model(Executor  e,
                                                                                                 Handler   handler,
                                                                                                 Predicate predicate)
    : work_guard_(std::move(e)), handler_(std::move(handler)), predicate_(std::move(predicate))
{
}

template <class Executor, class Handler, class Predicate, class... Ts>
void predicate_op_model<Executor, Handler, Predicate, void(error_code ec, Ts...)>::complete(error_code ec, Ts... args)
{
  get_cancellation_slot().clear();
  auto g = std::move(work_guard_);
  auto h = std::move(handler_);
  this->unlink();
  destroy(this, net::get_associated_allocator(h));
  net::post(g.get_executor(), net::append(std::move(h), ec, std::move(args)...));
}

template <class Executor, class Handler, class Predicate, class... Ts>
void predicate_op_model<Executor, Handler, Predicate, void(error_code ec, Ts...)>::shutdown()
{
  get_cancellation_slot().clear();
  this->unlink();
  destroy(this, net::get_associated_allocator(this->handler_));
}

} // namespace detail

BOOST_SAM_END_NAMESPACE

#endif /// BOOST_SAM_DETAIL_IMPL_PREDICATE_OP_MODEL_HPP
