#include "emojilistdelegate.h"
#include <QPainter>
#include <QApplication>
#include <QStyleOptionViewItem>

EmojiListDelegate::EmojiListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QSize EmojiListDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    // 单元格大小（宽 x 高）
    return QSize(120, 120);
}

void EmojiListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QRect rect = option.rect;
    // 背景（圆角白底 + 毛玻璃感可用更复杂的绘制，这里绘制圆角背景）
    // 【优化】：区分三种状态 - 选中、悬停、普通
    QColor bg;
    if (option.state & QStyle::State_Selected) {
        // 选中状态：更明显的背景色
        bg = QColor(230, 240, 255, 240);  // 淡蓝色背景
    } else if (option.state & QStyle::State_MouseOver) {
        // 悬停状态：半透明白色
        bg = QColor(255, 255, 255, 230);
    } else {
        // 普通状态：更透明的白色
        bg = QColor(255, 255, 255, 245);
    }
    
    QBrush brush(bg);
    painter->setRenderHint(QPainter::Antialiasing, true);

    QRect rBg = rect.adjusted(4,4,-4,-4);
    QPainterPath path;
    path.addRoundedRect(rBg, 8, 8);
    painter->fillPath(path, brush);

    // 绘制缩略图（来自 DecorationRole）
    QVariant dec = index.data(Qt::DecorationRole);
    QPixmap pix;
    if (dec.canConvert<QPixmap>()) pix = dec.value<QPixmap>();
    else if (dec.canConvert<QIcon>()) pix = dec.value<QIcon>().pixmap(96,96);

    if (!pix.isNull()) {
        // 计算图片区域（居中）
        QSize pixSz = pix.size();
        QSize targetSz(rBg.width()-12, rBg.height()-36);
        QPixmap scaled = pix.scaled(targetSz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPoint center(rBg.left() + (rBg.width()-scaled.width())/2, rBg.top() + 8);
        painter->drawPixmap(center, scaled);
    }

    // 绘制文件名（可隐藏由 MainWindow 控制，存在 DisplayRoleTextHidden flag）
    bool showName = index.data(Qt::UserRole + 10).toBool(); // UPGRADE: 控制是否显示文件名（由 MainWindow 设置）
    if (showName) {
        QString text = index.data(Qt::DisplayRole).toString();
        QRect textRect(rBg.left()+6, rBg.bottom()-24, rBg.width()-12, 20);
        painter->setPen(QColor(80,80,80));
        painter->drawText(textRect, Qt::AlignCenter | Qt::TextSingleLine, text);
    }

    // 【优化】：高亮边框 - 区分选中和悬停状态
    if (option.state & QStyle::State_Selected) {
        // 选中状态：iOS蓝色高亮边框，更粗更明显
        painter->setPen(QPen(QColor(0, 122, 255, 200), 2.5));
        painter->drawPath(path);
    } else if (option.state & QStyle::State_MouseOver) {
        // 悬停状态：较淡的蓝色边框
        painter->setPen(QPen(QColor(0, 122, 255, 80), 2));
        painter->drawPath(path);
    }

    painter->restore();
}
