#pragma once

#include <type_traits>
#include <utility>

template <typename T, int i, bool empty = std::is_empty_v<T> && !std::is_final_v<T>>
class Based {
private:
    T type_;

public:
    Based() : type_() {
    }

    Based(const T& ctype) : type_(ctype) {
    }

    Based(T&& ctype) : type_(std::move(ctype)) {
    }

    T& Get() {
        return type_;
    }
    const T& Get() const {
        return type_;
    }
};

template <typename T, int i>
class Based<T, i, true> : private T {
public:
    Based() : T() {
    }

    Based(const T& ctype) : T(ctype) {
    }

    Based(T&& ctype) : T(std::move(ctype)) {
    }

    T& Get() {
        return *this;
    }
    const T& Get() const {
        return *this;
    }
};

// Me think, why waste time write lot code, when few code do trick.
template <typename F, typename S>
class CompressedPair : private Based<F, 0>, private Based<S, 1> {
public:
    CompressedPair() = default;

    CompressedPair(const F& first, const S& second) : Based<F, 0>(first), Based<S, 1>(second) {
    }

    CompressedPair(F&& first, S&& second)
        : Based<F, 0>(std::move(first)), Based<S, 1>(std::move(second)) {
    }

    CompressedPair(const F& first, S&& second)
        : Based<F, 0>(first), Based<S, 1>(std::move(second)) {
    }

    CompressedPair(F&& first, const S& second)
        : Based<F, 0>(std::move(first)), Based<S, 1>(second) {
    }

    F& GetFirst() {
        return Based<F, 0>::Get();
    }
    const F& GetFirst() const {
        return Based<F, 0>::Get();
    }

    S& GetSecond() {
        return Based<S, 1>::Get();
    }
    const S& GetSecond() const {
        return Based<S, 1>::Get();
    }
};
