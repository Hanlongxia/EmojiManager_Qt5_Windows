# Emoji Manager Enhanced (Qt5)

## 新增/增强功能 一 版（相较基础版本）
- 网格列表（QListView + QStyledItemDelegate）显示缩略图。
- 单击复制图片到剪贴板，方便粘贴到聊天框。
- 双击打开预览（再次双击可关闭）。
- 拖动支持内部重排序（并持久化 orderIndex 到 JSON）。
- 右键菜单支持：预览 / 重命名 / 复制路径 / 删除（可选择是否删除磁盘文件）。
- 导入支持文件与文件夹批量导入。
- 工具栏支持：添加文件 / 添加文件夹 / 保存 / 删除选中 / 刷新 / 排序依据 + 升/降 / 显示文件名切换。
- JSON 存储包含：path, name, time, size, order。
- EmojiManager.exe可执行程序下，添加了imageformats（各类图片格式支持的dll）、platforms（可执行支持的dll）

## 构建
1. 在 Qt Creator 中打开 `EmojiManager.pro`。
2. 构建并运行（Qt5.x，已在 Windows 下使用 MinGW 测试构建流程）。

## 文件说明
参见 `src/` 中的 `.h` 文件顶部的模块注释。

## 新增/增强功能 二 版（相较一版）
- 当前运行后直接就是操作界面，增加一步操作，运行先显示一个圆形图标（该圆形图标可添加动图，可点击，可拖动，可拖动到桌面），点击图标再进入操作界面。
- 内部图片拖动后，自动重新排序、刷新。
- 程序启动时的圆形 SVG 图标，支持鼠标悬停弹跳动画、点击压下动画、拖动到桌面
- 圆形 SVG 图标，增加“原地缩放呼吸”式动画

## 文件修改说明 2025.10.22
- `src/splashiconwidget.cpp`
    
    1、说明（为何新增 eventFilter / 两个拖动点）：

    m_view->viewport() 是 QGraphicsView 的实际鼠标事件接收者。我们安装 eventFilter，捕获 QMouseEvent 并把拖动逻辑放在 widget 层，这样不会再被 view 拦截导致无法移动。

    两个拖动点：

    m_dragStartGlobal：记录按下时的 全局位置（globalPos），用于判断拖动阈值和计算位移。

    m_dragOffset：记录按下时鼠标相对于窗口左上角的偏移（frameGeometry().topLeft），用于 move(newTopLeft) 计算，保证窗口不会跳到鼠标左上角。

    2、为什么这样能解决问题（技术解释）

    QGraphicsView 的 viewport() 才是真正接收鼠标事件的 widget。直接重载 SplashIconWidget::mouseMoveEvent 无法保证捕获这些事件。安装 eventFilter 在 viewport() 上可以确保事件能被捕获并转交你的拖动逻辑。

    我把拖动逻辑都使用 全局坐标（globalPos()）和按下时与窗口左上角的偏移来计算新窗口位置，避免坐标系混淆。

    我不再使用 QDrag.exec()（即不做系统级拖放），因为那在半透明或 Layered 窗口上常导致 Windows API 报错。改为窗口自身随鼠标移动的行为，既稳定又符合“悬浮图标”的交互预期。

    避免使用 QGraphicsDropShadowEffect 或 WA_TranslucentBackground 的不当组合（你之前已按要求去掉或禁用），从根源避免 UpdateLayeredWindowIndirect 报错。

## 新增/增强功能 三 版（相较二版）
- 添加了图标点击，会先弹出一个新的淡入、淡出svg图标，新图标放在了`src/icon/`目录下，然后再显示操作界面。

## 功能优化与BUG修复 2025.10.30

### 1. 拖动排序问题修复
- **问题描述**：拖动A图片到B图片位置时，A图片会消失；从文件夹加载的图片总是按文件夹顺序排序，无法通过拖动重排
- **解决方案**：
  - 禁用列表内部拖动排序功能（`setDragDropMode(QAbstractItemView::DragOnly)`）
  - 只允许拖出到外部，不允许在列表内部重排
  - 注释掉 rowsMoved 信号连接，避免触发排序逻辑
- **修改文件**：`src/mainwindow.cpp` 的 `setupUI()` 函数和构造函数

### 2. 初始化排序条件问题修复
- **问题描述**：程序启动时排序下拉框会触发排序，覆盖用户之前的拖动排序
- **解决方案**：在 `addItems()` 前后使用 `blockSignals(true/false)` 阻止信号触发
- **修改文件**：`src/mainwindow.cpp` 的 `setupUI()` 函数

### 3. 添加调试日志
- **功能说明**：在所有重要操作中添加 DEBUG_LOG 语句，方便后续调试
- **涉及位置**：
  - `src/mainwindow.cpp`：加载JSON、保存JSON、导入文件、删除、排序等操作
  - `src/emojilistwidget.cpp`：鼠标事件、右键菜单、拖动操作等

### 4. 右键菜单UI优化（仿Apple iOS风格）
- **优化目标**：圆润柔和的视觉效果，符合iOS简约美学
- **具体改进**：
  - 移除模糊的emoji图标（👁 ✏️ 📋 🗑️），使用纯文字+空格对齐
  - 缩小字体：从 13pt 缩小为 11pt
  - 优化边框：从 1px 减为 0.5px，更细腻
  - 缩小圆角：菜单从 12px 改为 10px，菜单项从 6px 改为 5px
  - 缩小内外边距：整体菜单更紧凑精致
  - 优化分隔线：从 1px 减为 0.5px，颜色更淡
  - 添加最小宽度限制（120px），避免菜单过宽
  - 删除项使用 DemiBold 字重而非 Bold，更柔和
