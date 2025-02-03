#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

class EnableBase {};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() = default;
    SharedPtr(std::nullptr_t) : ptr_(nullptr), cb_(nullptr) {
    }
    template <typename F>
    explicit SharedPtr(F* ptr) : ptr_(ptr), cb_(new PointingControlBlock<F>(ptr)) {
        if constexpr (std::is_base_of_v<EnableBase, F>) {
            SharedPtr<F> sp(*this, ptr);
            ptr->weak_ = WeakPtr<F>(sp);
        }
    }

    SharedPtr(const SharedPtr& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->IncrStg();
        }
    }
    SharedPtr(SharedPtr&& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->IncrStg();
        }
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    template <typename F>
    SharedPtr(F* ptr, ControlBlockBase* cb) : ptr_(ptr), cb_(cb) {
        if constexpr (std::is_base_of_v<EnableBase, F>) {
            SharedPtr<F> sp(*this, ptr);
            ptr->weak_ = WeakPtr<F>(sp);
        }
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(SharedPtr<Y>& other, T* ptr) : ptr_(ptr), cb_(other.cb_) {
        if (cb_) {
            cb_->IncrStg();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        ptr_ = other.ptr_;
        cb_ = other.cb_;
        if (cb_) {
            cb_->IncrStg();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) noexcept {
        SharedPtr(other).Swap(*this);
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        SharedPtr(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (cb_) {
            cb_->DecrStr();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        SharedPtr().Swap(*this);
    }

    template <typename V>
    void Reset(V* ptr) {
        SharedPtr(ptr).Swap(*this);
    }
    void Swap(SharedPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(cb_, other.cb_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (cb_) {
            return cb_->stg_counter_;
        }
        return 0;
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    T* ptr_ = nullptr;
    ControlBlockBase* cb_ = nullptr;
    template <typename Y>
    friend class SharedPtr;
    template <typename Y>
    friend class WeakPtr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto new_block = new EmplacingControlBlock<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(new_block->Get(), new_block);
}

//// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis : public EnableBase {
public:
    SharedPtr<T> SharedFromThis() {
        return weak_.Lock();
    }

    SharedPtr<const T> SharedFromThis() const {
        return weak_.Lock();
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return WeakPtr<T>(weak_);
    }

    WeakPtr<const T> WeakFromThis() const noexcept {
        return WeakPtr<const T>(weak_);
    }

private:
    mutable WeakPtr<T> weak_;

    template <typename Y>
    friend class SharedPtr;
};
