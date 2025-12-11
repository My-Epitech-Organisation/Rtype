/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_aabb - Unit tests for AABB collision detection
*/

#include <gtest/gtest.h>

#include "../../../src/games/rtype/shared/Systems/Collision/AABB.hpp"

using namespace rtype::games::rtype::shared;
using namespace rtype::games::rtype::shared::collision;

class AABBTest : public ::testing::Test {
   protected:
    TransformComponent transformA;
    TransformComponent transformB;
    BoundingBoxComponent boxA{10.0F, 10.0F};
    BoundingBoxComponent boxB{10.0F, 10.0F};
};

// =============================================================================
// Basic Overlap Tests
// =============================================================================

TEST_F(AABBTest, OverlappingBoxesReturnTrue) {
    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 5.0F;
    transformB.y = 5.0F;

    EXPECT_TRUE(overlaps(transformA, boxA, transformB, boxB));
}

TEST_F(AABBTest, IdenticalPositionsOverlap) {
    transformA.x = 100.0F;
    transformA.y = 100.0F;
    transformB.x = 100.0F;
    transformB.y = 100.0F;

    EXPECT_TRUE(overlaps(transformA, boxA, transformB, boxB));
}

// =============================================================================
// Separation Tests - All 4 conditions of the separation check
// =============================================================================

TEST_F(AABBTest, SeparatedLeftNoOverlap) {
    // A is to the left of B (aRight < bLeft)
    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 20.0F;  // Far to the right
    transformB.y = 0.0F;

    EXPECT_FALSE(overlaps(transformA, boxA, transformB, boxB));
}

TEST_F(AABBTest, SeparatedRightNoOverlap) {
    // B is to the left of A (bRight < aLeft)
    transformA.x = 20.0F;  // Far to the right
    transformA.y = 0.0F;
    transformB.x = 0.0F;
    transformB.y = 0.0F;

    EXPECT_FALSE(overlaps(transformA, boxA, transformB, boxB));
}

TEST_F(AABBTest, SeparatedAboveNoOverlap) {
    // A is above B (aBottom < bTop)
    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 0.0F;
    transformB.y = 20.0F;  // Far below

    EXPECT_FALSE(overlaps(transformA, boxA, transformB, boxB));
}

TEST_F(AABBTest, SeparatedBelowNoOverlap) {
    // B is above A (bBottom < aTop)
    transformA.x = 0.0F;
    transformA.y = 20.0F;  // Far below
    transformB.x = 0.0F;
    transformB.y = 0.0F;

    EXPECT_FALSE(overlaps(transformA, boxA, transformB, boxB));
}

// =============================================================================
// Edge Cases - Touching but not overlapping
// =============================================================================

TEST_F(AABBTest, TouchingEdgesHorizontallyNoOverlap) {
    // Boxes touch exactly at the edge (aRight == bLeft)
    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 10.0F;  // Exactly touching
    transformB.y = 0.0F;

    // aRight = 5, bLeft = 5 -> aRight < bLeft is false, so they touch
    EXPECT_TRUE(overlaps(transformA, boxA, transformB, boxB));
}

TEST_F(AABBTest, TouchingEdgesVerticallyNoOverlap) {
    // Boxes touch exactly at the edge (aBottom == bTop)
    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 0.0F;
    transformB.y = 10.0F;  // Exactly touching

    EXPECT_TRUE(overlaps(transformA, boxA, transformB, boxB));
}

TEST_F(AABBTest, JustSeparatedHorizontally) {
    // Boxes are just barely separated horizontally
    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 10.1F;  // Slightly separated
    transformB.y = 0.0F;

    EXPECT_FALSE(overlaps(transformA, boxA, transformB, boxB));
}

TEST_F(AABBTest, JustSeparatedVertically) {
    // Boxes are just barely separated vertically
    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 0.0F;
    transformB.y = 10.1F;  // Slightly separated

    EXPECT_FALSE(overlaps(transformA, boxA, transformB, boxB));
}

// =============================================================================
// Corner Cases
// =============================================================================