- **修改文件**：`src/emojilistwidget.cpp` 的 `contextMenuEvent()` 函数

### 5. 样式表优化细节
```cpp
// 菜单主体样式
background-color: rgba(255, 255, 255, 250);  // 更高透明度
border: 0.5px solid rgba(220, 220, 220, 150);  // 更细边框
border-radius: 10px;  // 适中圆角
padding: 6px 0px;  // 紧凑内边距

// 菜单项样式
padding: 8px 20px 8px 20px;  // 舒适的项内边距
margin: 1px 6px;  // 精致外边距
border-radius: 5px;  // 柔和圆角

// 选中/按下效果
selected: rgba(0, 122, 255, 160);  // iOS蓝色
pressed: rgba(0, 122, 255, 200);  // 更深蓝色

// 分隔线
height: 0.5px;  // 极细分隔线
background: rgba(210, 210, 210, 100);  // 淡色
```

## 功能优化与交互增强 2025.10.30 (第二次)

### 1. 无效图片双击保护
- **问题描述**：无法加载的图片双击后会弹出"无法加载图片"提示,且该提示框可能点击不到正确位置,导致无法退出放大状态
- **解决方案**：
  - 在 [`EmojiListWidget`](f:\QT-3\EmojiManager_Qt5_Windows\src\emojilistwidget.cpp) 的 `mouseDoubleClickEvent` 中增加图片加载检测
  - 对于无法加载的图片,直接阻止双击事件,不弹出预览对话框
  - 避免用户陷入无法交互的状态
- **修改文件**：`src/emojilistwidget.cpp` 的 `mouseDoubleClickEvent()` 函数

### 2. 预览对话框交互优化
- **问题描述**：当前预览对话框是模态的,无法在查看一张图片时切换到其他图片
- **解决方案**：
  - 将 [`PreviewDialog`](f:\QT-3\EmojiManager_Qt5_Windows\src\previewdialog.cpp) 改为非模态对话框（`setModal(false)`）
  - 在 [`MainWindow`](f:\QT-3\EmojiManager_Qt5_Windows\src\mainwindow.cpp) 中使用单例预览对话框（`m_previewDialog`）
  - 点击其他图片时,自动更新预览内容而不是关闭再重新打开
  - 支持在预览状态下点击列表中的其他图片,实现快速浏览
- **技术细节**：
  - [`PreviewDialog::setImage()`](f:\QT-3\EmojiManager_Qt5_Windows\src\previewdialog.cpp) 只更新图片内容,不控制显示
  - [`MainWindow::onPreview()`](f:\QT-3\EmojiManager_Qt5_Windows\src\mainwindow.cpp) 负责控制对话框的显示、激活和置顶
  - 对无效图片提前返回,避免显示空白预览
- **修改文件**：
  - `src/mainwindow.h`：添加 `m_previewDialog` 成员变量
  - `src/mainwindow.cpp`：修改 `onPreview()` 函数和构造函数
  - `src/previewdialog.cpp`：修改 `PreviewDialog()` 构造函数和 `setImage()` 函数

### 3. 用户体验提升
- **优化效果**：
  - 防止无效图片导致的交互死锁
  - 支持连续快速预览多张图片
  - 预览窗口保持打开状态,减少开关动画的视觉干扰
  - 符合现代图片查看器的交互习惯

## SVG启动图标功能增强 2025.10.30

### 1. SVG图标滚动条问题修复
- **问题描述**：cat-with-wry-smile SVG图标显示时出现滚动条，拖动和双击时窗体会显示滚动条，图标似乎变小了
- **问题原因**：
  - QGraphicsView 默认启用了滚动条策略
  - 场景矩形（SceneRect）未明确设置，导致场景大小与视图大小不匹配
  - 视图对齐方式未设置
- **解决方案**：
  - 禁用水平和垂直滚动条：`setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff)` 和 `setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff)`
  - 设置视图居中对齐：`setAlignment(Qt::AlignCenter)`
  - 明确设置场景矩形：`setSceneRect(0, 0, w, h)`，确保场景大小与视图窗口完全一致
- **修改文件**：`src/splashiconwidget.cpp` 的构造函数
- **优化效果**：
  - SVG 图标完美填充窗口，不再出现滚动条
  - 拖动和双击时窗体保持稳定
  - SVG 图标始终居中显示，视觉效果更加精致

### 2. 右键菜单功能添加
- **新增功能**：为 SVG 启动图标添加右键菜单，支持退出程序操作
- **实现方案**：
  - 在 `splashiconwidget.h` 中添加 `contextMenuEvent()` 函数声明
  - 在 `splashiconwidget.cpp` 中实现右键菜单功能
  - 采用与项目其他组件一致的**苹果风格设计**
- **菜单样式**：
  - 半透明白色背景 `rgba(255, 255, 255, 250)`
  - 细边框 `0.5px` 柔和圆角 `10px`
  - iOS 蓝色选中效果 `rgba(0, 122, 255, 160)`
  - 紧凑精致的内边距和字体（微软雅黑 11pt）
- **菜单项功能**：
  - **"退出程序"**：调用 `QApplication::quit()` 优雅终止整个应用程序
- **修改文件**：
  - `src/splashiconwidget.h`：添加 `contextMenuEvent()` 声明
  - `src/splashiconwidget.cpp`：添加头文件引用和右键菜单实现
- **用户体验**：
  - 右键点击 SVG 启动图标即可弹出精致菜单
  - 选择"退出程序"优雅关闭应用
  - 菜单样式符合项目整体苹果风格设计