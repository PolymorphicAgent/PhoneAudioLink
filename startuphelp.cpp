#include "startuphelp.h"

StartupHelp::StartupHelp(QWidget *parent)
    : QDialog(parent)
{
    //set the dialog title and icon.
    setWindowTitle("Startup Instructions");
    setMinimumSize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);

    // Retrieve the executable path
    QString exePath = QCoreApplication::applicationFilePath();

    // Instructions with enlarged font and highlightable text
    QString instructions =
        "<p style='font-size:16px; font-weight:bold;'>Follow these steps to add a shortcut to the Startup folder:</p>"
        "<p style='font-size:14px;'>"
        "1. Press <b>Win + R</b> to open the Run dialog.<br>"
        "2. Type <i>shell:startup</i> and press Enter.<br>"
        "3. In the folder that opens, right-click and select <i>New → Shortcut</i>.<br>"
        "4. In the shortcut wizard, browse to or copy/paste the full path to the program's main executable:"
        "<p><code>" + exePath + "</code></p>"
        "<p style='font-size:14px;'>"
        "5. Click <i>Next</i>, then give the shortcut a name and click <i>Finish</i>.<br><br>"
        "Your Phone Audio Link will now start automatically when you log in."
        "</p>";

    // Read-only text box for selectable instructions
    QTextEdit *textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    textEdit->setHtml(instructions);
    textEdit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    layout->addWidget(textEdit);

    // OK button to close the dialog
    QPushButton *okButton = new QPushButton("OK", this);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(okButton);
}

