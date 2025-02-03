#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

template <typename T>
struct Slug {
    Slug() = default;

    void operator()(T* ptr) {
        static_assert(sizeof(T) > 0);
        if constexpr (std::is_same_v<T, void>) {
            free(ptr);
        } else {
            delete ptr;
        }
    }

    template <typename U>  // DOUBLE CHECK IF DOESN'T WORK LATER
    Slug(const Slug<U>&) {
    }

    template <typename U>
    Slug(Slug<U>&&) noexcept {
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : pair_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, Deleter deleter) noexcept : pair_(ptr, std::move(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept : pair_(other.Release(), std::move(other.GetDeleter())) {
    }

    template <typename F, typename S>
    UniquePtr(UniquePtr<F, S>&& other) noexcept
        : pair_(other.Release(), std::move(other.GetDeleter())) {
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            Reset();
            pair_.GetFirst() = other.Release();
            pair_.GetSecond() = std::move(other.GetDeleter());
        }
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        return std::exchange(pair_.GetFirst(), nullptr);
    }

    void Reset(T* ptr = nullptr) {
        T* old = std::exchange(pair_.GetFirst(), ptr);
        if (old) {
            GetDeleter()(old);
        }
    }

    void Swap(UniquePtr& other) {
        if (pair_.GetFirst() && other.pair_.GetFirst()) {
            std::swap(pair_.GetFirst(), other.pair_.GetFirst());
            std::swap(pair_.GetSecond(), other.pair_.GetSecond());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }

    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }

    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T operator*() const {
        return *Get();
    }

    T* operator->() const {
        if constexpr (!std::is_same_v<T, void>) {
            return Get();
        }
    }

private:
    CompressedPair<T*, Deleter> pair_;
};

template <typename T>
struct Slug<T[]> {
    Slug() = default;

    void operator()(T* ptr) {
        if (ptr) {
            delete[] ptr;
        }
    }

    template <typename U>  // DOUBLE CHECK LATER IF DOESN'T WORK
    Slug(const Slug<U>&) {
    }

    template <typename U>
    Slug(Slug<U>&&) noexcept {
    }
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : pair_(ptr, Deleter()) {
    }

    UniquePtr(T* ptr, Deleter deleter) noexcept : pair_(ptr, std::move(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept : pair_(other.Release(), std::move(other.GetDeleter())) {
    }

    template <typename F, typename S>
    UniquePtr(UniquePtr<F, S>&& other) noexcept
        : pair_(other.Release(), std::move(other.GetDeleter())) {
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            Reset();
            pair_.GetFirst() = other.Release();
            pair_.GetSecond() = std::move(other.GetDeleter());
        }
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        return std::exchange(pair_.GetFirst(), nullptr);
    }

    void Reset(T* ptr = nullptr) {
        T* old = std::exchange(pair_.GetFirst(), ptr);
        if (old) {
            GetDeleter()(old);
        }
    }

    void Swap(UniquePtr& other) {
        if (pair_.GetFirst() && other.pair_.GetFirst()) {
            std::swap(pair_.GetFirst(), other.pair_.GetFirst());
            std::swap(pair_.GetSecond(), other.pair_.GetSecond());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }

    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }

    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T operator*() const {
        return *Get();
    }

    T* operator->() const {
        if constexpr (!std::is_same_v<T, void>) {
            return Get();
        }
    }

    T& operator[](std::size_t i) {
        return Get()[i];
    }

private:
    CompressedPair<T*, Deleter> pair_;
};