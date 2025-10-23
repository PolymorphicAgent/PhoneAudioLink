#ifndef UPDATENOTIFICATIONBAR_H
#define UPDATENOTIFICATIONBAR_H

#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>

class UpdateNotificationBar : public QWidget
{
    Q_OBJECT
public:
    explicit UpdateNotificationBar(QWidget *parent = nullptr);

    void showUpdate(const QString &version, const QString &releaseNotesUrl);
    void hideBar();

signals:
    void seeDetailsClicked(const QString &releaseNotesUrl);
    void updateClicked();
    void closeClicked();

private:
    QLabel *m_iconLabel;
    QLabel *m_messageLabel;
    QPushButton *m_detailsButton;
    QPushButton *m_updateButton;
    QPushButton *m_closeButton;

    QString m_releaseNotesUrl;
};

#endif // UPDATENOTIFICATIONBAR_H
