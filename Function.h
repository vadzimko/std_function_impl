#ifndef FUNC_FUNCTION_H
#define FUNC_FUNCTION_H

#include <cstddef>
#include <iostream>
#include <array>
#include <cassert>
#include <cstring>

template <class R, class ...Args>
struct callable_base {
    virtual R operator()(Args... args)  = 0;
    virtual ~callable_base() = default;
    virtual callable_base *clone() const = 0;
};

template <class F, class R, class ...Args>
struct callable : callable_base<R, Args...> {
    explicit callable(F const& functor) : functor(functor) {}
    R operator()(Args... args)  { return functor(args...); }
    callable *clone() const { return new callable(functor); }

private:
    F functor;
};

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
    typedef R result_type;

    Function() noexcept;
    explicit Function(std::nullptr_t) noexcept;
    Function(Function const& rhs);
    Function(Function&& other) noexcept;

    template<typename F>
    Function(F func);

    ~Function();

    Function& operator=(const Function& other);
    Function& operator=(Function&& other) noexcept;

    void swap(Function& other) noexcept;

    explicit operator bool() const noexcept;
    R operator()(Args...) const;

private:
    callable_base<R,Args...> *f = nullptr;
    static unsigned const BUF_MAX_SIZE = 1024;
    char buffer[BUF_MAX_SIZE] = {};
    bool is_small = false;
};

template <class R, class ...Args>
Function<R(Args...)>::Function() noexcept = default;

template <class R, class ...Args>
Function <R(Args...)>::Function(std::nullptr_t) noexcept {}

template <class R, class ...Args>
R Function<R(Args...)>::operator()(Args... args) const {
    assert(*this);

    if (is_small) {
        return (*(callable_base<R, Args...> *)(buffer))(args ...);
    } else {
        return (*f)(args...);
    }
}

template <class R, class ...Args>
template <class F>
Function<R(Args...)>::Function(F func) {
    if (sizeof(func) < BUF_MAX_SIZE) {
        new (buffer) callable<typename func_type_getter<F>::type, R, Args...>(func);
        is_small = true;
    } else {
        f = new callable<typename func_type_getter<F>::type, R, Args...>(func);
        is_small = false;
    }
}

template <class R, class ...Args>
Function <R(Args...)>::Function(Function const& rhs) {
    if (rhs) {
        is_small = rhs.is_small;
        if (rhs.is_small) {
            memcpy(buffer, rhs.buffer, BUF_MAX_SIZE);
        } else {
            f = rhs.f->clone();
        }
    }
}

template <class R, class ...Args>
Function<R(Args...)>::operator bool() const noexcept {
    return (is_small || f != nullptr);
}

template <class R, class ...Args>
Function<R(Args...)>::~Function() {
    if (!is_small) {
        delete f;
    }
}

template<class R, class... Args>
Function<R(Args...)>::Function(Function &&other) noexcept : Function() {
    swap(other);
}

template<class R, class... Args>
void Function<R(Args...)>::swap(Function &other) noexcept {
    std::swap(is_small, other.is_small);
    std::swap(f, other.f);
    std::swap_ranges(buffer, buffer + BUF_MAX_SIZE, other.buffer);
}

template<class R, class... Args>
Function<R(Args...)>& Function<R(Args...)>::operator=(const Function &other) {
    if (this != &other) {
        Function<R(Args...)> temp(other);
        swap(temp);
    }
    return *this;
}

template<class R, class... Args>
Function<R(Args...)>& Function<R(Args...)>::operator=(Function &&other) noexcept {
    swap(other);
    return *this;
}
#endif //FUNC_FUNCTION_H
