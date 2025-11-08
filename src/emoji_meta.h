/*
* 文件名：emoji_meta.h
* 日期：2025-10-21
* 该文件功能大致描述：定义 EmojiItem 数据结构，用于在内存中保存表情项的元数据与排序信息；提供全局调试宏定义。
* 该文件函数功能描述：
*   - EmojiItem 结构体：包含文件路径、文件名、缩略图、创建时间、文件大小、排序索引等字段
*   - DEBUG_LOG 宏：用于全局调试输出，可通过 ENABLE_DEBUG 开关控制
* 与该文件相关联的其他文件：mainwindow.cpp, mainwindow.h, emojilistwidget.cpp, emojilistwidget.h, emojilistdelegate.cpp, emojilistdelegate.h
*/

#ifndef EMOJI_ITEM_H
#define EMOJI_ITEM_H

#include <QString>
#include <QPixmap>
#include <QDateTime>

#define ENABLE_DEBUG 1
#if ENABLE_DEBUG
    #include <QDebug>
    #define DEBUG_LOG(x) qDebug() << "[DEBUG]" << x
#else
    #define DEBUG_LOG(x)
#endif


struct EmojiItem {
    QString filePath;
    QString fileName;
    QPixmap thumbnail;    // 缩略图（可缓存）
    QDateTime createTime;
    qint64 fileSize;
    int orderIndex;       // UPGRADE: 持久化排序索引
};

#endif // EMOJI_ITEM_H
