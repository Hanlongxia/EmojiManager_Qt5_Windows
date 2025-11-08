#include "mainwindow.h"
#include <QApplication>
#include "splashiconwidget.h"
/*
* 文件名：main.cpp
* 日期：2025-10-21
* 该文件功能描述：程序入口，先显示可点击的 SVG 启动图标（SplashIconWidget），点击后打开主窗口，关闭主窗口后再次显示启动图标。
* 与该文件相关联的其他文件：splashiconwidget.h/cpp, mainwindow.h/cpp, resources.qrc
*/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 调试宏，可根据需要打开或关闭
#define ENABLE_DEBUG 1
#ifdef ENABLE_DEBUG
    qDebug("Application started");
#endif

    // UPGRADE: 轻量风格接近 macOS（Fusion + 自定义 QPalette 可进一步美化）
    QApplication::setStyle("Fusion");

    // 创建堆对象，保证生命周期长于 lambda
    SplashIconWidget *splash = new SplashIconWidget();
    MainWindow *w = new MainWindow();

    splash->show();

    // 点击启动图标：显示主窗口，隐藏启动图标
    QObject::connect(splash, &SplashIconWidget::clicked, [=](){
        w->show();
        splash->hide();
    });

    QObject::connect(w, &MainWindow::windowHidden, [=](){
        splash->show();
    });

    return a.exec();
}
