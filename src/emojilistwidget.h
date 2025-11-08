/*
* 文件名：emojilistwidget.h
* 日期：2025-10-21
* 该文件功能大致描述：自定义列表视图，响应双击（预览）、单击（复制到剪贴板）、右键菜单（删除/重命名/复制路径/预览）以及拖放排序功能。
* 该文件函数功能描述：
*   - mouseDoubleClickEvent()：双击预览图片，并在双击前检测图片是否可加载，防止无效图片弹出错误提示
*   - mousePressEvent()/mouseReleaseEvent()/mouseMoveEvent()：处理鼠标事件，区分点击与拖动操作
*   - contextMenuEvent()：右键菜单，提供预览、重命名、复制路径、删除等功能
*   - dragMoveEvent()：处理拖放移动事件
* 与该文件相关联的其他文件：emojilistwidget.cpp, emojilistdelegate.h, emojilistdelegate.cpp, mainwindow.h, mainwindow.cpp
*/

#ifndef EMOJILISTWIDGET_H
#define EMOJILISTWIDGET_H

#include <QListView>
#include <QMouseEvent>
#include <QMenu>

class EmojiListWidget : public QListView {
    Q_OBJECT
public:
    explicit EmojiListWidget(QWidget *parent = nullptr);

signals:
    void requestDeleteIndex(const QModelIndex &index);
    void requestPreview(const QString &path);
    void requestRename(const QModelIndex &index);
    void requestCopyPath(const QString &path);
    void itemClickedIndex(const QModelIndex &index); // UPGRADE: 单击事件（用来复制图片到剪贴板）

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;  // 【新增】：检测拖动开始
    void mouseReleaseEvent(QMouseEvent *event) override;  // 【新增】：判断是否为点击
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

private:
    QModelIndex m_pressedIndex;  // 【新增】：记录按下时的项索引
    QPoint m_pressPos;  // 【新增】：记录按下时的位置
    bool m_isDragging = false;  // 【新增】：标记是否正在拖动
};

#endif // EMOJILISTWIDGET_H
