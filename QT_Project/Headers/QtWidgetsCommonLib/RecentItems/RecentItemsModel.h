#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVector>

#include "QtWidgetsCommonLib/ApiMacro.h"
#include "QtWidgetsCommonLib/RecentItems/RecentItemPresentation.h"

/**
 * @file RecentItemsModel.h
 * @brief Declares a typed list model for domain-independent recent-item presentations.
 */
namespace QtWidgetsCommonLib
{
class QTWIDGETSCOMMONLIB_API RecentItemsModel: public QAbstractListModel
{
    public:
        /**
         * @brief Constructs an empty recent-items model.
         * @param parent Optional QObject parent.
         */
        explicit RecentItemsModel(QObject* parent = nullptr);

        /**
         * @brief Returns the number of presentation items.
         * @param parent Parent index, which must be invalid for this flat model.
         * @return Number of root-level items.
         */
        auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;

        /**
         * @brief Returns presentation data for an item and semantic role.
         * @param index Model index to inspect.
         * @param role Qt or semantic recent-item role.
         * @return Requested value or an invalid QVariant.
         */
        auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;

        /**
         * @brief Returns selectable flags and the presentation-controlled enabled state.
         * @param index Model index to inspect.
         * @return Item flags for the requested index.
         */
        auto flags(const QModelIndex& index) const -> Qt::ItemFlags override;

        /**
         * @brief Exposes stable semantic role names.
         * @return Mapping from role ids to role names.
         */
        auto roleNames() const -> QHash<int, QByteArray> override;

        /**
         * @brief Atomically replaces all presentation items.
         * @param items New presentation items in display order.
         */
        auto set_items(QVector<RecentItemPresentation> items) -> void;

        /**
         * @brief Inserts one presentation item at a requested row.
         * @param row Insertion row in the inclusive range from zero through `rowCount()`.
         * @param item Presentation item to insert.
         * @return True when the row was valid and the item was inserted.
         */
        auto insert_item(int row, RecentItemPresentation item) -> bool;

        /**
         * @brief Replaces one presentation item and emits a targeted data change.
         * @param row Existing row to replace.
         * @param item Replacement presentation item.
         * @return True when the row was valid and the item was replaced.
         */
        auto update_item(int row, RecentItemPresentation item) -> bool;

        /**
         * @brief Removes one presentation item.
         * @param row Existing row to remove.
         * @return True when the row was valid and the item was removed.
         */
        auto remove_item(int row) -> bool;

        /**
         * @brief Removes all presentation items.
         */
        auto clear() -> void;

    private:
        QVector<RecentItemPresentation> m_items;  ///< Typed presentation items in display order.
};
}  // namespace QtWidgetsCommonLib
