#include "QtWidgetsCommonLib/RecentItems/RecentItemsModelTest.h"

#include <QAction>
#include <QDateTime>
#include <QListView>
#include <QMenu>
#include <QSignalSpy>
#include <QString>

#include "QtWidgetsCommonLib/RecentItems/RecentItemsMenuBinder.h"
#include "QtWidgetsCommonLib/RecentItems/RecentItemsModel.h"
#include "QtWidgetsCommonLib/RecentItems/RecentRoles.h"

namespace
{
using QtWidgetsCommonLib::RecentItemPresentation;
using QtWidgetsCommonLib::RecentItemRole;
using QtWidgetsCommonLib::RecentItemsMenuBinder;
using QtWidgetsCommonLib::RecentItemsMenuRoles;
using QtWidgetsCommonLib::RecentItemsModel;
using QtWidgetsCommonLib::to_role_id;
}  // namespace

/**
 * @brief Verifies semantic roles and standard Qt roles expose typed presentation values.
 */
TEST_F(RecentItemsModelTest, ExposesPresentationThroughStableRoles)
{
    const QDateTime last_used =
        QDateTime::fromString(QStringLiteral("2025-03-10T12:34:56"), Qt::ISODate);
    RecentItemPresentation item;
    item.identity = QStringLiteral("item-id");
    item.title = QStringLiteral("Recent item");
    item.secondary_text = QStringLiteral("Supporting text");
    item.last_used = last_used;
    item.status_text = QStringLiteral("Available");
    item.tooltip = QStringLiteral("Detailed information");

    RecentItemsModel model;
    model.set_items({item});
    const QModelIndex index = model.index(0, 0);

    ASSERT_TRUE(index.isValid());
    EXPECT_EQ(index.data(Qt::DisplayRole).toString(), item.title);
    EXPECT_EQ(index.data(to_role_id(RecentItemRole::Identity)).toString(),
              item.identity.toString());
    EXPECT_EQ(index.data(to_role_id(RecentItemRole::SecondaryText)).toString(),
              item.secondary_text);
    EXPECT_EQ(index.data(to_role_id(RecentItemRole::LastUsed)).toDateTime(), last_used);
    EXPECT_EQ(index.data(to_role_id(RecentItemRole::StatusText)).toString(), item.status_text);
    EXPECT_EQ(index.data(Qt::ToolTipRole).toString(), item.tooltip);
    EXPECT_EQ(index.data(Qt::AccessibleDescriptionRole), index.data(Qt::ToolTipRole));
}

/**
 * @brief Verifies replacement emits a reset and atomically changes the visible items.
 */
TEST_F(RecentItemsModelTest, ResetsWhenItemsAreReplaced)
{
    RecentItemsModel model;
    QSignalSpy reset_spy(&model, &QAbstractItemModel::modelReset);
    RecentItemPresentation first;
    first.title = QStringLiteral("First");
    RecentItemPresentation second;
    second.title = QStringLiteral("Second");

    model.set_items({first, second});

    EXPECT_EQ(reset_spy.count(), 1);
    EXPECT_EQ(model.rowCount(), 2);
    EXPECT_EQ(model.index(1, 0).data(Qt::DisplayRole).toString(), second.title);
}

/**
 * @brief Verifies disabled presentations remain visible but cannot be activated.
 */
TEST_F(RecentItemsModelTest, RepresentsDisabledItemsThroughFlags)
{
    RecentItemPresentation item;
    item.title = QStringLiteral("Unavailable item");
    item.enabled = false;
    item.tooltip = QStringLiteral("The item is unavailable");
    RecentItemsModel model;
    model.set_items({item});

    const QModelIndex index = model.index(0, 0);
    EXPECT_TRUE(index.flags().testFlag(Qt::ItemIsSelectable));
    EXPECT_FALSE(index.flags().testFlag(Qt::ItemIsEnabled));
    EXPECT_EQ(index.data(Qt::ToolTipRole).toString(), item.tooltip);
}

/**
 * @brief Verifies clearing the list preserves valid list-model semantics.
 */
