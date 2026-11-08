/**
 * @file RecentFilesMenuControllerTest.cpp
 * @brief Tests the standard recent-files menu integration path.
 */

#include "QtWidgetsCommonLib/RecentItems/RecentFilesMenuControllerTest.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QFile>
#include <QMenu>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "QtWidgetsCommonLib/RecentItems/RecentFilesMenuController.h"

namespace
{
/**
 * @brief Creates the host-owned text configuration used by facade tests.
 * @return Complete deterministic text configuration.
 */
[[nodiscard]] auto make_texts() -> QtWidgetsCommonLib::RecentFilesMenuTexts
{
    QtWidgetsCommonLib::RecentFilesMenuTexts result;
    result.empty_text = QStringLiteral("No recent files");
    result.clear_text = QStringLiteral("Clear list");
    result.remove_unavailable_text = QStringLiteral("Remove unavailable");
    result.disabled_text_format = QStringLiteral("%1 — %2");
    result.available_status_text = QStringLiteral("Available");
    result.missing_status_text = QStringLiteral("Missing");
    result.not_readable_status_text = QStringLiteral("Not readable");
    result.not_a_file_status_text = QStringLiteral("Not a file");
    result.available_tooltip_format = QStringLiteral("Available: %1");
    result.missing_tooltip_format = QStringLiteral("Missing: %1");
    result.not_readable_tooltip_format = QStringLiteral("Not readable: %1");
    result.not_a_file_tooltip_format = QStringLiteral("Not a file: %1");
    return result;
}

/**
 * @brief Finds a generated menu action by exact text.
 * @param menu Menu containing generated actions.
 * @param text Exact action text to find.
 * @return Matching action or null when absent.
 */
[[nodiscard]] auto find_action(const QMenu& menu, const QString& text) -> QAction*
{
    QAction* result = nullptr;
    const QList<QAction*> actions = menu.actions();
    for (int index = 0; index < actions.size() && result == nullptr; ++index)
    {
        if (actions.at(index)->text() == text)
        {
            result = actions.at(index);
        }
    }
    return result;
}
}  // namespace

/**
 * @brief Verifies the facade maps paths, disables missing files, and emits host intent.
 */
TEST_F(RecentFilesMenuControllerTest, ProvidesStandardRecentFilesMenuIntegration)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString available_path = directory.filePath(QStringLiteral("available.log"));
    QFile available_file(available_path);
    ASSERT_TRUE(available_file.open(QIODevice::WriteOnly));
    available_file.close();
    const QString missing_path = directory.filePath(QStringLiteral("missing.log"));
    QMenu menu;
    QtWidgetsCommonLib::RecentFilesMenuController controller(&menu);
    controller.set_texts(make_texts());
    QSignalSpy activated_spy(&controller,
                             &QtWidgetsCommonLib::RecentFilesMenuController::file_activated);
    QSignalSpy clear_spy(&controller,
                         &QtWidgetsCommonLib::RecentFilesMenuController::clear_requested);
    QSignalSpy remove_unavailable_spy(
        &controller, &QtWidgetsCommonLib::RecentFilesMenuController::remove_unavailable_requested);

    controller.set_files({available_path, missing_path});

    QAction* available_action = find_action(menu, QStringLiteral("available.log"));
    QAction* missing_action = find_action(menu, QStringLiteral("missing.log — Missing"));
    QAction* clear_action = find_action(menu, QStringLiteral("Clear list"));
    QAction* remove_unavailable_action = find_action(menu, QStringLiteral("Remove unavailable"));
    ASSERT_NE(available_action, nullptr);
    ASSERT_NE(missing_action, nullptr);
    ASSERT_NE(clear_action, nullptr);
    ASSERT_NE(remove_unavailable_action, nullptr);
    EXPECT_TRUE(available_action->isEnabled());
    EXPECT_FALSE(missing_action->isEnabled());
    EXPECT_TRUE(remove_unavailable_action->isEnabled());
    EXPECT_EQ(controller.model()->rowCount(), 2);

    available_action->trigger();
    clear_action->trigger();
    remove_unavailable_action->trigger();

    ASSERT_EQ(activated_spy.count(), 1);
    EXPECT_EQ(activated_spy.front().front().toString(), available_path);
    EXPECT_EQ(clear_spy.count(), 1);
    EXPECT_EQ(remove_unavailable_spy.count(), 1);
}

/**
 * @brief Verifies explicit refresh reflects filesystem changes without replacing the path list.
 */
TEST_F(RecentFilesMenuControllerTest, RefreshesTransientFileAvailability)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString file_path = directory.filePath(QStringLiteral("changing.log"));
    QFile file(file_path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.close();
    QMenu menu;
    QtWidgetsCommonLib::RecentFilesMenuController controller(&menu);
    controller.set_texts(make_texts());
    controller.set_files({file_path});
    ASSERT_NE(find_action(menu, QStringLiteral("changing.log")), nullptr);

    ASSERT_TRUE(QFile::remove(file_path));
    controller.refresh_file_status();

    QAction* missing_action = find_action(menu, QStringLiteral("changing.log — Missing"));
    ASSERT_NE(missing_action, nullptr);
    EXPECT_FALSE(missing_action->isEnabled());
    EXPECT_EQ(controller.model()->rowCount(), 1);
}

/**
 * @brief Verifies empty text, limits, and clear behavior are configurable without host glue.
 */
TEST_F(RecentFilesMenuControllerTest, AppliesEmptyAndLimitConfiguration)
{
    QMenu menu;
    QtWidgetsCommonLib::RecentFilesMenuController controller(&menu);
    controller.set_texts(make_texts());
    controller.set_maximum_visible_items(1);

    controller.set_files({QStringLiteral("first.log"), QStringLiteral("second.log")});

    EXPECT_EQ(controller.model()->rowCount(), 2);
    EXPECT_EQ(menu.actions().size(), 4);

    controller.set_files({});

    QAction* empty_action = find_action(menu, QStringLiteral("No recent files"));
    QAction* clear_action = find_action(menu, QStringLiteral("Clear list"));
    ASSERT_NE(empty_action, nullptr);
    ASSERT_NE(clear_action, nullptr);
    EXPECT_FALSE(empty_action->isEnabled());
    EXPECT_FALSE(clear_action->isEnabled());
}
