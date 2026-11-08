/**
 * @file RecentItemsModel.cpp
 * @brief Implements the typed recent-items list model.
 */

#include "QtWidgetsCommonLib/RecentItems/RecentItemsModel.h"

#include <utility>

#include "QtWidgetsCommonLib/RecentItems/RecentRoles.h"

namespace QtWidgetsCommonLib
{

/**
 * @brief Constructs an empty recent-items model.
 * @param parent Optional QObject parent.
 */
RecentItemsModel::RecentItemsModel(QObject* parent): QAbstractListModel(parent) {}

/**
 * @brief Returns the number of presentation items.
 * @param parent Parent index, which must be invalid for this flat model.
 * @return Number of root-level items.
 */
auto RecentItemsModel::rowCount(const QModelIndex& parent) const -> int
{
    int count = 0;

    if (!parent.isValid())
    {
        count = m_items.size();
    }

    return count;
}

/**
 * @brief Returns presentation data for an item and semantic role.
 * @param index Model index to inspect.
 * @param role Qt or semantic recent-item role.
 * @return Requested value or an invalid QVariant.
 */
auto RecentItemsModel::data(const QModelIndex& index, int role) const -> QVariant
{
    QVariant value;
    if (index.isValid() && index.column() == 0 && index.row() >= 0 && index.row() < m_items.size())
    {
        const RecentItemPresentation& item = m_items.at(index.row());
        if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole ||
            role == to_role_id(RecentItemRole::Title))
        {
            value = item.title;
        }
        else if (role == Qt::DecorationRole || role == to_role_id(RecentItemRole::Icon))
        {
            value = item.icon;
        }
        else if (role == Qt::ToolTipRole || role == Qt::AccessibleDescriptionRole ||
                 role == to_role_id(RecentItemRole::ToolTip))
        {
            value = item.tooltip;
        }
        else if (role == to_role_id(RecentItemRole::Identity))
        {
            value = item.identity;
        }
        else if (role == to_role_id(RecentItemRole::SecondaryText))
        {
            value = item.secondary_text;
        }
        else if (role == to_role_id(RecentItemRole::LastUsed))
        {
            value = item.last_used;
        }
        else if (role == to_role_id(RecentItemRole::Enabled))
        {
            value = item.enabled;
        }
        else if (role == to_role_id(RecentItemRole::StatusText))
        {
            value = item.status_text;
        }
    }
    return value;
}

/**
 * @brief Returns selectable flags and the presentation-controlled enabled state.
 * @param index Model index to inspect.
 * @return Item flags for the requested index.
 */
auto RecentItemsModel::flags(const QModelIndex& index) const -> Qt::ItemFlags
{
    Qt::ItemFlags item_flags = Qt::NoItemFlags;

    if (index.isValid())
    {
        item_flags = Qt::ItemIsSelectable;
        if (index.row() >= 0 && index.row() < m_items.size() && m_items.at(index.row()).enabled)
        {
            item_flags |= Qt::ItemIsEnabled;
        }
    }

    return item_flags;
}

/**
 * @brief Exposes stable semantic role names.
 * @return Mapping from role ids to role names.
 */
auto RecentItemsModel::roleNames() const -> QHash<int, QByteArray>
{
    QHash<int, QByteArray> roles;
    roles.insert(to_role_id(RecentItemRole::Identity), QByteArray("identity"));
    roles.insert(to_role_id(RecentItemRole::Title), QByteArray("title"));
    roles.insert(to_role_id(RecentItemRole::SecondaryText), QByteArray("secondary_text"));
    roles.insert(to_role_id(RecentItemRole::LastUsed), QByteArray("last_used"));
    roles.insert(to_role_id(RecentItemRole::Icon), QByteArray("icon"));
    roles.insert(to_role_id(RecentItemRole::Enabled), QByteArray("enabled"));
    roles.insert(to_role_id(RecentItemRole::StatusText), QByteArray("status_text"));
    roles.insert(to_role_id(RecentItemRole::ToolTip), QByteArray("tooltip"));
    return roles;
}

/**
 * @brief Atomically replaces all presentation items.
 * @param items New presentation items in display order.
 */
auto RecentItemsModel::set_items(QVector<RecentItemPresentation> items) -> void
{
    beginResetModel();
    m_items = std::move(items);
    endResetModel();
}

/**
 * @brief Inserts one presentation item at a requested row.
 * @param row Insertion row in the inclusive range from zero through `rowCount()`.
 * @param item Presentation item to insert.
 * @return True when the row was valid and the item was inserted.
 */
auto RecentItemsModel::insert_item(int row, RecentItemPresentation item) -> bool
{
    const bool valid_row = row >= 0 && row <= m_items.size();
    if (valid_row)
    {
        beginInsertRows(QModelIndex(), row, row);
        m_items.insert(row, std::move(item));
        endInsertRows();
    }
    return valid_row;
}

/**
 * @brief Replaces one presentation item and emits a targeted data change.
 * @param row Existing row to replace.
 * @param item Replacement presentation item.
 * @return True when the row was valid and the item was replaced.
 */
auto RecentItemsModel::update_item(int row, RecentItemPresentation item) -> bool
{
    const bool valid_row = row >= 0 && row < m_items.size();
    if (valid_row)
    {
        m_items[row] = std::move(item);
        const QModelIndex changed_index = index(row, 0);
        emit dataChanged(changed_index, changed_index);
    }
    return valid_row;
}

/**
 * @brief Removes one presentation item.
 * @param row Existing row to remove.
 * @return True when the row was valid and the item was removed.
 */
auto RecentItemsModel::remove_item(int row) -> bool
{
    const bool valid_row = row >= 0 && row < m_items.size();
    if (valid_row)
    {
        beginRemoveRows(QModelIndex(), row, row);
        m_items.removeAt(row);
        endRemoveRows();
    }
    return valid_row;
}

/**
 * @brief Removes all presentation items.
 */
auto RecentItemsModel::clear() -> void
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}
}  // namespace QtWidgetsCommonLib
