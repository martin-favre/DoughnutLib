
#include <gtest/gtest.h>
#include "GraphicsManager.h"

TEST(GraphicsManagerTest, TestInitializeBasicGraphics) {
  // will assert internally on failure.
  GraphicsManager::initialize();
  GraphicsManager::teardown();
}