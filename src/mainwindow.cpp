#include "mainwindow.h"
#include "previewdialog.h"

#include <QToolBar>
#include <QFileDialog>
#include <QStandardItem>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>
#include <QMessageBox>
#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QInputDialog>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_view(new EmojiListWidget(this)),
      m_model(new QStandardItemModel(this)),
      m_jsonFile(QDir::home().filePath("emoji_data.json")),
      m_previewDialog(nullptr)  // 【新增】：初始化预览对话框指针
{
    DEBUG_LOG("MainWindow constructor started");
    resize(1000, 700);
    setupUI();
    // 【已禁用内部拖动排序】：不再需要 rowsMoved 信号连接
    // connect(m_model, &QStandardItemModel::rowsMoved, this, &MainWindow::onModelRowsMoved);
    loadFromJson();
    DEBUG_LOG("MainWindow initialized successfully");
}

void MainWindow::setupUI()
{
    DEBUG_LOG("Setting up UI components");
    // 中央视图
    setCentralWidget(m_view);
    m_view->setModel(m_model);
    m_view->setItemDelegate(new EmojiListDelegate(this));
    
    // 【禁用内部拖动排序】：支持拖动但不支持在列表内重排
    m_view->setDragDropMode(QAbstractItemView::DragOnly);  // 只允许拖出，不允许内部排序
    DEBUG_LOG("Drag mode set to DragOnly (internal sorting disabled)");

    // 工具栏
    QToolBar *tb = addToolBar("Main");
    QAction *addFilesAct = tb->addAction(tr("添加文件"));
    QAction *addFolderAct = tb->addAction(tr("添加文件夹"));
    QAction *saveAct = tb->addAction(tr("保存"));
    QAction *delSelAct = tb->addAction(tr("删除选中"));
    QAction *refreshAct = tb->addAction(tr("刷新"));

    tb->addSeparator();
    tb->addWidget(new QLabel(tr("排序:")));
    m_sortCombo = new QComboBox(this);
    
    // 【关键修复】：先阻止信号，防止初始化时触发排序
    m_sortCombo->blockSignals(true);
    m_sortCombo->addItems({tr("按添加日期"), tr("按大小"), tr("按名称")});
    m_sortCombo->blockSignals(false);
    
    tb->addWidget(m_sortCombo);
    m_orderCombo = new QComboBox(this);
    
    // 【关键修复】：先阻止信号，防止初始化时触发排序
    m_orderCombo->blockSignals(true);
    m_orderCombo->addItems({tr("降序"), tr("升序")});
    m_orderCombo->blockSignals(false);
    
    tb->addWidget(m_orderCombo);

    tb->addSeparator();
    m_showNamesAct = tb->addAction(tr("显示文件名"));
    m_showNamesAct->setCheckable(true);
    m_showNamesAct->setChecked(false);

    // 连接信号
    connect(addFilesAct, &QAction::triggered, this, &MainWindow::onAddFiles);
    connect(addFolderAct, &QAction::triggered, this, &MainWindow::onAddFolder);
    connect(saveAct, &QAction::triggered, this, &MainWindow::onSave);
    connect(delSelAct, &QAction::triggered, this, &MainWindow::onDeleteSelected);
    connect(refreshAct, &QAction::triggered, this, &MainWindow::loadFromJson);
    connect(m_view, &EmojiListWidget::requestDeleteIndex, this, &MainWindow::onDeleteIndex);
    connect(m_view, &EmojiListWidget::requestPreview, this, &MainWindow::onPreview);
    connect(m_view, &EmojiListWidget::requestRename, this, &MainWindow::onRenameIndex);
    connect(m_view, &EmojiListWidget::requestCopyPath, this, &MainWindow::onCopyPath);
    connect(m_view, &EmojiListWidget::itemClickedIndex, this, &MainWindow::onItemClicked);
    
    // 【关键修复】：在连接信号之前已经阻止了信号触发，所以这里安全连接
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSortCriteriaChanged);
    connect(m_orderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSortOrderChanged);
    connect(m_showNamesAct, &QAction::toggled, this, &MainWindow::onShowNamesToggled);
}

