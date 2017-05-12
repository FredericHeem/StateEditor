#include "AboutDialog.h"
#include "StateEditorDefs.h"

AboutDialog::AboutDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    const QString description = tr(
        "<h3>StateEditor %1</h3>"
        "Based on Qt %2 (%3 bit)<br/>"
        "<br/>"
        "Built on %4 at %5<br />"
        "<br/>"
        "Copyright 2011-%6 %7. All rights reserved.<br/>"
        "<br/>")
        .arg(QLatin1String(APP_VERSION), 
        QLatin1String(QT_VERSION_STR), QString::number(QSysInfo::WordSize),
        QLatin1String(__DATE__), QLatin1String(__TIME__), QLatin1String(APP_YEAR),
        (QLatin1String(APP_AUTHOR)));

    labelDescription->setText(description);

}

AboutDialog::~AboutDialog(void)
{
}

#include "AboutDialog.moc"