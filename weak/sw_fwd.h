#pragma once

#include <exception>
#include <cstddef>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class ControlBlockBase {
public:
    virtual ~ControlBlockBase() = default;
    virtual void OnZeroStrong() = 0;
    virtual void OnZeroWeak() = 0;

    void IncrStg() {
        ++stg_counter_;
    }

    void DecrStr() {
        --stg_counter_;
        if (stg_counter_ == 0 && weak_counter_ == 0) {
            OnZeroStrong();
            OnZeroWeak();
        } else if (stg_counter_ == 0) {
            OnZeroStrong();
        }
    }

    void IncWeak() {
        ++weak_counter_;
    }

    void DecrWeak() {
        --weak_counter_;
        if (stg_counter_ + weak_counter_ == 0) {
            OnZeroWeak();
        }
    }

    int stg_counter_ = 1;
    int weak_counter_ = 0;
};

template <typename T>
class PointingControlBlock : public ControlBlockBase {
public:
    ~PointingControlBlock() override = default;

    explicit PointingControlBlock(T* ptr) : ptr_(ptr) {
    }

    void OnZeroStrong() override {
        T* temp = ptr_;
        ptr_ = nullptr;
        delete temp;
    }

    void OnZeroWeak() override {
        delete this;
    }

private:
    T* ptr_ = nullptr;
};

template <typename T>
class EmplacingControlBlock : public ControlBlockBase {
public:
    template <typename... Args>
    EmplacingControlBlock(Args&&... args) {
        new (&buff_) T(std::forward<Args>(args)...);
    }

    void OnZeroStrong() override {
        Get()->~T();
    }

    void OnZeroWeak() override {
        delete this;
    }

    T* Get() {
        return reinterpret_cast<T*>(&buff_);
    }

private:
    alignas(T) std::byte buff_[sizeof(T)];
};