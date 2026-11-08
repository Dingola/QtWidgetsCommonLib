/**
 * @file main.cpp
 * @brief Demonstrates the installed recent-files menu facade without application-specific code.
 */

#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStringList>
#include <utility>

#include "QtWidgetsCommonLib/RecentItems/RecentFilesMenuController.h"

/**
 * @brief Runs the standalone recent-files menu example.
 * @param argc Number of command-line arguments.
 * @param argv Command-line argument values.
 * @return Application exit code.
 */
auto main(int argc, char* argv[]) -> int
{
    QApplication application(argc, argv);
    QMainWindow window;
    QMenu* recent_files_menu = window.menuBar()->addMenu(QStringLiteral("Recent Files"));
    QtWidgetsCommonLib::RecentFilesMenuController controller(recent_files_menu, &window);

    QtWidgetsCommonLib::RecentFilesMenuTexts texts;
    texts.empty_text = QStringLiteral("No recent files");
    texts.clear_text = QStringLiteral("Clear recent files");
    texts.remove_unavailable_text = QStringLiteral("Remove unavailable files");
    texts.disabled_text_format = QStringLiteral("%1 — %2");
    texts.available_status_text = QStringLiteral("Available");
    texts.missing_status_text = QStringLiteral("File not found");
    texts.not_readable_status_text = QStringLiteral("File is not readable");
    texts.not_a_file_status_text = QStringLiteral("Path is not a file");
    texts.available_tooltip_format = QStringLiteral("Available: %1");
    texts.missing_tooltip_format = QStringLiteral("File not found: %1");
    texts.not_readable_tooltip_format = QStringLiteral("File is not readable: %1");
    texts.not_a_file_tooltip_format = QStringLiteral("Path is not a file: %1");
    controller.set_texts(std::move(texts));
    controller.set_files({QStringLiteral("first.log"), QStringLiteral("second.log")});

    QObject::connect(&controller, &QtWidgetsCommonLib::RecentFilesMenuController::file_activated,
                     [](const QString& file_path) { qInfo() << "Open requested:" << file_path; });
    QObject::connect(&controller, &QtWidgetsCommonLib::RecentFilesMenuController::clear_requested,
                     [] { qInfo() << "Clear requested"; });
    QObject::connect(recent_files_menu, &QMenu::aboutToShow, &controller,
                     &QtWidgetsCommonLib::RecentFilesMenuController::refresh_file_status);

    window.resize(480, 240);
    window.show();
    const int result = application.exec();
    return result;
}
