/**
 * @file SplashIconWidget.cpp
 * @brief 悬浮启动图标组件的实现文件
 *
 * 本文件实现了 SplashIconWidget 类，用于在程序启动时显示一个可交互的悬浮 SVG 图标。
 * 图标支持动画效果、点击事件、以及平滑拖动。点击后可进入主窗口，
 * 主窗口关闭后重新显示该图标，形成“启动入口 → 主界面 → 返回启动入口”的完整逻辑。
 *
 * ### 实现要点
 *
 * #### 1. 视图与图标加载
 * - 使用 `QGraphicsView + QGraphicsScene + QGraphicsSvgItem` 实现矢量图标的显示。
 * - 图标缩放中心通过 `setTransformOriginPoint()` 设置为中心，确保动画自然。
 * - 透明背景通过 `Qt::WA_TranslucentBackground` + 无边框窗口实现。
 *
 * #### 2. 动画机制
 * - 所有缩放动画均通过 `QPropertyAnimation` 实现，属性名为 `"scale"`。
 * - 支持以下三种交互动画：
 *   - 鼠标悬停（enterEvent）：图标轻微缩小后回弹。
 *   - 鼠标离开（leaveEvent）：图标轻微放大后回弹。
 *   - 鼠标点击（mousePressEvent）：短暂压下（缩小）后恢复。
 * - 动画曲线使用 `QEasingCurve::OutBack`，产生自然的弹性回弹效果。
 *
 * #### 3. 拖动与事件过滤
 * - 为解决透明窗体 layered window 在 Win32 下出现
 *   “UpdateLayeredWindowIndirect 参数错误” 的问题，
 *   不再使用 `QDrag`，而是通过 `eventFilter` 捕获 `viewport()` 的鼠标事件，
 *   手动移动整个窗口实现拖动。
 * - 拖动时，计算鼠标全局坐标与窗口原始偏移量差值，实时移动窗体。
 *
 * #### 4. 信号触发机制
 * - 鼠标单击和双击均会触发 `clicked()` 信号；
 * - 外部使用者可连接该信号以打开主窗口或执行其他操作；
 * - 在主窗口关闭后，可重新显示本图标，实现“悬浮入口”循环。
 *
 * #### 5. 性能与兼容性
 * - 禁用了 QGraphicsDropShadowEffect 以避免透明层绘制错误；
 * - 所有动画为非阻塞异步执行；
 * - 兼容 Qt 5.9~Qt 5.15（Windows / macOS / Linux）。
 *
 * #### 6.主要改动说明：
 *  - 新增 onAddEmoji()：行为与 MainWindow::onAddFiles() 等价，支持批量导入多种图片。
 *  - 新增 appendEmojiItem(path)：将单个图片文件添加为缩略图项并加入场景与内部列表。
 *  - 新增 layoutEmojis()：按网格排列所有缩略图，新增项在网格结尾。
 *  - 添加 emojisChanged() 信号：在添加后触发，供外部保存 JSON。
 * @see SplashIconWidget.h
 * @author 龙夏
 * @date 2025-10-22
 * @version 1.1
 */

#include "SplashIconWidget.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QEasingCurve>
#include <QTimer>
#include <QMimeData>
#include <QDrag>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QMenu>
#include <QContextMenuEvent>
#include <QFont>
/**
 * @brief 构造函数
 * 初始化场景、加载 SVG 图标，并添加阴影与透明背景（注意：为避免 UpdateLayeredWindowIndirect 问题，
 * 我们不使用 QGraphicsDropShadowEffect，也不频繁改变窗口几何）。
 */
