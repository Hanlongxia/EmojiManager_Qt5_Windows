/*
* 文件名：previewdialog.h
* 日期：2025-10-21
* 该文件功能大致描述：预览对话框类，支持淡入动画、双击关闭、非模态显示，允许连续预览多张图片。
* 该文件函数功能描述：
*   - PreviewDialog()：构造函数，初始化非模态、无边框、半透明对话框
*   - setImage()：设置并显示预览图片，带淡入动画效果
*   - mouseDoubleClickEvent()：双击关闭预览对话框
*   - startShowAnimation()：启动透明度淡入动画
* 与该文件相关联的其他文件：previewdialog.cpp, mainwindow.h, mainwindow.cpp
*/

#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPixmap>

class PreviewDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreviewDialog(QWidget *parent = nullptr);
    // 显示图片，如果已经显示则切换行为由调用者控制（这里实现双击关闭）
    void setImage(const QPixmap &pixmap);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override; // 双击关闭

private:
    QLabel *m_imageLabel;
    QPropertyAnimation *m_opacityAnim;
    void startShowAnimation();
};

#endif // PREVIEWDIALOG_H
