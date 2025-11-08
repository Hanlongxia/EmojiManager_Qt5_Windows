/*
* 文件名：splashiconwidget.h
* 日期：2025-10-22
* 该文件功能大致描述：悬浮启动图标组件，支持鼠标拖动、缩放动画、点击信号以及主窗口集成，用于在程序启动时显示带动画效果的悬浮SVG图标。
* 该文件函数功能描述：
*   - SplashIconWidget()：构造函数，初始化无边框、置顶显示的圆形/方形图标窗口
*   - scale()/setScale()：获取/设置当前缩放比例，用于动画效果
*   - enterEvent()/leaveEvent()：鼠标进入/离开事件，触发缩放动画
*   - mousePressEvent()/mouseReleaseEvent()/mouseMoveEvent()：处理鼠标事件，实现拖动和点击功能
*   - mouseDoubleClickEvent()：双击事件处理
*   - contextMenuEvent()：右键菜单事件，提供“退出程序”菜单项
*   - eventFilter()：事件过滤器，捕获viewport()鼠标事件，实现平滑拖动
*   - animateScale()：执行缩放动画，支持自定义起始/结束值和时长
* 与该文件相关联的其他文件：splashiconwidget.cpp, main.cpp
*/

#ifndef SPLASHICONWIDGET_H
#define SPLASHICONWIDGET_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QPoint>

class SplashIconWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    explicit SplashIconWidget(QWidget *parent = nullptr);

    /**
     * @brief 当前缩放比例
     */
    qreal scale() const { return m_scaleFactor; }

    /**
     * @brief 设置缩放比例
     * @param s 新的缩放系数
     */
    void setScale(qreal s);

signals:
    /**
     * @brief 图标被点击时发出
     */
    void clicked();

protected:
    // 鼠标交互事件
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;       // 仍保留以防 widget 本身也收到事件
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;  // 【新增】：右键菜单事件

    // 新增：对 m_view->viewport() 安装事件过滤器时，会调用这里
    bool eventFilter(QObject *watched, QEvent *event) override;

private:    
    /**
     * @brief 执行缩放动画
     * @param start 起始缩放值
     * @param end   结束缩放值
     * @param duration 动画时长（毫秒）
     */
    void animateScale(qreal start, qreal end, int duration = 150);

    QGraphicsView *m_view;                // 图形视图容器
    QGraphicsScene *m_scene;              // 场景
    QGraphicsSvgItem *m_svgItem;          // 主 SVG 图标（启动图标）
    QList<QGraphicsPixmapItem*> m_emojiItems; // 缩略图项列表（导入的表情图片）
    QList<QString> m_emojiPaths;          // 对应的文件路径（保持与 m_emojiItems 一致）
    QPoint m_dragStartGlobal;             // 拖动起点 — 全局坐标（用于 window move）
    QPoint m_dragOffset;                  // 拖动起点 — 鼠标相对于窗口左上角的偏移
    qreal m_scaleFactor = 1.0;            // 当前缩放因子
    int m_thumbSize = 120;                // 缩略图大小（像素）

};

#endif // SPLASHICONWIDGET_H
