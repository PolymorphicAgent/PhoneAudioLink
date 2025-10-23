#include "updatenotificationbar.h"

#include <QGuiApplication>
#include <QStyleHints>
#include <QStyle>

UpdateNotificationBar::UpdateNotificationBar(QWidget *parent)
    : QWidget(parent)
{
    // Set background color similar to Qt Creator's notification bar
    if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light){
        setStyleSheet("UpdateNotificationBar { "
                      "background-color: #e8f4fc; "
                      "border-top: 1px solid #87ceeb; "
                      "}");

        // Message label
        m_messageLabel = new QLabel(this);
        m_messageLabel->setStyleSheet("QLabel { color: #000; }");

        // See details button
        m_detailsButton = new QPushButton("See Details", this);
        m_detailsButton->setFlat(true);
        m_detailsButton->setStyleSheet("QPushButton { color: #000; text-decoration: underline; border: none; }"
                                       "QPushButton:hover { color: #8c8c8c; }");
        m_detailsButton->setCursor(Qt::PointingHandCursor);
    }
    else{
        setStyleSheet("UpdateNotificationBar { "
                      "background-color: #3c3c3c; "
                      "border-top: 1px solid #5b5b5b; "
                      "}");

        // Message label
        m_messageLabel = new QLabel(this);
        m_messageLabel->setStyleSheet("QLabel { color: #fff; }");

        // See details button
        m_detailsButton = new QPushButton("See Details", this);
        m_detailsButton->setFlat(true);
        m_detailsButton->setStyleSheet("QPushButton { color: #8c8c8c; text-decoration: underline; border: none; }"
                                       "QPushButton:hover { color: #393939; }");
        m_detailsButton->setCursor(Qt::PointingHandCursor);
    }

    setFixedHeight(40);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);

    // Info icon
    m_iconLabel = new QLabel(this);
    QIcon infoIcon = QIcon::fromTheme(QIcon::ThemeIcon::DialogInformation);
    m_iconLabel->setPixmap(infoIcon.pixmap(20, 20));

    // Update button
    m_updateButton = new QPushButton("Update", this);

    // Close button
    m_closeButton = new QPushButton("✕", this);
    m_closeButton->setFlat(true);
    m_closeButton->setFixedSize(20, 20);
    m_closeButton->setStyleSheet("QPushButton { border: none; color: #666; font-weight: bold; }"
                                 "QPushButton:hover { color: #000; }");
    m_closeButton->setCursor(Qt::PointingHandCursor);

    // Add widgets to layout
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_messageLabel);
    layout->addWidget(m_detailsButton);
    layout->addWidget(m_updateButton);
    layout->addStretch();
    layout->addWidget(m_closeButton);

    // Connect signals
    connect(m_detailsButton, &QPushButton::clicked, this, [this]() {
        emit seeDetailsClicked(m_releaseNotesUrl);
    });
    connect(m_updateButton, &QPushButton::clicked, this, &UpdateNotificationBar::updateClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &UpdateNotificationBar::closeClicked);

    // Initially hidden
    hide();
}

void UpdateNotificationBar::showUpdate(const QString &version, const QString &releaseNotesUrl)
{
    m_releaseNotesUrl = releaseNotesUrl;
    m_messageLabel->setText(QString("A new version (%1) is available! Start?").arg(version));
    show();
}

void UpdateNotificationBar::hideBar()
{
    hide();
}
