#pragma once

#include <gtest/gtest.h>

#include "QtWidgetsCommonLib/Widgets/WindowTitleBar.h"

/**
 * @file WindowTitleBarTest.h
 * @brief Test fixture for WindowTitleBar.
 */
class WindowTitleBarTest: public ::testing::Test
{
    protected:
        WindowTitleBarTest() = default;
        ~WindowTitleBarTest() override = default;

        void SetUp() override;
        void TearDown() override;

        QtWidgetsCommonLib::WindowTitleBar* m_title_bar = nullptr;
};
