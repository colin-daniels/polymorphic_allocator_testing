#ifndef POLYMORPHIC_ALLOC_TESTING_LIBRARY_H
#define POLYMORPHIC_ALLOC_TESTING_LIBRARY_H

#include <memory_resource>
#include "gmock/gmock.h"

class mock_memory_resource : public std::pmr::memory_resource {
public:
    MOCK_METHOD(void*, do_allocate, (std::size_t bytes, std::size_t alignment), (override));
    MOCK_METHOD(void, do_deallocate, (void* p, std::size_t bytes, std::size_t alignment), (override));
    MOCK_METHOD(bool, do_is_equal, (const std::pmr::memory_resource& other), (const, noexcept, override));

    // Delegates the default actions of the methods to std::pmr::new_delete_resource.
    void delegate_to_new_delete() {
        // forward to new_delete_resource
        ON_CALL(*this, do_allocate).WillByDefault(
                [this](std::size_t bytes, std::size_t alignment) {
                    return std::pmr::new_delete_resource()->allocate(bytes, alignment);
                });

        // forward to new_delete_resource
        ON_CALL(*this, do_deallocate).WillByDefault(
                [this](void* p, std::size_t bytes, std::size_t alignment) {
                    return std::pmr::new_delete_resource()->deallocate(p, bytes, alignment);
                });

        // equal if the other one is a mock memory resource
        ON_CALL(*this, do_is_equal).WillByDefault(
                [this](const std::pmr::memory_resource& other) {
                    return dynamic_cast<const mock_memory_resource*>(&other) != nullptr;
                });
    }
};

/// @brief Run the given closure, passing it a newly-constructed mock_memory_resource.
template<class F>
void WithMockMemoryResource(F&& run) {
    static_assert(std::is_invocable_v<F, mock_memory_resource &>,
                  "The `run` argument must be able to accept mock_memory_resource& when invoked.");

    mock_memory_resource mock_resource;
    mock_resource.delegate_to_new_delete();

    // run the code/function with the resource
    std::invoke(std::forward<F>(run), mock_resource);
}

/// @brief Run the given closure after constructing a mock_memory_resource and
///     setting it as the default memory resource. Resets the default_memory resource back to its original
///     value after the closure is run (or if any exceptions are thrown).
template<class F>
void WithDefaultMockMemoryResource(F&& run) {
    static_assert(std::is_invocable_v<F, mock_memory_resource &>,
                  "The `run` argument must be able to accept mock_memory_resource& when invoked.");

    WithMockMemoryResource([&](auto &mock_resource) {
        // set the default resource to the mock one
        auto default_resource = std::pmr::get_default_resource();
        std::pmr::set_default_resource(&mock_resource);

        try {
            // run the code/function
            std::invoke(std::forward<F>(run), mock_resource);

            // reset to the default resource
            std::pmr::set_default_resource(default_resource);
        } catch (...) {
            // also reset in case of exception
            std::pmr::set_default_resource(default_resource);
            throw;
        }
    });
}

#endif //POLYMORPHIC_ALLOC_TESTING_LIBRARY_H
