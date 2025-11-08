/*
* 文件名：emojilistdelegate.h
* 日期：2025-10-21
* 该文件功能描述：自定义网格单元绘制（缩略图 + 可选文件名 + hover 高亮效果）
* 与该文件相关联的其他文件：emojilistdelegate.cpp, mainwindow.cpp, emojilistwidget.cpp
*/

/*
* 文件名：emojilistdelegate.h
* 日期：2025-10-21
* 该文件功能大致描述：自定义列表项委托，负责绘制每个表情项的外观（圆角背景、缩略图、文件名、悬停/选中高亮效果）。
* 该文件函数功能描述：
*   - paint()：重写绘制函数，绘制圆角背景、悬停/选中高亮边框、表情缩略图以及文件名（可选）
*   - sizeHint()：返回每个列表项的尺寸提示
*   - setShowNames()：控制是否显示文件名
* 与该文件相关联的其他文件：emojilistdelegate.cpp, emojilistwidget.h, emojilistwidget.cpp, mainwindow.h, mainwindow.cpp
*/

#ifndef EMOJILISTDELEGATE_H
#define EMOJILISTDELEGATE_H

#include <QStyledItemDelegate>

class EmojiListDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit EmojiListDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // EMOJILISTDELEGATE_H
