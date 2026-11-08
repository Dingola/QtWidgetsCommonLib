#pragma once

#include <QIcon>
#include <QObject>
#include <QString>
#include <QStringList>

#include "QtWidgetsCommonLib/ApiMacro.h"

class QMenu;
class QAbstractItemModel;
class QAction;

namespace QtCommonLib
{
struct LocalFileStatus;
}

namespace QtWidgetsCommonLib
{
class RecentItemsMenuBinder;
class RecentItemsModel;
struct RecentItemPresentation;

/**
 * @file RecentFilesMenuController.h
 * @brief Declares a high-level facade for model-backed recent-files menus.
 */

/**
 * @struct RecentFilesMenuTexts
 * @brief Provides all host-owned user-facing text used by a recent-files menu.
 */
struct QTWIDGETSCOMMONLIB_API RecentFilesMenuTexts {
        QString empty_text;                   ///< Disabled action shown for an empty list.
        QString clear_text;                   ///< Optional action requesting list clearing.
        QString remove_unavailable_text;      ///< Optional action requesting unavailable cleanup.
        QString disabled_text_format;         ///< Optional `%1` title and `%2` status format.
        QString available_status_text;        ///< Status text for an available file.
        QString missing_status_text;          ///< Status text for a missing file.
        QString not_readable_status_text;     ///< Status text for an unreadable file.
        QString not_a_file_status_text;       ///< Status text for a path that is not a file.
        QString available_tooltip_format;     ///< Optional `%1` normalized-path format.
        QString missing_tooltip_format;       ///< Optional `%1` normalized-path format.
        QString not_readable_tooltip_format;  ///< Optional `%1` normalized-path format.
        QString not_a_file_tooltip_format;    ///< Optional `%1` normalized-path format.
};

/**
 * @class RecentFilesMenuController
 * @brief Composes file inspection, presentation model, and menu binding for a host application.
 *
 * The host owns persistence and file-opening behavior. This facade owns only transient
 * presentation state and generated menu actions.
 */
class QTWIDGETSCOMMONLIB_API RecentFilesMenuController final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs an empty recent-files facade for a menu.
         * @param menu Menu receiving generated recent-file actions.
         * @param parent Optional QObject parent.
         */
        explicit RecentFilesMenuController(QMenu* menu, QObject* parent = nullptr);

        /**
         * @brief Replaces the ordered file paths represented by the facade.
         * @param file_paths Application-owned recent-file paths in display order.
         */
        auto set_files(QStringList file_paths) -> void;

        /**
         * @brief Replaces all host-owned user-facing text.
         * @param texts Text and format values used for actions and file states.
         */
        auto set_texts(RecentFilesMenuTexts texts) -> void;

        /**
         * @brief Sets the icon shared by generated recent-file entries.
         * @param icon Icon displayed for each represented file.
         */
        auto set_file_icon(QIcon icon) -> void;

        /**
         * @brief Sets the maximum number of file entries represented in the menu.
         * @param maximum Maximum visible file entries.
         */
        auto set_maximum_visible_items(int maximum) -> void;

        /** @brief Re-inspects all paths and refreshes their transient presentation state. */
        auto refresh_file_status() -> void;

        /**
         * @brief Returns the owned model for optional list-view reuse.
         * @return Model containing the current recent-file presentation.
         */
        [[nodiscard]] auto model() -> QAbstractItemModel*;

    signals:
        /**
         * @brief Requests activation of an available recent file.
         * @param file_path Original application-owned file path.
         */
        void file_activated(const QString& file_path);

        /** @brief Requests clearing recent-file metadata in the host application. */
        void clear_requested();

        /** @brief Requests removal of unavailable recent-file metadata by the host. */
        void remove_unavailable_requested();

        /**
         * @brief Reports whether the represented list currently contains unavailable files.
         * @param contains_unavailable_files True when at least one file is unavailable.
         */
        void contains_unavailable_files_changed(bool contains_unavailable_files);

    private:
        /**
         * @brief Rebuilds the model from the current paths and configuration.
         */
        auto rebuild_presentations() -> void;

        /**
         * @brief Maps one inspected path to a domain-neutral presentation item.
         * @param file_path Original application-owned file path.
         * @param file_status Current local filesystem status.
         * @return Presentation values consumed by lists and menus.
         */
        [[nodiscard]] auto make_presentation(const QString& file_path,
                                             const QtCommonLib::LocalFileStatus& file_status) const
            -> RecentItemPresentation;

    private:
        RecentItemsModel* m_model{nullptr};
        RecentItemsMenuBinder* m_binder{nullptr};
        QAction* m_remove_unavailable_action{nullptr};
        QStringList m_file_paths;
        RecentFilesMenuTexts m_texts;
        QIcon m_file_icon;
};
}  // namespace QtWidgetsCommonLib
