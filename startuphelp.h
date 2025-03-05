#ifndef STARTUPHELP_H
#define STARTUPHELP_H

#include <QCoreApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QDialog>
#include <QWidget>

class StartupHelp : public QDialog
{
public:
    explicit StartupHelp(QWidget *parent = nullptr);
};

#endif // STARTUPHELP_H