void MainWindow::appendEmojiItem(const QString &path, bool preserveOrder)
{
    QFileInfo fi(path);
    if (!fi.exists()) return;

    QImageReader reader(path);
    reader.setAutoTransform(true);
    QImage img = reader.read();
    QPixmap pix;

    if (img.isNull()) {
        /*
        * 原有问题：
        *   其他类型图片无法加载，导致网格空白。
        * 修改日期：2025-10-21
        * 修改操作：失败时使用占位图 resources/invalid.png
        * 修改后续期达到的效果：网格显示占位图，防止空白。
        */
        pix.load(":/new/prefix1/icons/invalid.png");
        DEBUG_LOG("Invalid pixmap replaced with placeholder:" << path);
    } else {
        pix = QPixmap::fromImage(img);
    }

    EmojiItem item;
    item.filePath = path;
    item.fileName = fi.fileName();
    item.thumbnail = pix.scaled(180,180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    item.createTime = fi.created();
    item.fileSize = fi.size();
    item.orderIndex = preserveOrder ? m_list.size() : m_list.size();
    DEBUG_LOG("Loaded emoji:" << path);

    m_list.append(item);

    QStandardItem *it = new QStandardItem();
    it->setIcon(QIcon(item.thumbnail));
    it->setText(item.fileName);
    it->setData(item.filePath, Qt::UserRole);
    it->setData(m_showNames, Qt::UserRole + 10); // pass showName flag to delegate via model
    it->setEditable(false);
    m_model->appendRow(it);
}

void MainWindow::onAddFiles()
{
    DEBUG_LOG("Opening file selection dialog");
    QStringList files = QFileDialog::getOpenFileNames(this, tr("选择图片文件"), QDir::homePath(),
                                                      tr("图片 (*.png *.jpg *.jpeg *.gif *.webp *.bmp)"));
    if (files.isEmpty()) {
        DEBUG_LOG("No files selected");
        return;
    }
    DEBUG_LOG("Adding" << files.size() << "files");
    for (const QString &f : files) {
        appendEmojiItem(f);
    }
    saveToJson(); // 自动保存变更
    DEBUG_LOG("Files added and saved successfully");
}

void MainWindow::onAddFolder()
{
    DEBUG_LOG("Opening folder selection dialog");
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), QDir::homePath());
    if (dir.isEmpty()) {
        DEBUG_LOG("No folder selected");
        return;
    }
    DEBUG_LOG("Selected folder:" << dir);
    QDir d(dir);
    QStringList filters = {"*.png","*.jpg","*.jpeg","*.gif","*.webp","*.bmp"};
    QFileInfoList list = d.entryInfoList(filters, QDir::Files | QDir::NoSymLinks, QDir::Name);
    DEBUG_LOG("Found" << list.size() << "image files in folder");
    for (const QFileInfo &fi : list) {
        appendEmojiItem(fi.absoluteFilePath());
    }
    saveToJson();
    DEBUG_LOG("Folder contents added and saved successfully");
}

void MainWindow::onSave()
{
    DEBUG_LOG("Manual save triggered");
    saveToJson();
    statusBar()->showMessage(tr("已保存"), 2000);
}

void MainWindow::onDeleteSelected()
{
    QModelIndexList sels = m_view->selectionModel()->selectedIndexes();
    if (sels.isEmpty()) {
        DEBUG_LOG("Delete selected: no items selected");
        return;
    }
    DEBUG_LOG("Deleting" << sels.size() << "selected items");

    int ret = QMessageBox::question(this, tr("删除确认"), tr("是否同时删除磁盘文件？点击否仅从列表移除。"),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel) {
        DEBUG_LOG("Delete cancelled by user");
        return;
    }
    bool deleteDisk = (ret == QMessageBox::Yes);
    DEBUG_LOG("Delete mode:" << (deleteDisk ? "from disk" : "from list only"));

    // collect rows to delete (unique rows)
    QList<int> rows;
    for (const QModelIndex &idx : sels) {
        rows.append(idx.row());
    }
    std::sort(rows.begin(), rows.end(), std::greater<int>()); // delete from bottom upward
    for (int r : rows) {
        QModelIndex idx = m_model->index(r, 0);
        QString path = idx.data(Qt::UserRole).toString();
        if (deleteDisk) QFile::remove(path);
        m_model->removeRow(r);
        // remove from m_list by path
        for (int i=0;i<m_list.size();++i){
            if (m_list[i].filePath == path){ m_list.removeAt(i); break;}
        }
    }
    // UPGRADE: 更新 orderIndex
    for (int i=0;i<m_list.size();++i) m_list[i].orderIndex = i;
    saveToJson();
    DEBUG_LOG("Deleted items, remaining count:" << m_list.size());
}

