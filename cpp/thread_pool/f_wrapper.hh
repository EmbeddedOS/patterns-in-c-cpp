#pragma once

#include <future>
#include <memory>

namespace larva {

    class f_wrapper {
        struct impl_base {
            virtual void call() = 0;
            virtual ~impl_base() {}
        };

        template <typename F>
        struct impl: impl_base {
            F _f;
            impl(F&& f): _f {std::move(f)} {}
            void call() { this->_f(); }
        };

        std::unique_ptr<impl_base> _impl {nullptr};

    public:
        template <typename F>
        f_wrapper(F&& f): _impl {std::make_unique<impl<F>>(std::move(f))} {}
        f_wrapper(f_wrapper&& other): _impl {std::move(other._impl)} {}
        f_wrapper() = default;

        void operator() ()
        {
            this->_impl->call();
        }

        f_wrapper& operator=(f_wrapper&& other)
        {
            this->_impl = std::move(other._impl);
        }

        f_wrapper(const f_wrapper&) = delete;
        f_wrapper(f_wrapper&) = delete;
        f_wrapper& operator=(const f_wrapper&) = delete;
    };

}