SplashIconWidget::SplashIconWidget(QWidget *parent)
    : QWidget(parent)
{
    // 窗口属性：无边框、置顶、透明背景
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    // 禁用 WA_TranslucentBackground（如需透明可小心使用），以规避 layered window 的问题
    setAttribute(Qt::WA_TranslucentBackground, false);

    // 创建 QGraphicsView + Scene + SvgItem
    m_view = new QGraphicsView(this);
    m_scene = new QGraphicsScene(this);
    // svg图标使用前，修改下大小，不然加载后会很大
    m_svgItem = new QGraphicsSvgItem(":/new/prefix1/icons/cat-with-wry-smile-svgrepo-com.svg");

    m_scene->addItem(m_svgItem);
    m_view->setScene(m_scene);
    m_view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    m_view->setStyleSheet("background: transparent;");
    m_view->setFrameShape(QFrame::NoFrame);
    
    // 【关键修复】：禁用滚动条
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // 【关键修复】：设置视图对齐方式为居中
    m_view->setAlignment(Qt::AlignCenter);

    // 设置缩放原点为中心
    m_svgItem->setTransformOriginPoint(m_svgItem->boundingRect().center());

    // 计算合适窗口大小并居中显示
    QRectF svgRect = m_svgItem->boundingRect();
    const int padding = 10;
    int w = static_cast<int>(svgRect.width() + padding);
    int h = static_cast<int>(svgRect.height() + padding);
    
    // 【关键修复】：设置场景矩形，确保场景大小与视图一致
    m_scene->setSceneRect(0, 0, w, h);
    
    resize(w, h);
    m_view->setFixedSize(w, h);
    m_svgItem->setPos(padding / 2.0, padding / 2.0);

    QRect screen = QApplication::desktop()->screenGeometry();
    move(screen.center() - rect().center());

    // 关键：安装事件过滤器到 view 的 viewport()，以便拦截鼠标事件
    // viewport() 是 QGraphicsView 上实际接收鼠标事件的 widget
    m_view->viewport()->installEventFilter(this);
}

/**
 * @brief 使用缩放动画（基于 QGraphicsSvgItem::setScale）
 * 避免 layered window 报错（Qt::WA_TranslucentBackground 场景）
 */
void SplashIconWidget::setScale(qreal s)
{
    m_scaleFactor = s;
    m_svgItem->setScale(s);
}

/**
 * @brief 控制缩放动画，带弹性曲线
 */
void SplashIconWidget::animateScale(qreal start, qreal end, int duration)
{
    QPropertyAnimation *anim = new QPropertyAnimation(this, "scale");
    anim->setStartValue(start);
    anim->setEndValue(end);
    anim->setDuration(duration);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}


/**
 * @brief eventFilter
 * 说明：该函数会捕获安装了 filter 的对象（此处为 m_view->viewport()）的事件。
 * 我们在这里处理按下/移动/释放事件，把它们视作对整个 SplashIconWidget 的拖动操作。
 */
bool SplashIconWidget::eventFilter(QObject *watched, QEvent *event)
{
    // 只处理来自 m_view->viewport() 的鼠标事件
    if (watched == m_view->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                // 记录全局按下点（用于计算 move），以及鼠标相对于窗口左上角的偏移
                m_dragStartGlobal = me->globalPos();
                m_dragOffset = m_dragStartGlobal - frameGeometry().topLeft();
                // 点击动画（非阻塞）
                animateScale(1.0, 0.85, 100);
                QTimer::singleShot(100, [this](){ animateScale(0.85, 1.0, 120); });
                return true; // 我们消费此事件（避免 view 进一步处理）
            }
        }
        else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->buttons() & Qt::LeftButton) {
                QPoint globalPos = me->globalPos();
                QPoint delta = globalPos - m_dragStartGlobal;

                // 如果移动距离小，按拖动窗口移动逻辑
                if (delta.manhattanLength() < QApplication::startDragDistance()) {
                    // 平滑移动：新窗口左上角 = 当前全局鼠标 - 按下时的偏移
                    QPoint newTopLeft = globalPos - m_dragOffset;
                    move(newTopLeft);
                } else {
                    // 超过阈值：仍然保持窗口随鼠标移动（我们不做系统级 QDrag）
                    QPoint newTopLeft = globalPos - m_dragOffset;
                    move(newTopLeft);
                }
                return true; // 事件被处理
            }
        }
//        else if (event->type() == QEvent::MouseButtonRelease) {
//            QMouseEvent *me = static_cast<QMouseEvent*>(event);
//            if (me->button() == Qt::LeftButton) {
//                // 触发 clicked 信号（释放时触发，若你希望只有短点击才触发，可增加阈值判断）
//                emit clicked();
//                return true;
//            }
//        }
    }
    // 其他事件仍由基类处理/传递
    return QWidget::eventFilter(watched, event);
}

/**
 * @brief 以下为窗口自身的 mouse...Event：保留以防 widget 自身也接收到事件
 * 这些函数可保持空实现或仅调用基类，主要拖动交由 eventFilter 处理。
 */
void SplashIconWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}
void SplashIconWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}
void SplashIconWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}

void SplashIconWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 创建表情图标项
        QGraphicsSvgItem *emojiItem = new QGraphicsSvgItem(":/new/prefix1/icons/dog-face-svgrepo-com.svg");
        QRectF svgRect = m_svgItem->boundingRect();
        QPointF center = svgRect.center();
        emojiItem->setPos(center.x() - emojiItem->boundingRect().width() / 2,
                          center.y() - emojiItem->boundingRect().height() / 2);
        emojiItem->setOpacity(0.0);
        m_scene->addItem(emojiItem);

        // 图标收缩动画
        QPropertyAnimation *shrinkAnim = new QPropertyAnimation(this, "scale");
        shrinkAnim->setDuration(300);
        shrinkAnim->setStartValue(1.0);
        shrinkAnim->setEndValue(0.4);
        shrinkAnim->setEasingCurve(QEasingCurve::InCubic);

        // 表情淡入动画
        QPropertyAnimation *emojiFadeIn = new QPropertyAnimation(emojiItem, "opacity");
        emojiFadeIn->setDuration(200);
        emojiFadeIn->setStartValue(0.0);
        emojiFadeIn->setEndValue(1.0);

        // 图标回弹动画
        QPropertyAnimation *expandAnim = new QPropertyAnimation(this, "scale");
        expandAnim->setDuration(300);
        expandAnim->setStartValue(0.4);
        expandAnim->setEndValue(1.0);
        expandAnim->setEasingCurve(QEasingCurve::OutBack);

        // 表情淡出动画
        QPropertyAnimation *emojiFadeOut = new QPropertyAnimation(emojiItem, "opacity");
        emojiFadeOut->setDuration(250);
        emojiFadeOut->setStartValue(1.0);
        emojiFadeOut->setEndValue(0.0);

        // 动画组合
        QSequentialAnimationGroup *group = new QSequentialAnimationGroup(this);
        group->addAnimation(shrinkAnim);

        QParallelAnimationGroup *emojiPhase = new QParallelAnimationGroup;
        emojiPhase->addAnimation(emojiFadeIn);
        emojiPhase->addAnimation(expandAnim);
        group->addAnimation(emojiPhase);

        group->addAnimation(emojiFadeOut);

        // 动画结束后触发 clicked() 信号并移除表情项
        connect(group, &QSequentialAnimationGroup::finished, this, [this, emojiItem]() {
            emit clicked();
            m_scene->removeItem(emojiItem);
            delete emojiItem;
        });

        group->start(QAbstractAnimation::DeleteWhenStopped);
    }

    QWidget::mouseDoubleClickEvent(event);
}

/**
 * @brief 鼠标悬停动画：略缩小然后回弹
 */
void SplashIconWidget::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
    animateScale(1.0, 0.9, 150);
    QTimer::singleShot(150, [this] { animateScale(0.9, 1.0, 150); });
}

/**
 * @brief 鼠标离开动画：略放大然后回弹
 */
void SplashIconWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    animateScale(1.0, 1.1, 150);
    QTimer::singleShot(150, [this] { animateScale(1.1, 1.0, 150); });
}

/**
 * @brief 右键菜单事件 - 显示关闭选项
 * 【新增功能】：参考 EmojiListWidget 的苹果风格菜单样式
 */
void SplashIconWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    
    // 【苹果风格样式】：更圆润、精致、小巧的菜单
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
        "    min-width: 100px;"  // 限制最小宽度
        "}"
        "QMenu::item:selected {"
        "    background-color: rgba(0, 122, 255, 160);"  // 稍降低不透明度
        "    color: white;"
        "}"
        "QMenu::item:pressed {"
        "    background-color: rgba(0, 122, 255, 200);"
        "}"
    );
    
    // 添加关闭菜单项
    QAction *closeAct = menu.addAction(tr("  退出程序"));
    
    // 设置字体
    QFont menuFont("Microsoft YaHei UI", 11);
    closeAct->setFont(menuFont);
    
    // 显示菜单并获取选中的操作
    QAction *chosen = menu.exec(event->globalPos());
    
    if (chosen == closeAct) {
        // 【修改】：关闭图标窗口的同时，终止整个程序
        QApplication::quit();
    }
    
    QWidget::contextMenuEvent(event);
}
