/**
 * @file RecentFilesMenuController.cpp
 * @brief Implements the high-level recent-files menu facade.
 */

#include "QtWidgetsCommonLib/RecentItems/RecentFilesMenuController.h"

#include <QAction>
#include <QFileInfo>
#include <QMenu>
#include <QVector>
#include <utility>

#include "QtCommonLib/FileSystem/LocalFileStatus.h"
#include "QtWidgetsCommonLib/RecentItems/RecentItemPresentation.h"
#include "QtWidgetsCommonLib/RecentItems/RecentItemsMenuBinder.h"
#include "QtWidgetsCommonLib/RecentItems/RecentItemsModel.h"
#include "QtWidgetsCommonLib/RecentItems/RecentRoles.h"

namespace
{
/**
 * @brief Applies an optional one-placeholder tooltip format to a normalized path.
 * @param format Host-provided tooltip format.
 * @param normalized_path Current normalized file path.
 * @return Formatted tooltip or the normalized path when no format was supplied.
 */
[[nodiscard]] auto format_tooltip(const QString& format, const QString& normalized_path) -> QString
{
    QString result = normalized_path;
    if (!format.isEmpty())
    {
        result = format.arg(normalized_path);
    }
    return result;
}
}  // namespace

namespace QtWidgetsCommonLib
{
/**
 * @brief Constructs an empty recent-files facade for a menu.
 * @param menu Menu receiving generated recent-file actions.
 * @param parent Optional QObject parent.
 */
RecentFilesMenuController::RecentFilesMenuController(QMenu* menu, QObject* parent): QObject(parent)
{
    m_model = new RecentItemsModel(this);
    const RecentItemsMenuRoles roles{
        to_role_id(RecentItemRole::Title),   to_role_id(RecentItemRole::Identity),
        to_role_id(RecentItemRole::Enabled), to_role_id(RecentItemRole::ToolTip),
        to_role_id(RecentItemRole::Icon),    to_role_id(RecentItemRole::StatusText)};
    m_binder = new RecentItemsMenuBinder(menu, m_model, roles, this);
    m_remove_unavailable_action = new QAction(this);
    m_binder->set_footer_actions({m_remove_unavailable_action});
    connect(m_remove_unavailable_action, &QAction::triggered, this,
            &RecentFilesMenuController::remove_unavailable_requested);
    connect(m_binder, &RecentItemsMenuBinder::item_activated, this,
            [this](const QVariant& identity) { emit file_activated(identity.toString()); });
    connect(m_binder, &RecentItemsMenuBinder::clear_requested, this,
            &RecentFilesMenuController::clear_requested);
    connect(m_binder, &RecentItemsMenuBinder::contains_disabled_items_changed, this,
            &RecentFilesMenuController::contains_unavailable_files_changed);
    connect(m_binder, &RecentItemsMenuBinder::contains_disabled_items_changed,
            m_remove_unavailable_action, &QAction::setEnabled);
}

/**
 * @brief Replaces the ordered file paths represented by the facade.
 * @param file_paths Application-owned recent-file paths in display order.
 */
auto RecentFilesMenuController::set_files(QStringList file_paths) -> void
{
    m_file_paths = std::move(file_paths);
    rebuild_presentations();
}

/**
 * @brief Replaces all host-owned user-facing text.
 * @param texts Text and format values used for actions and file states.
 */
auto RecentFilesMenuController::set_texts(RecentFilesMenuTexts texts) -> void
{
    m_texts = std::move(texts);
    m_binder->set_empty_text(m_texts.empty_text);
    m_binder->set_clear_text(m_texts.clear_text);
    m_binder->set_disabled_text_format(m_texts.disabled_text_format);
    m_remove_unavailable_action->setText(m_texts.remove_unavailable_text);
    m_remove_unavailable_action->setVisible(!m_texts.remove_unavailable_text.isEmpty());
    rebuild_presentations();
}

/**
 * @brief Sets the icon shared by generated recent-file entries.
 * @param icon Icon displayed for each represented file.
 */
auto RecentFilesMenuController::set_file_icon(QIcon icon) -> void
{
    m_file_icon = std::move(icon);
    rebuild_presentations();
}

/**
 * @brief Sets the maximum number of file entries represented in the menu.
 * @param maximum Maximum visible file entries.
 */
auto RecentFilesMenuController::set_maximum_visible_items(int maximum) -> void
{
    m_binder->set_maximum_visible_items(maximum);
}

/** @brief Re-inspects all paths and refreshes their transient presentation state. */
auto RecentFilesMenuController::refresh_file_status() -> void
{
    rebuild_presentations();
}

/**
 * @brief Returns the owned model for optional list-view reuse.
 * @return Model containing the current recent-file presentation.
 */
auto RecentFilesMenuController::model() -> QAbstractItemModel*
{
    QAbstractItemModel* result = m_model;
    return result;
}

/** @brief Rebuilds the model from the current paths and configuration. */
auto RecentFilesMenuController::rebuild_presentations() -> void
{
    QVector<RecentItemPresentation> items;
    items.reserve(m_file_paths.size());
    for (const QString& file_path: std::as_const(m_file_paths))
    {
        const QtCommonLib::LocalFileStatus status = QtCommonLib::inspect_local_file(file_path);
        items.push_back(make_presentation(file_path, status));
    }
    m_model->set_items(std::move(items));
}

/**
 * @brief Maps one inspected path to a domain-neutral presentation item.
 * @param file_path Original application-owned file path.
 * @param file_status Current local filesystem status.
 * @return Presentation values consumed by lists and menus.
 */
auto RecentFilesMenuController::make_presentation(
    const QString& file_path,
    const QtCommonLib::LocalFileStatus& file_status) const -> RecentItemPresentation
{
    RecentItemPresentation result;
    result.identity = file_path;
    result.title = QFileInfo(file_path).fileName();
    result.secondary_text = file_path;
    result.icon = m_file_icon;

    switch (file_status.availability)
    {
    case QtCommonLib::LocalFileAvailability::Available:
        result.enabled = true;
        result.status_text = m_texts.available_status_text;
        result.tooltip =
            format_tooltip(m_texts.available_tooltip_format, file_status.normalized_path);
        break;
    case QtCommonLib::LocalFileAvailability::Missing:
        result.enabled = false;
        result.status_text = m_texts.missing_status_text;
        result.tooltip =
            format_tooltip(m_texts.missing_tooltip_format, file_status.normalized_path);
        break;
    case QtCommonLib::LocalFileAvailability::NotReadable:
        result.enabled = false;
        result.status_text = m_texts.not_readable_status_text;
        result.tooltip =
            format_tooltip(m_texts.not_readable_tooltip_format, file_status.normalized_path);
        break;
    case QtCommonLib::LocalFileAvailability::NotAFile:
        result.enabled = false;
        result.status_text = m_texts.not_a_file_status_text;
        result.tooltip =
            format_tooltip(m_texts.not_a_file_tooltip_format, file_status.normalized_path);
        break;
    }
    return result;
}
}  // namespace QtWidgetsCommonLib
