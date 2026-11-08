/**
 * @file RecentItemsMenuBinderTest.cpp
 * @brief Tests model synchronization and semantic signals of the recent-items menu binder.
 */

#include "QtWidgetsCommonLib/RecentItems/RecentItemsMenuBinderTest.h"

#include <QAction>
#include <QMenu>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>

#include "QtWidgetsCommonLib/RecentItems/RecentItemsMenuBinder.h"

namespace
{
constexpr int k_identity_role = Qt::UserRole + 1;
constexpr int k_enabled_role = Qt::UserRole + 2;
constexpr int k_status_role = Qt::UserRole + 3;

/**
 * @brief Creates the semantic role configuration used by the independent test model.
 * @return Role configuration for binder tests.
 */
[[nodiscard]] auto make_roles() -> QtWidgetsCommonLib::RecentItemsMenuRoles
{
    const QtWidgetsCommonLib::RecentItemsMenuRoles roles{Qt::DisplayRole,    k_identity_role,
                                                         k_enabled_role,     Qt::ToolTipRole,
                                                         Qt::DecorationRole, k_status_role};
    return roles;
}

/**
 * @brief Creates a model item with all semantic roles consumed by the binder.
 * @param text Action text.
 * @param identity Activation identity.
 * @param enabled Whether the generated action is enabled.
 * @param tooltip Optional action tooltip.
 * @param status Optional short status text.
 * @return Newly allocated model item owned by the receiving model.
 */
[[nodiscard]] auto make_item(const QString& text, const QString& identity, bool enabled,
                             const QString& tooltip = {},
                             const QString& status = {}) -> QStandardItem*
{
    auto* item = new QStandardItem(text);
    item->setData(identity, k_identity_role);
    item->setData(enabled, k_enabled_role);
    item->setData(tooltip, Qt::ToolTipRole);
    item->setData(status, k_status_role);
    return item;
}
}  // namespace

/** @brief Verifies generated actions expose model data and semantic activation identities. */
TEST_F(RecentItemsMenuBinderTest, MirrorsItemsAndEmitsActivationIdentity)
{
    QStandardItemModel model;
    model.appendRow(make_item(QStringLiteral("Available item"), QStringLiteral("available-id"),
                              true, QStringLiteral("Open the available item")));
    model.appendRow(make_item(QStringLiteral("Missing item"), QStringLiteral("missing-id"), false,
                              {}, QStringLiteral("Not found")));
    QMenu menu;
    QtWidgetsCommonLib::RecentItemsMenuBinder binder(&menu, &model, make_roles());
    binder.set_disabled_text_format(QStringLiteral("%1 — %2"));
    QSignalSpy activated_spy(&binder, &QtWidgetsCommonLib::RecentItemsMenuBinder::item_activated);

    const QList<QAction*> actions = menu.actions();
    ASSERT_EQ(actions.size(), 2);
    EXPECT_EQ(actions.at(0)->text(), QStringLiteral("Available item"));
    EXPECT_EQ(actions.at(0)->toolTip(), QStringLiteral("Open the available item"));
    EXPECT_FALSE(actions.at(1)->isEnabled());
    EXPECT_EQ(actions.at(1)->text(), QStringLiteral("Missing item — Not found"));

    actions.at(0)->trigger();
    ASSERT_EQ(activated_spy.count(), 1);
    EXPECT_EQ(activated_spy.at(0).at(0).toString(), QStringLiteral("available-id"));
}

/** @brief Verifies model changes, visible limits, host text, and clear intent are handled locally.
 */
TEST_F(RecentItemsMenuBinderTest, ReactsToModelChangesAndOptionalActions)
{
    QStandardItemModel model;
    QMenu menu;
    QAction footer_action(QStringLiteral("Footer"));
    QtWidgetsCommonLib::RecentItemsMenuBinder binder(&menu, &model, make_roles());
    binder.set_maximum_visible_items(1);
    binder.set_empty_text(QStringLiteral("No recent items"));
    binder.set_clear_text(QStringLiteral("Clear recent items"));
    binder.set_footer_actions({&footer_action});
    QSignalSpy clear_spy(&binder, &QtWidgetsCommonLib::RecentItemsMenuBinder::clear_requested);
    QSignalSpy disabled_spy(
        &binder, &QtWidgetsCommonLib::RecentItemsMenuBinder::contains_disabled_items_changed);

    ASSERT_EQ(menu.actions().size(), 4);
    EXPECT_EQ(menu.actions().at(0)->text(), QStringLiteral("No recent items"));
    EXPECT_FALSE(menu.actions().at(0)->isEnabled());
    EXPECT_FALSE(menu.actions().constLast()->isEnabled());

    model.appendRow(make_item(QStringLiteral("First"), QStringLiteral("first"), true));
    model.appendRow(make_item(QStringLiteral("Second"), QStringLiteral("second"), false));

    ASSERT_EQ(menu.actions().size(), 4);
    EXPECT_EQ(menu.actions().at(0)->text(), QStringLiteral("First"));
    EXPECT_EQ(menu.actions().at(2), &footer_action);
    EXPECT_TRUE(menu.actions().constLast()->isEnabled());
    ASSERT_FALSE(disabled_spy.isEmpty());
    EXPECT_TRUE(disabled_spy.constLast().at(0).toBool());
    menu.actions().constLast()->trigger();
    EXPECT_EQ(clear_spy.count(), 1);
}
