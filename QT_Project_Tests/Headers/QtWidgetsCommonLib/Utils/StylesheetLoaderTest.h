#pragma once

#include <gtest/gtest.h>

#include <QString>
#include <QVector>
#include <QtGlobal>

#include "QtWidgetsCommonLib/Utils/StylesheetLoader.h"

/**
 * @file StylesheetLoaderTest.h
 * @brief Test fixture for StylesheetLoader.
 */
class StylesheetLoaderTest: public ::testing::Test
{
    protected:
        StylesheetLoaderTest() = default;
        ~StylesheetLoaderTest() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to create a temporary QSS file with given content.
         * @param content The QSS content to write.
         * @return The file path of the created temporary file.
         */
        [[nodiscard]] auto create_temp_qss(const QString& content) -> QString;

        /**
         * @brief Installs a temporary message handler to capture warnings/debug logs.
         * @return A pair of (captured_messages, previous_handler) to inspect and restore later.
         */
        [[nodiscard]] static auto install_message_capture()
            -> std::pair<QVector<QString>*, QtMessageHandler>;

        /**
         * @brief Restores previous message handler and frees capture buffer.
         * @param prev Previous handler returned by install_message_capture.
         * @param buf Pointer returned by install_message_capture.
         */
        static auto restore_message_capture(QtMessageHandler prev, QVector<QString>* buf) -> void;

        QtWidgetsCommonLib::StylesheetLoader* m_loader = nullptr;
};
