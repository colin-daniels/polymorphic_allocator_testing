#include "../src/library.h"

#include <vector>

TEST(PMRVectorTest, AllocatorPropagation) { // NOLINT(cert-err58-cpp)
    WithDefaultMockMemoryResource([](auto &default_resource) {
        auto resource = std::pmr::new_delete_resource();
        auto alloc = std::pmr::polymorphic_allocator<double>(resource);

        // the following constructors should set the allocator properly + not allocate from the default memory resource
        EXPECT_CALL(default_resource, do_allocate).Times(0);
        EXPECT_CALL(default_resource, do_deallocate).Times(0);

        EXPECT_EQ(std::pmr::vector<double>(32, alloc).get_allocator(), alloc);
        EXPECT_EQ(std::pmr::vector<double>(32, 1.0, alloc).get_allocator(), alloc);
        EXPECT_EQ(std::pmr::vector<double>({1.0, 2.0, 3.0}, alloc).get_allocator(), alloc);
        double arr[10] = {1.0, 2.0, 3.0};
        EXPECT_EQ(std::pmr::vector<double>(std::begin(arr), std::end(arr), alloc).get_allocator(), alloc);
        // copy/move constructors
        auto vec = std::pmr::vector<double>({1.0, 2.0, 3.0}, alloc);
        EXPECT_EQ(std::pmr::vector<double>(vec, alloc).get_allocator(), alloc);
        EXPECT_EQ(std::pmr::vector<double>(std::move(vec), alloc).get_allocator(), alloc);
    });

    EXPECT_EQ(1 + 1, 2);
}

TEST(PMRVectorTest, NestedVectorsPropagateAllocators) { // NOLINT(cert-err58-cpp)
    // check that std::pmr::vector propagates its allocator to inner containers automatically
    WithDefaultMockMemoryResource([](auto &default_resource) {
        EXPECT_CALL(default_resource, do_allocate).Times(0);
        EXPECT_CALL(default_resource, do_deallocate).Times(0);

        auto resource = std::pmr::new_delete_resource();
        auto alloc = std::pmr::polymorphic_allocator<int>(resource);

        std::pmr::vector<std::pmr::vector<int>> vector_of_vectors(alloc);

        // non-default allocation
        vector_of_vectors.reserve(16);

        // emplace back should work
        vector_of_vectors.emplace_back();
        vector_of_vectors.front().reserve(512);
        vector_of_vectors.emplace_back(std::initializer_list<int>{1, 2, 3, 4, 5});
        // re-allocate
        vector_of_vectors.resize(1024);

        // copy assignment should work + not default allocate
        std::pmr::vector<std::pmr::vector<int>> more_vectors(alloc);
        more_vectors = vector_of_vectors;
        // move assignment should also work
        std::pmr::vector<std::pmr::vector<int>> even_more_vectors = std::move(more_vectors);
        even_more_vectors.resize(2048);
    });

    EXPECT_EQ(1 + 1, 2);
}
