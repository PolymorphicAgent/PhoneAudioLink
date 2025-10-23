#ifndef RELEASENOTESDIALOG_H
#define RELEASENOTESDIALOG_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialog>
#include <QObject>

class ReleaseNotesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReleaseNotesDialog(const QString &version, const QString &releaseNotesUrl, QWidget *parent = nullptr);

signals:
    void updateRequested();

private slots:
    void onReleaseNotesDownloaded();

private:
    QTextBrowser *m_textBrowser;
    QPushButton *m_updateButton;
    QPushButton *m_closeButton;
    QNetworkAccessManager *m_networkManager;

    void downloadReleaseNotes(const QString &url);
};

#endif // RELEASENOTESDIALOG_H
