/**
 * @file TwoLineItemDelegateTest.cpp
 * @brief Tests configurable sizing and rendering of the two-line item delegate.
 */

#include "QtWidgetsCommonLib/ItemViews/Delegates/TwoLineItemDelegateTest.h"

#include <QImage>
#include <QPainter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>

#include "QtWidgetsCommonLib/ItemViews/Delegates/TwoLineItemDelegate.h"

namespace
{
constexpr int k_secondary_role = Qt::UserRole + 1;
constexpr int k_metadata_role = Qt::UserRole + 2;
constexpr int k_status_role = Qt::UserRole + 3;

/**
 * @brief Creates a model item containing every supported semantic value.
 * @return Populated item owned by the caller.
 */
[[nodiscard]] auto make_item() -> QStandardItem*
{
    auto* result = new QStandardItem(QStringLiteral("Primary title"));
    result->setData(QStringLiteral("Secondary text"), k_secondary_role);
    result->setData(42, k_metadata_role);
    result->setData(QStringLiteral("Unavailable"), k_status_role);
    result->setEnabled(false);
    return result;
}
}  // namespace

/**
 * @brief Verifies that layout settings determine the preferred row height.
 */
TEST_F(TwoLineItemDelegateTest, SizeHintUsesConfiguredLayout)
{
    QtWidgetsCommonLib::TwoLineItemDelegate::Roles roles;
    QtWidgetsCommonLib::TwoLineItemDelegate delegate(roles);
    QtWidgetsCommonLib::TwoLineItemDelegate::Layout layout;
    layout.content_margins = QMargins(2, 7, 2, 9);
    layout.icon_size = QSize(16, 40);
    layout.row_spacing = 3;
    delegate.set_layout(layout);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 240, 20);
    const QSize result = delegate.sizeHint(option, QModelIndex());

    EXPECT_EQ(result.width(), 240);
    EXPECT_EQ(result.height(), 56);
}

/**
 * @brief Verifies that semantic roles and host metadata formatting are used while painting.
 */
TEST_F(TwoLineItemDelegateTest, PaintUsesConfiguredRolesAndMetadataFormatter)
{
    QStandardItemModel model;
    model.appendRow(make_item());

    QtWidgetsCommonLib::TwoLineItemDelegate::Roles roles;
    roles.secondary = k_secondary_role;
    roles.metadata = k_metadata_role;
    roles.status = k_status_role;
    QtWidgetsCommonLib::TwoLineItemDelegate delegate(roles);
    bool formatter_called = false;
    delegate.set_metadata_formatter([&formatter_called](const QVariant& value) {
        formatter_called = true;
        return QStringLiteral("Value: %1").arg(value.toInt());
    });

    QImage image(320, 80, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = image.rect();
    option.state = QStyle::State_Enabled;
    delegate.paint(&painter, option, model.index(0, 0));
    painter.end();

    EXPECT_TRUE(formatter_called);
    EXPECT_FALSE(image.isNull());
}
