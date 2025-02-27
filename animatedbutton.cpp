#include "animatedbutton.h"

AnimatedButton::AnimatedButton(QWidget *parent)
    : QPushButton(parent), m_progress(0.0)
{
    if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light) {
        m_playPixmap = QPixmap(":/icons/Iconparts/play-512.png");
        m_pausePixmap = QPixmap(":/icons/Iconparts/pause-512.png");
    }
    else if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
        m_playPixmap = QPixmap(":/icons/Iconparts/play-512-white.png");
        m_pausePixmap = QPixmap(":/icons/Iconparts/pause-512-white.png");
    }
}

qreal AnimatedButton::progress() const { return m_progress; }

void AnimatedButton::setProgress(qreal p) {
    m_progress = p;
    update();  // repaint with the new progress value
    emit progressChanged(p);
}

void AnimatedButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    QRect iconRect = rect(); // Adjust as needed for your layout

    // Draw play icon (fades out)
    painter.setOpacity(1 - m_progress);
    painter.drawPixmap(iconRect, m_playPixmap);

    // Draw pause icon (fades in)
    painter.setOpacity(m_progress);
    painter.drawPixmap(iconRect, m_pausePixmap);
}

void AnimatedButton::toggleState() {
    QPropertyAnimation *anim = new QPropertyAnimation(this, "progress");
    anim->setDuration(200); // Duration in milliseconds
    // Assume m_progress==0 for play and 1 for pause state
    if (m_progress == 0) {
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
    } else {
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);
    }
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
