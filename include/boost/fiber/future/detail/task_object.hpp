
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DETAIL_TASK_OBJECT_H
#define BOOST_FIBERS_DETAIL_TASK_OBJECT_H

#include <exception>
#include <memory>
#include <utility>

#include <boost/config.hpp>
#include <boost/context/detail/apply.hpp>

#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/future/detail/task_base.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace detail {

template< typename Fn, typename Allocator, typename R, typename ... Args >
class task_object : public task_base< R, Args ... > {
public:
    typedef typename std::allocator_traits< Allocator >::template rebind_alloc< 
        task_object
    >                                           allocator_t;

    task_object( allocator_t const& alloc, Fn && fn) :
        task_base< R, Args ... >(),
        fn_( std::move( fn) ),
        alloc_( alloc) {
    }

    void run( Args && ... args) override final {
        try {
            this->set_value(
                    boost::context::detail::apply(
                        fn_, std::make_tuple( std::forward< Args >( args) ... ) ) );
        } catch (...) {
            this->set_exception( std::current_exception() );
        }
    }

protected:
    void deallocate_future() noexcept override final {
        destroy_( alloc_, this);
    }

private:
    Fn                  fn_;
    allocator_t         alloc_;

    static void destroy_( allocator_t & alloc, task_object * p) noexcept {
        alloc.destroy( p);
        alloc.deallocate( p, 1);
    }
};

template< typename Fn, typename Allocator, typename ... Args >
class task_object< Fn, Allocator, void, Args ... > : public task_base< void, Args ... > {
public:
    typedef typename Allocator::template rebind<
        task_object< Fn, Allocator, void, Args ... >
    >::other                                      allocator_t;

    task_object( allocator_t const& alloc, Fn && fn) :
        task_base< void, Args ... >(),
        fn_( std::move( fn) ),
        alloc_( alloc) {
    }

    void run( Args && ... args) override final {
        try {
            boost::context::detail::apply(
                    fn_, std::make_tuple( std::forward< Args >( args) ... ) );
            this->set_value();
        } catch (...) {
            this->set_exception( std::current_exception() );
        }
    }

protected:
    void deallocate_future() noexcept override final {
        destroy_( alloc_, this);
    }

private:
    Fn                  fn_;
    allocator_t         alloc_;

    static void destroy_( allocator_t & alloc, task_object * p) noexcept {
        alloc.destroy( p);
        alloc.deallocate( p, 1);
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_DETAIL_TASK_OBJECT_H
