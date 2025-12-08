/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for ComponentTraits compile-time type analysis
*/

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>
#include "../../../lib/rtype_ecs/src/traits/ComponentTraits.hpp"

using namespace ECS;

// ============================================================================
// TEST COMPONENT TYPES
// ============================================================================

// Empty tag component
struct EmptyTag {};

// Another empty tag
struct MarkerComponent {};

// Trivially copyable component
struct TrivialComponent {
    int x;
    float y;
    double z;
};

// Trivially copyable POD struct
struct PODComponent {
    int data[10];
    float value;
};

// Non-trivially copyable (has std::string)
struct NonTrivialComponent {
    std::string name;
    int value;
};

// Non-trivially destructible (has std::vector)
struct VectorComponent {
    std::vector<int> data;
};

// Component with unique_ptr (non-copyable)
struct ResourceComponent {
    std::unique_ptr<int> resource;

    ResourceComponent() = default;
    ResourceComponent(ResourceComponent&&) = default;
    ResourceComponent& operator=(ResourceComponent&&) = default;
};

// Simple move-only component
struct MoveOnlyComponent {
    int value;

    MoveOnlyComponent() = default;
    MoveOnlyComponent(int v) : value(v) {}
    MoveOnlyComponent(const MoveOnlyComponent&) = delete;
    MoveOnlyComponent& operator=(const MoveOnlyComponent&) = delete;
    MoveOnlyComponent(MoveOnlyComponent&&) = default;
    MoveOnlyComponent& operator=(MoveOnlyComponent&&) = default;
};

// Non-movable component (invalid for ECS)
struct NonMovableComponent {
    int value;

    NonMovableComponent() = default;
    NonMovableComponent(const NonMovableComponent&) = delete;
    NonMovableComponent& operator=(const NonMovableComponent&) = delete;
    NonMovableComponent(NonMovableComponent&&) = delete;
    NonMovableComponent& operator=(NonMovableComponent&&) = delete;
};

// ============================================================================
// COMPONENT TRAITS TESTS - isEmpty
// ============================================================================

class ComponentTraitsTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ComponentTraitsTest, IsEmpty_TrueForEmptyStruct) {
    EXPECT_TRUE(ComponentTraits<EmptyTag>::isEmpty);
    EXPECT_TRUE(ComponentTraits<MarkerComponent>::isEmpty);
}

TEST_F(ComponentTraitsTest, IsEmpty_FalseForDataComponents) {
    EXPECT_FALSE(ComponentTraits<TrivialComponent>::isEmpty);
    EXPECT_FALSE(ComponentTraits<PODComponent>::isEmpty);
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::isEmpty);
    EXPECT_FALSE(ComponentTraits<VectorComponent>::isEmpty);
}

TEST_F(ComponentTraitsTest, IsEmpty_FalseForMoveOnlyComponent) {
    EXPECT_FALSE(ComponentTraits<MoveOnlyComponent>::isEmpty);
}

// ============================================================================
// COMPONENT TRAITS TESTS - isTrivial (trivially copyable)
// ============================================================================

TEST_F(ComponentTraitsTest, IsTrivial_TrueForPODTypes) {
    EXPECT_TRUE(ComponentTraits<TrivialComponent>::isTrivial);
    EXPECT_TRUE(ComponentTraits<PODComponent>::isTrivial);
}

TEST_F(ComponentTraitsTest, IsTrivial_TrueForEmptyTypes) {
    EXPECT_TRUE(ComponentTraits<EmptyTag>::isTrivial);
    EXPECT_TRUE(ComponentTraits<MarkerComponent>::isTrivial);
}

TEST_F(ComponentTraitsTest, IsTrivial_FalseForNonTrivialTypes) {
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::isTrivial);
    EXPECT_FALSE(ComponentTraits<VectorComponent>::isTrivial);
    EXPECT_FALSE(ComponentTraits<ResourceComponent>::isTrivial);
}