void MainWindow::onDeleteIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        DEBUG_LOG("Delete index: invalid index");
        return;
    }
    QString path = index.data(Qt::UserRole).toString();
    DEBUG_LOG("Deleting item:" << path);

    int ret = QMessageBox::question(this, tr("删除确认"), tr("是否同时删除磁盘文件？(是=删除磁盘文件，否=仅移除列表)"),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret == QMessageBox::Yes) QFile::remove(path);

    // remove from model and list
    m_model->removeRow(index.row());
    for (int i=0;i<m_list.size();++i){
        if (m_list[i].filePath == path){ m_list.removeAt(i); break;}
    }
    // update indices
    for (int i=0;i<m_list.size();++i) m_list[i].orderIndex = i;
    saveToJson();
}

void MainWindow::onPreview(const QString &path)
{
    DEBUG_LOG("Opening preview for:" << path);
    
    // 【优化】：使用单例预览对话框，支持在预览时点击其他图片自动切换
    if (!m_previewDialog) {
        m_previewDialog = new PreviewDialog(this);
        DEBUG_LOG("Preview dialog created");
    }
    
    QPixmap pix(path);
    if (pix.isNull()) {
        DEBUG_LOG("Cannot load image for preview:" << path);
        // 【重要】：不弹出无效图片的预览，避免用户无法退出的问题
        return;
    }
    
    // 如果对话框已经显示，直接更新图片；否则显示对话框
    m_previewDialog->setImage(pix);
    
    if (!m_previewDialog->isVisible()) {
        m_previewDialog->show();
    }
    m_previewDialog->raise();
    m_previewDialog->activateWindow();
    
    DEBUG_LOG("Preview updated/shown");
}

void MainWindow::onRenameIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        DEBUG_LOG("Rename: invalid index");
        return;
    }
    QString path = index.data(Qt::UserRole).toString();
    QString oldName = index.data(Qt::DisplayRole).toString();
    DEBUG_LOG("Renaming item:" << path << "current name:" << oldName);
    bool ok;
    QString newName = QInputDialog::getText(this, tr("重命名"), tr("显示名（仅列表显示，不重命名文件）:"), QLineEdit::Normal,
                                            oldName, &ok);
    if (!ok) {
        DEBUG_LOG("Rename cancelled");
        return;
    }
    DEBUG_LOG("New name:" << newName);
    // update model display name only
    m_model->setData(index, newName, Qt::DisplayRole);
    // update in m_list
    for (int i=0;i<m_list.size();++i){
        if (m_list[i].filePath == path){ m_list[i].fileName = newName; break;}
    }
    saveToJson();
}

void MainWindow::onCopyPath(const QString &path)
{
    DEBUG_LOG("Copying path to clipboard:" << path);
    QClipboard *clip = QApplication::clipboard();
    clip->setText(path);
    statusBar()->showMessage(tr("路径已复制到剪贴板"), 1500);
}

void MainWindow::onItemClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        DEBUG_LOG("Item clicked: invalid index");
        return;
    }
    QString path = index.data(Qt::UserRole).toString();
    DEBUG_LOG("Item clicked, copying to clipboard:" << path);
    QPixmap pix(path);
    if (pix.isNull()) {
        DEBUG_LOG("Failed to load image:" << path);
        statusBar()->showMessage(tr("无法复制：图片无法打开"), 2000);
        return;
    }
    QClipboard *clip = QApplication::clipboard();
    clip->setPixmap(pix);
    // UPGRADE: 单击直接把图片复制到剪贴板，方便在聊天框粘贴
    statusBar()->showMessage(tr("图片已复制到剪贴板，粘贴到聊天框即可。"), 2000);
    DEBUG_LOG("Image copied to clipboard successfully");
}

