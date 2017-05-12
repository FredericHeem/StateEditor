#ifndef STATEEDITOR_MAINWINDOW_H
#define STATEEDITOR_MAINWINDOW_H

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QPointer>
#include <QtCore/QProcess>
#include <QtCore/QFileSystemWatcher>

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>

#include <QWebInspector>

#include "GeneratorInfo.h"
#include "ui_StateEditorMainWindow.h"

class QWebFrame;

class StateEditorMainWindow : public QMainWindow, private Ui::Window
{
    Q_OBJECT

public:
    StateEditorMainWindow();
    ~StateEditorMainWindow();

    Q_INVOKABLE QString getXmlContent();
    Q_INVOKABLE void modelModified();
	Q_INVOKABLE QString indentXml(const QString& content);

    void setFileName(const QString& fileName);
    void setContentFromFile(const QString& fileName);

    void initLog();
    void loadView();
    void loadFile(const QString& fileName);
    void loadFileReadOnly(const QString& fileName);

	bool isFileValid(const QString& fileName, QString &errorMessage);
private slots:

    void appAboutToQuit();

    void viewLoadProgress(int p);
    void viewLoadFinished(bool);
    void viewUrlChanged(const QUrl& url);
    void viewJavaScriptWindowObjectCleared();

    void progressDialogCancel();

    void actionOpenTriggered();
    void actionSaveTriggered();
    void actionSaveAsTriggered();
    void actionGenerateCodeTriggered();
    void actionQuitTriggered();
    void actionAboutTriggered();
    void actionHelpTriggered();
    void actionOpenLogTriggered();
    void actionNewCpp();
    void actionNewCSharp();
    void actionNewJava();
    void actionScreenShootTriggered();

    // QProcess
    void processStarted();
    void processError(QProcess::ProcessError error);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

    // QFileSystemWatcher 
    void fileChanged(const QString &path);

private:
	
	bool askSaveFile();
    QString getJsXmlContent();
    void setXmlContent(const QString& xmlContent);

    void setIsFileToSaved(bool bFileToSaved);
    bool isFileToSaved();

    void render();
    bool writeContentToFile(const QString& fileName);
    void saveFile(const QString& fileName);
    void initFileReadWrite(const QString& fileName);
    void initFileReadOnly(const QString& fileName);
    void initFileCommon(const QString& fileName);
    void setStateMachineFromFileName(const QString& fileName);
    void loadFileCommon(const QString& fileName);

    void fileWatchEnable();
    void fileWatchDisable();

    void updateWindowTitle();
    

    void initApp();
	void initConfig();
    void initAction();
    void initProgressDialog();
    void initWebView();
    void initProxy();
    void initProcess();
    void initGenerator(const QString& fileName);

    QString getLogPath();
    const QString& getStateMachineName() const;
    const QString getStateMachineAbsolutePath() const;
    const QString getScreenShootAbsoluteFilePath() const;

    int getWidth();
    int getOffsetLeft();
    int getHeight();
    int getOffsetTop();

    QUrl m_urlServer;
    int m_iStatusBarMessageTimeout;
    bool m_bFileToSaved;
    QString m_fileName;
    QString m_name;
    QString m_xmlContent;
    QWebFrame *m_pWebFrame;
    QWebInspector m_webInspector;
    QProgressDialog *m_pProgessDialog;
    QProcess m_generatorProcess;
    QPointer<GeneratorInfo> m_runner;
    QString m_logFile;
    QString m_logDir;
    QFileSystemWatcher m_fileWatcher;

};

#endif // STATEEDITOR_MAINWINDOW_H
