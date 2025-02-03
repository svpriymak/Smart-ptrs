#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() noexcept = default;

    WeakPtr(const WeakPtr& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->IncWeak();
        }
    }
    WeakPtr(WeakPtr&& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) noexcept : ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->IncWeak();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) noexcept {
        WeakPtr(other).Swap(*this);
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) noexcept {
        WeakPtr(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (cb_) {
            cb_->DecrWeak();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        WeakPtr().Swap(*this);
    }
    void Swap(WeakPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(cb_, other.cb_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (cb_) {
            return cb_->stg_counter_;
        }
        return 0;
    }
    bool Expired() const {
        return !cb_ || cb_->stg_counter_ == 0;
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }

private:
    T* ptr_ = nullptr;
    ControlBlockBase* cb_ = nullptr;

    friend class SharedPtr<T>;

    template <typename Y>
    friend class WeakPtr;
};
