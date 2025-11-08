#include "emojilistwidget.h"
#include <QStandardItemModel>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include <QInputDialog>
#include <QFileInfo>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QDropEvent>
#include <QFont>
#include "emoji_meta.h"  // for DEBUG_LOG

EmojiListWidget::EmojiListWidget(QWidget *parent)
    : QListView(parent)
{
    DEBUG_LOG("EmojiListWidget constructor");
    setViewMode(QListView::IconMode);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Snap);
    setSpacing(12);
    setDragDropMode(QAbstractItemView::InternalMove); // UPGRADE: 允许内部拖放改变顺序
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setMouseTracking(true);
    DEBUG_LOG("EmojiListWidget initialized with IconMode, drag-drop enabled");
}

void EmojiListWidget::mousePressEvent(QMouseEvent *event)
{
    QModelIndex idx = indexAt(event->pos());
    if (idx.isValid() && event->button() == Qt::LeftButton) {
        DEBUG_LOG("Mouse pressed on item at row:" << idx.row());
        // 【修改位置】：记录按下位置，延迟发送点击信号
        // 【修改原因】：不在 mousePressEvent 中立即发送 itemClickedIndex 信号，
        //             而是记录按下位置，在 mouseReleaseEvent 中判断是否为纯点击（无拖动）
        m_pressedIndex = idx;
        m_pressPos = event->pos();
        m_isDragging = false;
    }
    QListView::mousePressEvent(event);
}

// 【新增】：鼠标移动事件 - 判断是否开始拖动
void EmojiListWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 如果鼠标移动距离超过 5 像素，则标记为拖动中
    if (m_pressedIndex.isValid() && !m_isDragging) {
        QPoint delta = event->pos() - m_pressPos;
        if (delta.manhattanLength() > 5) {
            m_isDragging = true;
            DEBUG_LOG("Drag started for item at row:" << m_pressedIndex.row());
        }
    }
    QListView::mouseMoveEvent(event);
}

// 【新增】：鼠标释放事件 - 判断是否为纯点击
void EmojiListWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // 如果记录了按下位置且未进行拖动，则发出点击信号
    if (m_pressedIndex.isValid() && !m_isDragging && event->button() == Qt::LeftButton) {
        DEBUG_LOG("Item clicked (no drag) at row:" << m_pressedIndex.row());
        emit itemClickedIndex(m_pressedIndex);
    } else if (m_isDragging) {
        DEBUG_LOG("Drag operation completed");
    }
    
    // 清空标记
    m_pressedIndex = QModelIndex();
    m_isDragging = false;
    
    QListView::mouseReleaseEvent(event);
}

void EmojiListWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex idx = indexAt(event->pos());
    if (idx.isValid()) {
        QString path = idx.data(Qt::UserRole).toString();
        
        // 【优化】：双击前检查图片是否可加载，避免无效图片弹出错误提示后无法交互
        QPixmap testLoad(path);
        if (testLoad.isNull()) {
            DEBUG_LOG("Double-click blocked: image cannot be loaded:" << path);
            return;  // 无法加载的图片不响应双击
        }
        
        DEBUG_LOG("Item double-clicked, requesting preview:" << path);
        emit requestPreview(path); // UPGRADE: 双击预览
    }
}

void EmojiListWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex idx = indexAt(event->pos());
    DEBUG_LOG("Context menu requested, valid index:" << idx.isValid());
    
    QMenu menu(this);
    
    // 【苹果风格样式优化】：更圆润、精致、小巧的菜单
    menu.setStyleSheet(
        "QMenu {"
        "    background-color: rgba(255, 255, 255, 250);"  // 更高透明度
        "    border: 0.5px solid rgba(220, 220, 220, 150);"  // 更细的边框
        "    border-radius: 10px;"  // 稍小的圆角
        "    padding: 6px 0px;"  // 减小内边距
        "    font-size: 12px;"  // 缩小字体
        "}"
        "QMenu::item {"
        "    background-color: transparent;"
        "    padding: 8px 20px 8px 20px;"  // 减小项的内边距
        "    margin: 1px 6px;"  // 减小项的外边距
        "    border-radius: 5px;"  // 更小的圆角
        "    color: #333333;"
        "    min-width: 120px;"  // 限制最小宽度
        "}"
        "QMenu::item:selected {"
        "    background-color: rgba(0, 122, 255, 160);"  // 稍降低不透明度
        "    color: white;"
        "}"
        "QMenu::item:pressed {"
        "    background-color: rgba(0, 122, 255, 200);"
        "}"
        "QMenu::separator {"
        "    height: 0.5px;"  // 更细的分隔线
        "    background: rgba(210, 210, 210, 100);"  // 更淡的分隔线
        "    margin: 4px 10px;"  // 减小分隔线边距
        "}"
    );
    
    if (!idx.isValid()) {
        // 空白处右键——可考虑添加"导入"菜单(MainWindow 也有工具栏)
        QAction *importAct = menu.addAction(tr("  导入表情..."));  // 使用空格代替emoji
        importAct->setFont(QFont("Microsoft YaHei UI", 11));  // 缩小字体
        QAction *chosen = menu.exec(event->globalPos());
        if (chosen == importAct) {
            // emit something? leave to main window toolbar action
        }
        return;
    }

    // 使用更清晰的文字描述,不使用emoji图标
    QAction *previewAct = menu.addAction(tr("  预览"));
    QAction *renameAct = menu.addAction(tr("  重命名"));
    QAction *copyPathAct = menu.addAction(tr("  复制路径"));
    
    menu.addSeparator();
    
    QAction *delAct = menu.addAction(tr("  删除"));
    
    // 设置统一字体,缩小尺寸
    QFont menuFont("Microsoft YaHei UI", 11);  // 从13改为11
    previewAct->setFont(menuFont);
    renameAct->setFont(menuFont);
    copyPathAct->setFont(menuFont);
    
    // 删除项使用稍粗字体但不要太粗
    QFont delFont("Microsoft YaHei UI", 11, QFont::DemiBold);  // 使用DemiBold而不是Bold
    delAct->setFont(delFont);

    QAction *chosen = menu.exec(event->globalPos());
    if (!chosen) return;
    if (chosen == previewAct) {
        emit requestPreview(idx.data(Qt::UserRole).toString());
    } else if (chosen == renameAct) {
        emit requestRename(idx);
    } else if (chosen == copyPathAct) {
        emit requestCopyPath(idx.data(Qt::UserRole).toString());
    } else if (chosen == delAct) {
        emit requestDeleteIndex(idx);
    }
}

void EmojiListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    // Allow dragging inside
    event->acceptProposedAction();
    QListView::dragMoveEvent(event);
}
