#include "previewdialog.h"
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>

PreviewDialog::PreviewDialog(QWidget *parent)
    : QDialog(parent),
      m_imageLabel(new QLabel(this)),
      m_opacityAnim(new QPropertyAnimation(this))
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(false);  // 【修改】：设置为非模态，允许在预览时点击其他图片

    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setScaledContents(false);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(m_imageLabel);
    lay->setContentsMargins(12,12,12,12);
    setLayout(lay);
    resize(640, 480);

    // 建立淡入动画
    auto *effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    m_opacityAnim->setTargetObject(effect);
    m_opacityAnim->setPropertyName("opacity");
    m_opacityAnim->setDuration(220);
    m_opacityAnim->setStartValue(0.0);
    m_opacityAnim->setEndValue(1.0);
    m_opacityAnim->setEasingCurve(QEasingCurve::InOutCubic);
}

void PreviewDialog::setImage(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        // 【修改】：移除错误提示，在上层处理无效图片
        return;
    }
    QPixmap scaled = pixmap.scaled(size()*0.9, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_imageLabel->setPixmap(scaled);
    startShowAnimation();
    // 【修改】：不在这里调用 show()，由调用者控制
}

void PreviewDialog::startShowAnimation()
{
    if (m_opacityAnim) m_opacityAnim->start();
}

void PreviewDialog::mouseDoubleClickEvent(QMouseEvent * /*event*/)
{
    // UPGRADE: 双击预览框关闭（第二次双击）
    close();
}
