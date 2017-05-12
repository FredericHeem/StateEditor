#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QtWidgets/QDialog>

#include "ui_AboutDialog.h"

class AboutDialog : public QDialog, private Ui_Dialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = NULL);
    ~AboutDialog(void);
};

#endif