TEST_F(RecentItemsModelTest, ClearsPresentationItems)
{
    RecentItemPresentation item;
    item.title = QStringLiteral("Recent item");
    RecentItemsModel model;
    model.set_items({item});
    QSignalSpy reset_spy(&model, &QAbstractItemModel::modelReset);

    model.clear();

    EXPECT_EQ(reset_spy.count(), 1);
    EXPECT_EQ(model.rowCount(), 0);
    EXPECT_FALSE(model.index(0, 1).isValid());
}

/**
 * @brief Verifies insertion uses row signals and rejects invalid positions.
 */
TEST_F(RecentItemsModelTest, InsertsPresentationItemAtRequestedRow)
{
    RecentItemsModel model;
    QSignalSpy inserted_spy(&model, &QAbstractItemModel::rowsInserted);
    RecentItemPresentation item;
    item.title = QStringLiteral("Inserted");

    EXPECT_TRUE(model.insert_item(0, item));
    EXPECT_FALSE(model.insert_item(2, item));
    EXPECT_EQ(inserted_spy.count(), 1);
    EXPECT_EQ(model.index(0, 0).data(Qt::DisplayRole).toString(), item.title);
}

/**
 * @brief Verifies targeted replacement emits dataChanged without resetting the model.
 */
TEST_F(RecentItemsModelTest, UpdatesOnePresentationItem)
{
    RecentItemsModel model;
    RecentItemPresentation original;
    original.title = QStringLiteral("Original");
    model.set_items({original});
    QSignalSpy changed_spy(&model, &QAbstractItemModel::dataChanged);
    QSignalSpy reset_spy(&model, &QAbstractItemModel::modelReset);
    RecentItemPresentation replacement;
    replacement.title = QStringLiteral("Replacement");

    EXPECT_TRUE(model.update_item(0, replacement));
    EXPECT_FALSE(model.update_item(1, replacement));
    EXPECT_EQ(changed_spy.count(), 1);
    EXPECT_EQ(reset_spy.count(), 0);
    EXPECT_EQ(model.index(0, 0).data(Qt::DisplayRole).toString(), replacement.title);
}

/**
 * @brief Verifies removal uses row signals and preserves remaining presentation items.
 */
TEST_F(RecentItemsModelTest, RemovesPresentationItemAtRequestedRow)
{
    RecentItemsModel model;
    RecentItemPresentation first;
    first.title = QStringLiteral("First");
    RecentItemPresentation second;
    second.title = QStringLiteral("Second");
    model.set_items({first, second});
    QSignalSpy removed_spy(&model, &QAbstractItemModel::rowsRemoved);

    EXPECT_TRUE(model.remove_item(0));
    EXPECT_FALSE(model.remove_item(1));
    EXPECT_EQ(removed_spy.count(), 1);
    EXPECT_EQ(model.rowCount(), 1);
    EXPECT_EQ(model.index(0, 0).data(Qt::DisplayRole).toString(), second.title);
}

/**
 * @brief Verifies an independent host can share the model between a list and menu binder.
 */
TEST_F(RecentItemsModelTest, SuppliesIndependentListAndMenuConsumers)
{
    RecentItemsModel model;
    RecentItemPresentation item;
    item.identity = QStringLiteral("document-42");
    item.title = QStringLiteral("Independent item");
    model.set_items({item});
    QListView list_view;
    list_view.setModel(&model);
    QMenu menu;
    const RecentItemsMenuRoles roles{
        to_role_id(RecentItemRole::Title),   to_role_id(RecentItemRole::Identity),
        to_role_id(RecentItemRole::Enabled), to_role_id(RecentItemRole::ToolTip),
        to_role_id(RecentItemRole::Icon),    to_role_id(RecentItemRole::StatusText)};
    RecentItemsMenuBinder binder(&menu, &model, roles);
    QSignalSpy activated_spy(&binder, &RecentItemsMenuBinder::item_activated);

    binder.refresh();
    menu.actions().front()->trigger();

    ASSERT_EQ(list_view.model()->rowCount(), 1);
    ASSERT_EQ(menu.actions().size(), 1);
    ASSERT_EQ(activated_spy.count(), 1);
    EXPECT_EQ(menu.actions().front()->text(), item.title);
    EXPECT_EQ(activated_spy.front().front(), item.identity);
}
