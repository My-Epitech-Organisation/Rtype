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
#include "../../../src/engine/ecs/traits/ComponentTraits.hpp"

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
    EXPECT_TRUE(ComponentTraits<EmptyTag>::_isEmpty);
    EXPECT_TRUE(ComponentTraits<MarkerComponent>::_isEmpty);
}

TEST_F(ComponentTraitsTest, IsEmpty_FalseForDataComponents) {
    EXPECT_FALSE(ComponentTraits<TrivialComponent>::_isEmpty);
    EXPECT_FALSE(ComponentTraits<PODComponent>::_isEmpty);
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::_isEmpty);
    EXPECT_FALSE(ComponentTraits<VectorComponent>::_isEmpty);
}

TEST_F(ComponentTraitsTest, IsEmpty_FalseForMoveOnlyComponent) {
    EXPECT_FALSE(ComponentTraits<MoveOnlyComponent>::_isEmpty);
}

// ============================================================================
// COMPONENT TRAITS TESTS - isTrivial (trivially copyable)
// ============================================================================

TEST_F(ComponentTraitsTest, IsTrivial_TrueForPODTypes) {
    EXPECT_TRUE(ComponentTraits<TrivialComponent>::_isTrivial);
    EXPECT_TRUE(ComponentTraits<PODComponent>::_isTrivial);
}

TEST_F(ComponentTraitsTest, IsTrivial_TrueForEmptyTypes) {
    EXPECT_TRUE(ComponentTraits<EmptyTag>::_isTrivial);
    EXPECT_TRUE(ComponentTraits<MarkerComponent>::_isTrivial);
}

TEST_F(ComponentTraitsTest, IsTrivial_FalseForNonTrivialTypes) {
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::_isTrivial);
    EXPECT_FALSE(ComponentTraits<VectorComponent>::_isTrivial);
    EXPECT_FALSE(ComponentTraits<ResourceComponent>::_isTrivial);
}

TEST_F(ComponentTraitsTest, IsTrivial_TrueForPrimitives) {
    EXPECT_TRUE(ComponentTraits<int>::_isTrivial);
    EXPECT_TRUE(ComponentTraits<float>::_isTrivial);
    EXPECT_TRUE(ComponentTraits<double>::_isTrivial);
    EXPECT_TRUE(ComponentTraits<char>::_isTrivial);
}

// ============================================================================
// COMPONENT TRAITS TESTS - isTrivialDestructible
// ============================================================================