TEST_F(AABBTest, OverlapAtCorner) {
    // Boxes overlap at a corner
    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 9.0F;
    transformB.y = 9.0F;

    EXPECT_TRUE(overlaps(transformA, boxA, transformB, boxB));
}

TEST_F(AABBTest, DiagonalSeparation) {
    // Boxes are diagonally separated
    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 20.0F;
    transformB.y = 20.0F;

    EXPECT_FALSE(overlaps(transformA, boxA, transformB, boxB));
}

// =============================================================================
// Different Box Sizes
// =============================================================================

TEST_F(AABBTest, DifferentSizedBoxesOverlap) {
    BoundingBoxComponent largeBox{20.0F, 20.0F};
    BoundingBoxComponent smallBox{5.0F, 5.0F};

    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 8.0F;
    transformB.y = 8.0F;

    EXPECT_TRUE(overlaps(transformA, largeBox, transformB, smallBox));
}

TEST_F(AABBTest, SmallBoxInsideLargeBox) {
    BoundingBoxComponent largeBox{100.0F, 100.0F};
    BoundingBoxComponent smallBox{10.0F, 10.0F};

    transformA.x = 50.0F;
    transformA.y = 50.0F;
    transformB.x = 50.0F;
    transformB.y = 50.0F;

    EXPECT_TRUE(overlaps(transformA, largeBox, transformB, smallBox));
}

TEST_F(AABBTest, TallAndWideBoxesOverlap) {
    BoundingBoxComponent tallBox{5.0F, 50.0F};
    BoundingBoxComponent wideBox{50.0F, 5.0F};

    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 0.0F;
    transformB.y = 0.0F;

    EXPECT_TRUE(overlaps(transformA, tallBox, transformB, wideBox));
}

TEST_F(AABBTest, TallAndWideBoxesSeparated) {
    BoundingBoxComponent tallBox{5.0F, 50.0F};
    BoundingBoxComponent wideBox{50.0F, 5.0F};

    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 30.0F;  // Moved far right
    transformB.y = 30.0F;  // Moved far down

    EXPECT_FALSE(overlaps(transformA, tallBox, transformB, wideBox));
}

// =============================================================================
// Negative Coordinates
// =============================================================================

TEST_F(AABBTest, NegativeCoordinatesOverlap) {
    transformA.x = -5.0F;
    transformA.y = -5.0F;
    transformB.x = 0.0F;
    transformB.y = 0.0F;

    EXPECT_TRUE(overlaps(transformA, boxA, transformB, boxB));
}

TEST_F(AABBTest, NegativeCoordinatesSeparated) {
    transformA.x = -20.0F;
    transformA.y = -20.0F;
    transformB.x = 0.0F;
    transformB.y = 0.0F;

    EXPECT_FALSE(overlaps(transformA, boxA, transformB, boxB));
}

TEST_F(AABBTest, MixedPositiveNegativeOverlap) {
    transformA.x = -3.0F;
    transformA.y = 3.0F;
    transformB.x = 3.0F;
    transformB.y = -3.0F;

    EXPECT_TRUE(overlaps(transformA, boxA, transformB, boxB));
}

// =============================================================================
// Symmetry Tests
// =============================================================================

TEST_F(AABBTest, OverlapIsSymmetric) {
    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 5.0F;
    transformB.y = 5.0F;

    bool resultAB = overlaps(transformA, boxA, transformB, boxB);
    bool resultBA = overlaps(transformB, boxB, transformA, boxA);

    EXPECT_EQ(resultAB, resultBA);
    EXPECT_TRUE(resultAB);
}

TEST_F(AABBTest, SeparationIsSymmetric) {
    transformA.x = 0.0F;
    transformA.y = 0.0F;
    transformB.x = 50.0F;
    transformB.y = 50.0F;

    bool resultAB = overlaps(transformA, boxA, transformB, boxB);
    bool resultBA = overlaps(transformB, boxB, transformA, boxA);

    EXPECT_EQ(resultAB, resultBA);
    EXPECT_FALSE(resultAB);
}