void MainWindow::onSortCriteriaChanged(int /*idx*/)
{
    // 【关键修复】：需要区分用户手动拖动排序 vs 工具栏选择排序
    // 
    // 问题现象：即使用户拖动排序成功，只要工具栏排序条件改变就会被重新排序
    // 正确做法：工具栏排序应该有一个"自定义排序"选项，拖动后的排序应保持不变
    // 
    // 暂时方案：在 m_list 中记录排序模式，只有当用户主动改变工具栏排序时才重新排序
    // 当用户从文件夹加载或手动拖动时，应该恢复到"自定义"排序模式
    
    int criteria = m_sortCombo->currentIndex();
    int order = m_orderCombo->currentIndex(); // 0:降序,1:升序
    DEBUG_LOG("Sort criteria changed - criteria:" << criteria << "order:" << order);
    
    // apply sort on m_list, then rebuild model
    if (criteria == 0) { // date
        std::sort(m_list.begin(), m_list.end(), [](const EmojiItem &a, const EmojiItem &b){
            return a.createTime < b.createTime;
        });
    } else if (criteria == 1) { // size
        std::sort(m_list.begin(), m_list.end(), [](const EmojiItem &a, const EmojiItem &b){
            return a.fileSize < b.fileSize;
        });
    } else { // name
        std::sort(m_list.begin(), m_list.end(), [](const EmojiItem &a, const EmojiItem &b){
            return a.fileName.toLower() < b.fileName.toLower();
        });
    }
    if (order == 0) { // 要求降序 -> reverse
        std::reverse(m_list.begin(), m_list.end());
    }
    // 更新 orderIndex
    for (int i=0;i<m_list.size();++i) m_list[i].orderIndex = i;
    rebuildModelFromList();
    saveToJson();
    DEBUG_LOG("List sorted and saved, count:" << m_list.size());
}

void MainWindow::onSortOrderChanged(int /*idx*/)
{
    // Just reuse the same handler
    onSortCriteriaChanged(m_sortCombo->currentIndex());
}

void MainWindow::onShowNamesToggled(bool checked)
{
    DEBUG_LOG("Show names toggled:" << (checked ? "ON" : "OFF"));
    m_showNames = checked;
    // propagate to model: set a role to each item so delegate can read
    for (int r=0; r<m_model->rowCount(); ++r) {
        QModelIndex idx = m_model->index(r,0);
        m_model->setData(idx, m_showNames, Qt::UserRole + 10);
    }
    // request view update
    m_view->viewport()->update();
}

void MainWindow::onModelRowsMoved(const QModelIndex &/*parent*/, int start, int end, const QModelIndex &/*destination*/, int row)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(row);
    
    // 【彻底修复】：不依赖 rowsMoved 的参数（这些参数容易导致理解错误）
    // 【正确做法】：直接根据 model 的当前状态重新同步 m_list
    // 【关键问题】：拖动排序后，需要同步 model 和 m_list，并立即保存到 JSON，防止被后续操作覆盖
    // 
    // 理由：
    // 1. Qt 的 rowsMoved 信号参数在不同场景下含义可能有歧义
    // 2. model 此时已经完成了行的移动，我们直接读取 model 的最新顺序最安全
    // 3. 通过 filePath（唯一标识）在 model 中的顺序来重建 m_list 的顺序
    // 4. 必须同时更新 model 中每个 item 的 orderIndex，确保数据一致
    
    // 根据 model 中的顺序重新排列 m_list
    QList<EmojiItem> orderedList;
    
    for (int r = 0; r < m_model->rowCount(); ++r) {
        QModelIndex idx = m_model->index(r, 0);
        QString path = idx.data(Qt::UserRole).toString();
        
        // 从 m_list 中找到对应的项
        for (const EmojiItem &item : m_list) {
            if (item.filePath == path) {
                EmojiItem copy = item;
                copy.orderIndex = orderedList.size();  // 更新排序索引
                orderedList.append(copy);
                break;
            }
        }
    }
    
    // 【关键】：只有当 orderedList 的大小与 m_list 相同时才进行替换
    // 这确保我们没有丢失任何项
    if (orderedList.size() == m_list.size()) {
        m_list = orderedList;
        
        // 【重要】：同时更新 model 中每个 item 的 orderIndex
        // 这样即使 model 被重建，orderIndex 也能正确保存
        for (int r = 0; r < m_model->rowCount(); ++r) {
            QModelIndex idx = m_model->index(r, 0);
            // 更新 model 中存储的 orderIndex（使用一个自定义 role）
            m_model->setData(idx, orderedList[r].orderIndex, Qt::UserRole + 50);
        }
        
        // 立即保存到 JSON，防止被其他操作覆盖
        saveToJson();
        DEBUG_LOG("Drag sync completed, list size:" << m_list.size());
    } else {
        DEBUG_LOG("WARNING: Size mismatch! orderedList:" << orderedList.size() << "m_list:" << m_list.size());
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();   // 忽略默认关闭
    this->hide();      // 隐藏窗口
    emit windowHidden(); // 可自定义信号
}


