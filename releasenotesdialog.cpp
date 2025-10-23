#include "releasenotesdialog.h"

#include <QRegularExpression>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

ReleaseNotesDialog::ReleaseNotesDialog(const QString &version, const QString &releaseNotesUrl, QWidget *parent)
    : QDialog(parent),
    m_networkManager(new QNetworkAccessManager(this))
{
    setWindowTitle(QString("PhoneAudioLink %1 - Release Notes").arg(version));
    setMinimumSize(600, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Text browser for release notes
    m_textBrowser = new QTextBrowser(this);
    m_textBrowser->setOpenExternalLinks(true);
    m_textBrowser->setHtml("<p><i>Loading release notes...</i></p>");

    mainLayout->addWidget(m_textBrowser);

    // Button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_updateButton = new QPushButton("Update Now", this);
    m_updateButton->setDefault(true);

    m_closeButton = new QPushButton("Close", this);

    buttonLayout->addWidget(m_updateButton);
    buttonLayout->addWidget(m_closeButton);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_updateButton, &QPushButton::clicked, this, [this]() {
        emit updateRequested();
        accept();
    });
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);

    // Download release notes
    downloadReleaseNotes(releaseNotesUrl);
}

void ReleaseNotesDialog::downloadReleaseNotes(const QString &url)
{
    qDebug() << "Downloading release notes from:" << url;

    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ReleaseNotesDialog::onReleaseNotesDownloaded);
}

void ReleaseNotesDialog::onReleaseNotesDownloaded()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }


    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Failed to download release notes:" << reply->errorString();
        m_textBrowser->setHtml("<p><b>Failed to load release notes.</b></p>"
                               "<p>Error: " + reply->errorString() + "</p>");
        return;
    }

    QString markdown = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    m_textBrowser->setMarkdown(markdown);
}
