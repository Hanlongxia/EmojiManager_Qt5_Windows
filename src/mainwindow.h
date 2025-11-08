/*
* 文件名：mainwindow.h
* 日期：2025-10-21
* 该文件功能大致描述：主窗口，负责 UI 布局、工具栏（导入/保存/删除/刷新/设置）、排序逻辑、JSON 持久化，以及连接列表视图发出的信号进行具体操作。
* 该文件函数功能描述：
*   - setupUI()：初始化主窗口UI，包括工具栏、列表视图、委托等
*   - loadFromJson()/saveToJson()：JSON文件读写，持久化表情数据
*   - onAddFiles()/onAddFolder()：批量导入文件和文件夹
*   - onDeleteSelected()/onDeleteIndex()：删除选中的表情项
*   - onPreview()：预览图片，支持单例非模态对话框，允许连续查看多张图片
*   - onRenameIndex()：重命名表情项
*   - onCopyPath()：复制文件路径到剪贴板
*   - onItemClicked()：单击复制图片到剪贴板
*   - onSortCriteriaChanged()/onSortOrderChanged()：处理排序逻辑
*   - onShowNamesToggled()：切换文件名显示/隐藏
*   - onModelRowsMoved()：处理拖放排序后的数据同步
*   - rebuildModelFromList()：根据内存数据重建模型
* 与该文件相关联的其他文件：mainwindow.cpp, emojilistwidget.h, emojilistwidget.cpp, emojilistdelegate.h, emojilistdelegate.cpp, previewdialog.h, previewdialog.cpp, emoji_meta.h
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QStandardItemModel>
#include <QComboBox>
#include <QAction>
#include "emojilistwidget.h"
#include "emojilistdelegate.h"
#include "emoji_meta.h"
#include <QStatusBar>
#include <QMessageBox>  // 如果需要使用其他Qt类
#include <QApplication>
#include <QClipboard>
#include <QImageReader>

class PreviewDialog;  // 前置声明

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onAddFiles();
    void onAddFolder(); // UPGRADE: 批量导入文件夹
    void onSave();
    void onDeleteSelected();
    void onDeleteIndex(const QModelIndex &index);
    void onPreview(const QString &path);
    void onRenameIndex(const QModelIndex &index);
    void onCopyPath(const QString &path);
    void onItemClicked(const QModelIndex &index); // UPGRADE: 单击复制图片到剪贴板
    void onSortCriteriaChanged(int idx);
    void onSortOrderChanged(int idx);
    void onShowNamesToggled(bool checked);
    void onModelRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);

    void closeEvent(QCloseEvent *event);
private:
    void setupUI();
    void loadFromJson();
    void saveToJson();
    void appendEmojiItem(const QString &path, bool preserveOrder=false);
    void rebuildModelFromList();
    EmojiListWidget *m_view;
    QStandardItemModel *m_model;
    QList<EmojiItem> m_list;
    QString m_jsonFile;
    // UI controls
    QComboBox *m_sortCombo;     // 排序依据
    QComboBox *m_orderCombo;    // 升序/降序
    QAction *m_showNamesAct;    // 切换显示文件名
    bool m_showNames = false;   // 默认隐藏文件名
    
    PreviewDialog *m_previewDialog = nullptr;  // 【新增】：保持单例预览对话框

signals:
    void windowHidden();
};

#endif // MAINWINDOW_H