void MainWindow::rebuildModelFromList()
{
    DEBUG_LOG("Rebuilding model from list, count:" << m_list.size());
    m_model->clear();
    for (const EmojiItem &item : m_list) {
        QStandardItem *s = new QStandardItem();
        s->setIcon(QIcon(item.thumbnail));
        s->setText(item.fileName);
        s->setData(item.filePath, Qt::UserRole);
        s->setData(m_showNames, Qt::UserRole + 10);
        s->setEditable(false);
        m_model->appendRow(s);
    }
    DEBUG_LOG("Model rebuilt successfully");
}

void MainWindow::saveToJson()
{
    DEBUG_LOG("Saving to JSON:" << m_jsonFile << "item count:" << m_list.size());
    QJsonArray arr;
    for (const EmojiItem &it : m_list) {
        QJsonObject obj;
        obj["path"] = it.filePath;
        obj["name"] = it.fileName;
        obj["time"] = it.createTime.toString(Qt::ISODate);
        obj["size"] = static_cast<double>(it.fileSize);
        obj["order"] = it.orderIndex;
        arr.append(obj);
    }
    QJsonDocument doc(arr);
    QFile f(m_jsonFile);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(doc.toJson());
        f.close();
        DEBUG_LOG("JSON saved successfully");
    } else {
        DEBUG_LOG("Failed to save JSON:" << m_jsonFile);
        QMessageBox::warning(this, tr("保存失败"), tr("无法写入 %1").arg(m_jsonFile));
    }
}

void MainWindow::loadFromJson()
{
    DEBUG_LOG("Loading from JSON:" << m_jsonFile);
    m_list.clear();
    m_model->clear();

    QFile f(m_jsonFile);
    if (!f.exists()) {
        DEBUG_LOG("JSON file does not exist, starting with empty list");
        return;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        DEBUG_LOG("Failed to open JSON file for reading");
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isArray()) {
        DEBUG_LOG("Invalid JSON format, expected array");
        return;
    }

    // Build list, but only include existing files
    QJsonArray arr = doc.array();
    for (const QJsonValue &v : arr) {
        QJsonObject o = v.toObject();
        QString path = o["path"].toString();
        QFileInfo fi(path);
        if (!fi.exists()) continue;
        EmojiItem it;
        it.filePath = path;
        it.fileName = o["name"].toString();
        it.thumbnail = QPixmap(path).scaled(180,180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        it.createTime = QDateTime::fromString(o["time"].toString(), Qt::ISODate);
        it.fileSize = static_cast<qint64>(o["size"].toDouble());
        it.orderIndex = o["order"].toInt();
        m_list.append(it);
    }

    // sort by orderIndex to restore original ordering
    std::sort(m_list.begin(), m_list.end(), [](const EmojiItem &a, const EmojiItem &b){
        return a.orderIndex < b.orderIndex;
    });

    DEBUG_LOG("Loaded" << m_list.size() << "items from JSON");
    rebuildModelFromList();
}

