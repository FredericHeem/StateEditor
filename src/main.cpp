#include <QtCore/QDebug>
#include <QsLog.h>
#include "StateEditorDefs.h"
#include "StateEditorMainWindow.h"
#include <assert.h>

void unitest(StateEditorMainWindow &stateEditorMainWindow)
{
	assert(stateEditorMainWindow.isFileValid("C:/Users/frederic/0.fsmcpp", error) == false);
	assert(stateEditorMainWindow.isFileValid("0.fsmcpp", error) == false);
	assert(stateEditorMainWindow.isFileValid("000aaa.fsmcpp", error) == false);
	assert(stateEditorMainWindow.isFileValid(" aaa.fsmcpp", error) == false);
	assert(stateEditorMainWindow.isFileValid("a aa.fsmcpp", error) == false);
	assert(stateEditorMainWindow.isFileValid("a  aa.fsmcpp", error) == false);
	assert(stateEditorMainWindow.isFileValid("a .fsmcpp", error) == false);

	assert(stateEditorMainWindow.isFileValid("aaaa.fsmcpp", error) == true);
	assert(stateEditorMainWindow.isFileValid("a0.fsmcpp", error) == true);
	assert(stateEditorMainWindow.isFileValid("a0aa.fsmcpp", error) == true);
}

int main(int argc, char** argv)
{
   
    QApplication application(argc, argv);
    application.setApplicationName(APP_NAME);
    application.setOrganizationName(APP_COMPANY);
    QStringList argList = application.arguments();

    StateEditorMainWindow stateEditorMainWindow;
	QString error;

	unitest(stateEditorMainWindow);

    stateEditorMainWindow.show();

    if(argList.count() == 2){
        stateEditorMainWindow.loadFile(argList.at(1));
    } else {
        stateEditorMainWindow.loadView();
    }

    return application.exec();
}

