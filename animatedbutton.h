#ifndef ANIMATEDBUTTON_H
#define ANIMATEDBUTTON_H

#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QStyleHints>
#include <QPushButton>
#include <QPainter>
#include <QObject>
#include <QWidget>
#include <QRect>

class AnimatedButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(qreal progress READ progress WRITE setProgress NOTIFY progressChanged)
public:
    explicit AnimatedButton(QWidget *parent = nullptr);
    void toggleState();
    qreal progress() const;
    void setProgress(qreal);
signals:
    void progressChanged(qreal);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    qreal m_progress;
    QPixmap m_playPixmap;
    QPixmap m_pausePixmap;
};

#endif // ANIMATEDBUTTON_H