TEST_F(ComponentTraitsTest, IsTrivial_TrueForPrimitives) {
    EXPECT_TRUE(ComponentTraits<int>::isTrivial);
    EXPECT_TRUE(ComponentTraits<float>::isTrivial);
    EXPECT_TRUE(ComponentTraits<double>::isTrivial);
    EXPECT_TRUE(ComponentTraits<char>::isTrivial);
}

// ============================================================================
// COMPONENT TRAITS TESTS - isTrivialDestructible
// ============================================================================

TEST_F(ComponentTraitsTest, IsTrivialDestructible_TrueForPODTypes) {
    EXPECT_TRUE(ComponentTraits<TrivialComponent>::isTrivialDestructible);
    EXPECT_TRUE(ComponentTraits<PODComponent>::isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, IsTrivialDestructible_TrueForEmptyTypes) {
    EXPECT_TRUE(ComponentTraits<EmptyTag>::isTrivialDestructible);
    EXPECT_TRUE(ComponentTraits<MarkerComponent>::isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, IsTrivialDestructible_FalseForNonTrivialTypes) {
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::isTrivialDestructible);
    EXPECT_FALSE(ComponentTraits<VectorComponent>::isTrivialDestructible);
    EXPECT_FALSE(ComponentTraits<ResourceComponent>::isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, IsTrivialDestructible_TrueForPrimitives) {
    EXPECT_TRUE(ComponentTraits<int>::isTrivialDestructible);
    EXPECT_TRUE(ComponentTraits<float>::isTrivialDestructible);
    EXPECT_TRUE(ComponentTraits<double>::isTrivialDestructible);
}

// ============================================================================
// COMPONENT CONCEPT TESTS
// ============================================================================

TEST_F(ComponentTraitsTest, ComponentConcept_SatisfiedByMoveConstructibleTypes) {
    // These should satisfy the Component concept
    static_assert(Component<EmptyTag>, "EmptyTag should satisfy Component concept");
    static_assert(Component<TrivialComponent>, "TrivialComponent should satisfy Component concept");
    static_assert(Component<NonTrivialComponent>, "NonTrivialComponent should satisfy Component concept");
    static_assert(Component<MoveOnlyComponent>, "MoveOnlyComponent should satisfy Component concept");
    static_assert(Component<ResourceComponent>, "ResourceComponent should satisfy Component concept");

    SUCCEED();
}

TEST_F(ComponentTraitsTest, ComponentConcept_NotSatisfiedByNonMovableTypes) {
    // NonMovableComponent should NOT satisfy the Component concept
    static_assert(!Component<NonMovableComponent>, "NonMovableComponent should NOT satisfy Component concept");

    SUCCEED();
}

TEST_F(ComponentTraitsTest, ComponentConcept_SatisfiedByPrimitives) {
    static_assert(Component<int>, "int should satisfy Component concept");
    static_assert(Component<float>, "float should satisfy Component concept");
    static_assert(Component<double>, "double should satisfy Component concept");
    static_assert(Component<std::string>, "std::string should satisfy Component concept");

    SUCCEED();
}

// ============================================================================
// COMPILE-TIME VERIFICATION TESTS
// ============================================================================

TEST_F(ComponentTraitsTest, CompileTime_TraitsAreConstexpr) {
    // Verify traits can be used at compile time
    constexpr bool empty_is_empty = ComponentTraits<EmptyTag>::isEmpty;
    constexpr bool trivial_is_trivial = ComponentTraits<TrivialComponent>::isTrivial;
    constexpr bool pod_is_destructible = ComponentTraits<PODComponent>::isTrivialDestructible;

    static_assert(empty_is_empty, "EmptyTag should be empty at compile time");
    static_assert(trivial_is_trivial, "TrivialComponent should be trivial at compile time");
    static_assert(pod_is_destructible, "PODComponent should be trivially destructible at compile time");

    SUCCEED();
}

TEST_F(ComponentTraitsTest, CompileTime_ConditionalCompilation) {
    // Test that traits can be used for if constexpr
    auto test_func = []<typename T>() {
        if constexpr (ComponentTraits<T>::isEmpty) {
            return 1;
        } else if constexpr (ComponentTraits<T>::isTrivial) {
            return 2;
        } else {
            return 3;
        }
    };

    EXPECT_EQ(test_func.template operator()<EmptyTag>(), 1);
    EXPECT_EQ(test_func.template operator()<TrivialComponent>(), 2);
    EXPECT_EQ(test_func.template operator()<NonTrivialComponent>(), 3);
}

// ============================================================================
// STANDARD LIBRARY TYPE TESTS
// ============================================================================

TEST_F(ComponentTraitsTest, StdTypes_String) {
    EXPECT_FALSE(ComponentTraits<std::string>::isEmpty);
    EXPECT_FALSE(ComponentTraits<std::string>::isTrivial);
    EXPECT_FALSE(ComponentTraits<std::string>::isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, StdTypes_Vector) {
    EXPECT_FALSE(ComponentTraits<std::vector<int>>::isEmpty);
    EXPECT_FALSE(ComponentTraits<std::vector<int>>::isTrivial);
    EXPECT_FALSE(ComponentTraits<std::vector<int>>::isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, StdTypes_UniquePtr) {
    EXPECT_FALSE(ComponentTraits<std::unique_ptr<int>>::isEmpty);
    EXPECT_FALSE(ComponentTraits<std::unique_ptr<int>>::isTrivial);
    EXPECT_FALSE(ComponentTraits<std::unique_ptr<int>>::isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, StdTypes_SharedPtr) {
    EXPECT_FALSE(ComponentTraits<std::shared_ptr<int>>::isEmpty);
    EXPECT_FALSE(ComponentTraits<std::shared_ptr<int>>::isTrivial);
    EXPECT_FALSE(ComponentTraits<std::shared_ptr<int>>::isTrivialDestructible);
}

// ============================================================================
// COMBINED TRAITS TESTS
// ============================================================================

TEST_F(ComponentTraitsTest, CombinedTraits_EmptyAndTrivial) {
    // Empty types are always trivially copyable and destructible
    EXPECT_TRUE(ComponentTraits<EmptyTag>::isEmpty);
    EXPECT_TRUE(ComponentTraits<EmptyTag>::isTrivial);
    EXPECT_TRUE(ComponentTraits<EmptyTag>::isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, CombinedTraits_TrivialButNotEmpty) {
    EXPECT_FALSE(ComponentTraits<TrivialComponent>::isEmpty);
    EXPECT_TRUE(ComponentTraits<TrivialComponent>::isTrivial);
    EXPECT_TRUE(ComponentTraits<TrivialComponent>::isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, CombinedTraits_NonTrivialNonEmpty) {
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::isEmpty);
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::isTrivial);
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::isTrivialDestructible);
}

// ============================================================================
// EDGE CASES
// ============================================================================

// Component with padding
struct PaddedComponent {
    char a;
    // padding here
    int b;
    char c;
    // more padding
};

TEST_F(ComponentTraitsTest, EdgeCase_PaddedComponent) {
    EXPECT_FALSE(ComponentTraits<PaddedComponent>::isEmpty);
    EXPECT_TRUE(ComponentTraits<PaddedComponent>::isTrivial);
    EXPECT_TRUE(ComponentTraits<PaddedComponent>::isTrivialDestructible);
}

// Component with bit fields
struct BitFieldComponent {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int value : 30;
};

TEST_F(ComponentTraitsTest, EdgeCase_BitFieldComponent) {
    EXPECT_FALSE(ComponentTraits<BitFieldComponent>::isEmpty);
    EXPECT_TRUE(ComponentTraits<BitFieldComponent>::isTrivial);
    EXPECT_TRUE(ComponentTraits<BitFieldComponent>::isTrivialDestructible);
}

// Component with static members only (still empty in terms of instance size)
struct StaticOnlyComponent {
    static int counter;
    static void increment() { counter++; }
};
int StaticOnlyComponent::counter = 0;

TEST_F(ComponentTraitsTest, EdgeCase_StaticOnlyComponent) {
    // Static members don't contribute to instance size
    EXPECT_TRUE(ComponentTraits<StaticOnlyComponent>::isEmpty);
    EXPECT_TRUE(ComponentTraits<StaticOnlyComponent>::isTrivial);
}

// Nested empty struct
struct OuterEmpty {
    struct InnerEmpty {};
    InnerEmpty inner;  // Empty member - but EBO only applies to base classes, not members
};

// Empty base class for EBO test
struct EmptyBase {};
struct DerivedFromEmpty : EmptyBase {
    // EBO applies here - derived class can be empty if it has no members
};

TEST_F(ComponentTraitsTest, EdgeCase_NestedEmpty) {
    // A struct containing an empty member is NOT empty (EBO doesn't apply to members)
    // Each object must have a unique address, so sizeof(OuterEmpty) >= 1
    EXPECT_FALSE(ComponentTraits<OuterEmpty>::isEmpty);
    EXPECT_TRUE(ComponentTraits<OuterEmpty>::isTrivial);
}

TEST_F(ComponentTraitsTest, EdgeCase_EmptyBaseOptimization) {
    // EBO applies to inheritance - a class derived from empty base with no members is empty
    EXPECT_TRUE(ComponentTraits<DerivedFromEmpty>::isEmpty);
    EXPECT_TRUE(ComponentTraits<DerivedFromEmpty>::isTrivial);
}

// ============================================================================
// ADDITIONAL COVERAGE TESTS
// ============================================================================

// Component with const member
struct ConstMemberComponent {
    const int value = 42;
};

TEST_F(ComponentTraitsTest, ConstMemberComponent) {
    EXPECT_FALSE(ComponentTraits<ConstMemberComponent>::isEmpty);
    // Const members make the type non-copyable by default assignment
    EXPECT_TRUE(ComponentTraits<ConstMemberComponent>::isTrivialDestructible);
}

// Component with reference member (makes it non-copyable)
struct RefMemberComponent {
    int& ref;
    explicit RefMemberComponent(int& r) : ref(r) {}
};

TEST_F(ComponentTraitsTest, RefMemberComponent) {
    EXPECT_FALSE(ComponentTraits<RefMemberComponent>::isEmpty);
    // References don't make it non-trivially copyable, but the type is trivially copyable
    EXPECT_TRUE(ComponentTraits<RefMemberComponent>::isTrivial);
}

// Component with virtual method
struct VirtualComponent {
    virtual ~VirtualComponent() = default;
    virtual void update() {}
    int value = 0;
};

TEST_F(ComponentTraitsTest, VirtualComponent) {
    EXPECT_FALSE(ComponentTraits<VirtualComponent>::isEmpty);  // Has vtable pointer
    EXPECT_FALSE(ComponentTraits<VirtualComponent>::isTrivial);
    EXPECT_FALSE(ComponentTraits<VirtualComponent>::isTrivialDestructible);
}

// Component with deleted copy constructor
struct NoCopyComponent {
    int value = 0;
    NoCopyComponent() = default;
    NoCopyComponent(const NoCopyComponent&) = delete;
    NoCopyComponent& operator=(const NoCopyComponent&) = delete;
    NoCopyComponent(NoCopyComponent&&) = default;
    NoCopyComponent& operator=(NoCopyComponent&&) = default;
};

TEST_F(ComponentTraitsTest, NoCopyComponent) {
    EXPECT_FALSE(ComponentTraits<NoCopyComponent>::isEmpty);
    // In C++, std::is_trivially_copyable can be true even if copy is deleted
    // as long as the type has trivial move operations and is trivially destructible
    EXPECT_TRUE(ComponentTraits<NoCopyComponent>::isTrivial);
    EXPECT_TRUE(ComponentTraits<NoCopyComponent>::isTrivialDestructible);
}

// Array component
struct ArrayComponent {
    int data[10];
};

TEST_F(ComponentTraitsTest, ArrayComponent) {
    EXPECT_FALSE(ComponentTraits<ArrayComponent>::isEmpty);
    EXPECT_TRUE(ComponentTraits<ArrayComponent>::isTrivial);
    EXPECT_TRUE(ComponentTraits<ArrayComponent>::isTrivialDestructible);
}

// Union component
union UnionComponent {
    int i;
    float f;
    char c;
};

TEST_F(ComponentTraitsTest, UnionComponent) {
    EXPECT_FALSE(ComponentTraits<UnionComponent>::isEmpty);
    EXPECT_TRUE(ComponentTraits<UnionComponent>::isTrivial);
    EXPECT_TRUE(ComponentTraits<UnionComponent>::isTrivialDestructible);
}

// Enum class (scoped enum)
enum class ComponentState : uint8_t {
    Active,
    Inactive,
    Destroyed
};

TEST_F(ComponentTraitsTest, EnumComponent) {
    EXPECT_FALSE(ComponentTraits<ComponentState>::isEmpty);
    EXPECT_TRUE(ComponentTraits<ComponentState>::isTrivial);
    EXPECT_TRUE(ComponentTraits<ComponentState>::isTrivialDestructible);
}

// Component with alignas
struct alignas(64) AlignedComponent {
    float x, y, z, w;
};

TEST_F(ComponentTraitsTest, AlignedComponent) {
    EXPECT_FALSE(ComponentTraits<AlignedComponent>::isEmpty);
    EXPECT_TRUE(ComponentTraits<AlignedComponent>::isTrivial);
    EXPECT_TRUE(ComponentTraits<AlignedComponent>::isTrivialDestructible);
}

// Component with optional
struct OptionalComponent {
    std::optional<int> value;
};

TEST_F(ComponentTraitsTest, OptionalComponent) {
    EXPECT_FALSE(ComponentTraits<OptionalComponent>::isEmpty);
    // std::optional<trivial> is trivially copyable if T is trivial
    EXPECT_TRUE(ComponentTraits<OptionalComponent>::isTrivialDestructible);
}

// Component with variant
struct VariantComponent {
    std::variant<int, float, std::string> data;
};

TEST_F(ComponentTraitsTest, VariantComponent) {
    EXPECT_FALSE(ComponentTraits<VariantComponent>::isEmpty);
    EXPECT_FALSE(ComponentTraits<VariantComponent>::isTrivial);  // Contains non-trivial string
    EXPECT_FALSE(ComponentTraits<VariantComponent>::isTrivialDestructible);
}

// Empty final class
struct FinalEmptyComponent final {};

TEST_F(ComponentTraitsTest, FinalEmptyComponent) {
    EXPECT_TRUE(ComponentTraits<FinalEmptyComponent>::isEmpty);
    EXPECT_TRUE(ComponentTraits<FinalEmptyComponent>::isTrivial);
}

// Component with atomic
struct AtomicComponent {
    std::atomic<int> counter{0};
};

TEST_F(ComponentTraitsTest, AtomicComponent) {
    EXPECT_FALSE(ComponentTraits<AtomicComponent>::isEmpty);
    // std::atomic<int> is_trivially_copyable can be true in some implementations
    // The actual value depends on the platform/compiler
    // What we care about is that the trait exists and gives a valid result
    [[maybe_unused]] bool isTrivial = ComponentTraits<AtomicComponent>::isTrivial;
    // Atomics have trivial destructors
    EXPECT_TRUE(ComponentTraits<AtomicComponent>::isTrivialDestructible);
}