TEST_F(ComponentTraitsTest, IsTrivialDestructible_TrueForPODTypes) {
    EXPECT_TRUE(ComponentTraits<TrivialComponent>::_isTrivialDestructible);
    EXPECT_TRUE(ComponentTraits<PODComponent>::_isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, IsTrivialDestructible_TrueForEmptyTypes) {
    EXPECT_TRUE(ComponentTraits<EmptyTag>::_isTrivialDestructible);
    EXPECT_TRUE(ComponentTraits<MarkerComponent>::_isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, IsTrivialDestructible_FalseForNonTrivialTypes) {
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::_isTrivialDestructible);
    EXPECT_FALSE(ComponentTraits<VectorComponent>::_isTrivialDestructible);
    EXPECT_FALSE(ComponentTraits<ResourceComponent>::_isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, IsTrivialDestructible_TrueForPrimitives) {
    EXPECT_TRUE(ComponentTraits<int>::_isTrivialDestructible);
    EXPECT_TRUE(ComponentTraits<float>::_isTrivialDestructible);
    EXPECT_TRUE(ComponentTraits<double>::_isTrivialDestructible);
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
    constexpr bool empty_is_empty = ComponentTraits<EmptyTag>::_isEmpty;
    constexpr bool trivial_is_trivial = ComponentTraits<TrivialComponent>::_isTrivial;
    constexpr bool pod_is_destructible = ComponentTraits<PODComponent>::_isTrivialDestructible;

    static_assert(empty_is_empty, "EmptyTag should be empty at compile time");
    static_assert(trivial_is_trivial, "TrivialComponent should be trivial at compile time");
    static_assert(pod_is_destructible, "PODComponent should be trivially destructible at compile time");

    SUCCEED();
}

TEST_F(ComponentTraitsTest, CompileTime_ConditionalCompilation) {
    // Test that traits can be used for if constexpr
    auto test_func = []<typename T>() {
        if constexpr (ComponentTraits<T>::_isEmpty) {
            return 1;
        } else if constexpr (ComponentTraits<T>::_isTrivial) {
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
    EXPECT_FALSE(ComponentTraits<std::string>::_isEmpty);
    EXPECT_FALSE(ComponentTraits<std::string>::_isTrivial);
    EXPECT_FALSE(ComponentTraits<std::string>::_isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, StdTypes_Vector) {
    EXPECT_FALSE(ComponentTraits<std::vector<int>>::_isEmpty);
    EXPECT_FALSE(ComponentTraits<std::vector<int>>::_isTrivial);
    EXPECT_FALSE(ComponentTraits<std::vector<int>>::_isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, StdTypes_UniquePtr) {
    EXPECT_FALSE(ComponentTraits<std::unique_ptr<int>>::_isEmpty);
    EXPECT_FALSE(ComponentTraits<std::unique_ptr<int>>::_isTrivial);
    EXPECT_FALSE(ComponentTraits<std::unique_ptr<int>>::_isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, StdTypes_SharedPtr) {
    EXPECT_FALSE(ComponentTraits<std::shared_ptr<int>>::_isEmpty);
    EXPECT_FALSE(ComponentTraits<std::shared_ptr<int>>::_isTrivial);
    EXPECT_FALSE(ComponentTraits<std::shared_ptr<int>>::_isTrivialDestructible);
}

// ============================================================================
// COMBINED TRAITS TESTS
// ============================================================================

TEST_F(ComponentTraitsTest, CombinedTraits_EmptyAndTrivial) {
    // Empty types are always trivially copyable and destructible
    EXPECT_TRUE(ComponentTraits<EmptyTag>::_isEmpty);
    EXPECT_TRUE(ComponentTraits<EmptyTag>::_isTrivial);
    EXPECT_TRUE(ComponentTraits<EmptyTag>::_isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, CombinedTraits_TrivialButNotEmpty) {
    EXPECT_FALSE(ComponentTraits<TrivialComponent>::_isEmpty);
    EXPECT_TRUE(ComponentTraits<TrivialComponent>::_isTrivial);
    EXPECT_TRUE(ComponentTraits<TrivialComponent>::_isTrivialDestructible);
}

TEST_F(ComponentTraitsTest, CombinedTraits_NonTrivialNonEmpty) {
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::_isEmpty);
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::_isTrivial);
    EXPECT_FALSE(ComponentTraits<NonTrivialComponent>::_isTrivialDestructible);
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
    EXPECT_FALSE(ComponentTraits<PaddedComponent>::_isEmpty);
    EXPECT_TRUE(ComponentTraits<PaddedComponent>::_isTrivial);
    EXPECT_TRUE(ComponentTraits<PaddedComponent>::_isTrivialDestructible);
}

// Component with bit fields
struct BitFieldComponent {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int value : 30;
};

TEST_F(ComponentTraitsTest, EdgeCase_BitFieldComponent) {
    EXPECT_FALSE(ComponentTraits<BitFieldComponent>::_isEmpty);
    EXPECT_TRUE(ComponentTraits<BitFieldComponent>::_isTrivial);
    EXPECT_TRUE(ComponentTraits<BitFieldComponent>::_isTrivialDestructible);
}

// Component with static members only (still empty in terms of instance size)
struct StaticOnlyComponent {
    static int counter;
    static void increment() { counter++; }
};
int StaticOnlyComponent::counter = 0;

TEST_F(ComponentTraitsTest, EdgeCase_StaticOnlyComponent) {
    // Static members don't contribute to instance size
    EXPECT_TRUE(ComponentTraits<StaticOnlyComponent>::_isEmpty);
    EXPECT_TRUE(ComponentTraits<StaticOnlyComponent>::_isTrivial);
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
    EXPECT_FALSE(ComponentTraits<OuterEmpty>::_isEmpty);
    EXPECT_TRUE(ComponentTraits<OuterEmpty>::_isTrivial);
}

TEST_F(ComponentTraitsTest, EdgeCase_EmptyBaseOptimization) {
    // EBO applies to inheritance - a class derived from empty base with no members is empty
    EXPECT_TRUE(ComponentTraits<DerivedFromEmpty>::_isEmpty);
    EXPECT_TRUE(ComponentTraits<DerivedFromEmpty>::_isTrivial);
}
