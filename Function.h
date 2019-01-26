#ifndef FUNC_FUNCTION_H
#define FUNC_FUNCTION_H

#include <cstddef>
#include <iostream>
#include <array>
#include <cassert>
#include <cstring>
#include <memory>

template <class Func>
struct func_type_getter {
    typedef Func type;
};

template <class Result,class ...Args>
struct func_type_getter<Result(Args...)>
{
    typedef Result (*type)(Args...);
};

template <class> class Function;

template <class R, class ...Args>
struct Function<R(Args...)> {

    struct callable_base {
        virtual R operator()(Args... args)  = 0;
        virtual ~callable_base() = default;
        virtual callable_base *clone()  = 0;
        virtual void placement_clone(void* buff) = 0;
    };

    template <typename F>
    struct callable : callable_base {
        explicit callable(F& functor) : functor(std::move(functor)) {}
        R operator()(Args... args)  { return functor(std::forward<Args>(args)...); }
        callable *clone() { return new callable(functor); }
        void placement_clone(void* buff) { new (buff) callable<F>(functor); }

    private:
        F functor;
    };

    Function() noexcept : f(nullptr) {}
    explicit Function(std::nullptr_t) noexcept {}
    Function(Function const& rhs) {
        *this = rhs;
    }

    Function(Function&& other) noexcept {
        swap(other);
    }

    template<typename F>
    Function(F func) {
        if (sizeof(func) < BUF_MAX_SIZE) {
            new (buffer) callable<typename func_type_getter<F>::type>(func);
            is_small = true;
        } else {
            f = new callable<typename func_type_getter<F>::type>(func);
            is_small = false;
        }
        assigned = true;
    }

    ~Function() {
        if (!assigned) {
            return;
        }

        if (!is_small) {
            delete f;
        } else {
            ((callable_base*)buffer)->~callable_base();
        }
    }

    Function& operator=(const Function& other) {
        is_small = other.is_small;
        assigned = other.assigned;
        if (assigned) {
            if (other.is_small) {
                auto func = (callable_base*)(other.buffer);
                func->placement_clone(buffer);
            } else {
                f = other.f->clone();
            }
        }
        return *this;
    }

    Function& operator=(Function&& other) noexcept {
        swap(other);
        return *this;
    }

    void swap(Function& other) noexcept {
        std::swap(is_small, other.is_small);
        std::swap(assigned, other.assigned);
        std::swap(buffer, other.buffer);
    }

    explicit operator bool() const noexcept {
        return assigned;
    }
    R operator()(Args... args) const {
        assert(*this);

        if (is_small) {
            return (*(callable_base*)(buffer))(std::forward<Args>(args)...);
        } else {
            return (*f)(args...);
        }
    }

private:
    static unsigned const BUF_MAX_SIZE = 1024;
    bool is_small = false;
    bool assigned = false;

    union {
        callable_base *f = nullptr;
        char buffer[BUF_MAX_SIZE];
    };
};
#endif //FUNC_FUNCTION_H
