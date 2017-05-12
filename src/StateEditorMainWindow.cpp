#include <QtCore/QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QPoint>
#include <QtCore/QStandardPaths>
#include <QtCore/QSettings>
#include <QtCore/QRegExp>

#include <QtGui/QDesktopServices>
#include <QtGui/QPixmap>
#include <QtGui/QRegion>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include <QtWebKitWidgets/QWebFrame>
#include <QWebSettings>

#include <QtNetwork/QNetworkProxyQuery>
#include <QtNetwork/QNetworkProxyFactory>

#include <QXmlQuery>

#include <QsLog.h>
#include <QsLogDest.h>

#include "StateEditorMainWindow.h"
#include "AboutDialog.h"
#include "StateEditorDefs.h"

StateEditorMainWindow::StateEditorMainWindow() 
:m_urlServer(STATE_EDITOR_URL)
,m_iStatusBarMessageTimeout(5000)
,m_bFileToSaved(false)
,m_logFile(APP_LOGFILE)
{
    initLog();

    QLOG_INFO() << APP_NAME << " version " << APP_VERSION << "by " << APP_COMPANY;
    QLOG_DEBUG() << QApplication::instance()->arguments().join(" ");

	initConfig();

    setupUi(this);
    setWindowState(Qt::WindowMaximized);

    initApp();
    initProxy();
    initProgressDialog();
    initWebView();
    
    initAction();
    initProcess();
}


StateEditorMainWindow::~StateEditorMainWindow()
{
}

/////////////////
// init
/////////////////
void StateEditorMainWindow::initApp()
{
    QObject::connect(QApplication::instance(), &QApplication::aboutToQuit, this, &StateEditorMainWindow::appAboutToQuit);

	QObject::connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &StateEditorMainWindow::fileChanged);
}

void StateEditorMainWindow::initConfig()
{
	
	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings settings;
	QLOG_DEBUG() << "config file " << settings.fileName();

	//Local dev
	//settings.setValue(KEY_APP_URL, STATE_EDITOR_URL_LOCAL);

	m_urlServer = settings.value(KEY_APP_URL, STATE_EDITOR_URL).toString();
	
    QLOG_DEBUG() << "url " << m_urlServer.toString();

}

QString StateEditorMainWindow::getLogPath()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath(m_logFile);
}

void StateEditorMainWindow::initLog()
{
   QCoreApplication *pApp = QCoreApplication::instance();
   QsLogging::Logger& logger = QsLogging::Logger::instance();
   logger.setLoggingLevel(QsLogging::TraceLevel);

   const QString logPath = getLogPath();
   QsLogging::DestinationPtr fileDestination(
      QsLogging::DestinationFactory::MakeFileDestination(logPath) );
   logger.addDestination(fileDestination);
   //Log to standard output
   QsLogging::DestinationPtr debugDestination = QsLogging::DestinationFactory::MakeDebugOutputDestination();
   logger.addDestination(debugDestination);

    QLOG_DEBUG() << "Log file: " << logPath;
}

void StateEditorMainWindow::initWebView()
{
    QLOG_DEBUG();

	webView->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
	m_webInspector.setPage(webView->page());

    m_pWebFrame = webView->page()->mainFrame();

    QObject::connect(webView, &QWebView::loadProgress, this, &StateEditorMainWindow::viewLoadProgress);
    QObject::connect(webView, &QWebView::loadFinished, this,  &StateEditorMainWindow::viewLoadFinished);
    QObject::connect(webView, &QWebView::urlChanged, this,  &StateEditorMainWindow::viewUrlChanged);

    QObject::connect(m_pWebFrame, &QWebFrame::javaScriptWindowObjectCleared, this, &StateEditorMainWindow::viewJavaScriptWindowObjectCleared);

	m_pWebFrame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
/*
	
	m_pWebFrame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);*/
}

void StateEditorMainWindow::loadView()
{
    QLOG_DEBUG() << m_urlServer.toString();
    webView->setUrl(m_urlServer);
}

void StateEditorMainWindow::initProxy()
{
    QLOG_DEBUG();
    // Set the eventual http proxy
    // tested with 188.93.20.179:8080
    QNetworkProxyQuery npq;
    npq.setUrl(m_urlServer);
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if( (proxies.size() > 0) && (proxies[0].type() != QNetworkProxy::NoProxy)){
        QNetworkProxy& proxy = proxies[0];
        QLOG_DEBUG() << "Proxy found " << proxy.hostName() << ":" << proxy.port() <<", user " << proxy.user();
        QNetworkProxy::setApplicationProxy(proxy);
    } else {
        QLOG_DEBUG() << "No proxy server selected";
    }
}

void StateEditorMainWindow::initProgressDialog()
{
    m_pProgessDialog = new QProgressDialog("Loading ...", "Quit", 0, 100, this);
    QObject::connect(m_pProgessDialog, &QProgressDialog::canceled, this, &StateEditorMainWindow::progressDialogCancel);
}

void StateEditorMainWindow::initAction()
{
    QObject::connect(actionOpen, &QAction::triggered, this, &StateEditorMainWindow::actionOpenTriggered);
    QObject::connect(actionSave, &QAction::triggered, this,  &StateEditorMainWindow::actionSaveTriggered);
    QObject::connect(actionSaveAs, &QAction::triggered, this, &StateEditorMainWindow::actionSaveAsTriggered);
    QObject::connect(actionQuit, &QAction::triggered, this, &StateEditorMainWindow::actionQuitTriggered);
    QObject::connect(actionAbout, &QAction::triggered, this, &StateEditorMainWindow::actionAboutTriggered);
    QObject::connect(actionHelp, &QAction::triggered, this, &StateEditorMainWindow::actionHelpTriggered);
    QObject::connect(actionOpenLog, &QAction::triggered, this, &StateEditorMainWindow::actionOpenLogTriggered);
    QObject::connect(actionGenerateCode, &QAction::triggered, this, &StateEditorMainWindow::actionGenerateCodeTriggered);
    QObject::connect(actionNewCppStateMachine, &QAction::triggered, this, &StateEditorMainWindow::actionNewCpp);
    QObject::connect(actionNewJavaStateMachine, &QAction::triggered, this, &StateEditorMainWindow::actionNewJava);
    QObject::connect(actionNewCSharpStateMachine, &QAction::triggered, this, &StateEditorMainWindow::actionNewCSharp);
    QObject::connect(actionScreenShoot, &QAction::triggered, this, &StateEditorMainWindow::actionScreenShootTriggered);

}

void StateEditorMainWindow::initProcess()
{
    QObject::connect(&m_generatorProcess, 
        SIGNAL(started), this, 
         SLOT(processStarted));

    QObject::connect(&m_generatorProcess, 
        SIGNAL(finished(int, QProcess::ExitStatus)),
        SLOT(processFinished(int, QProcess::ExitStatus)));

    QObject::connect(&m_generatorProcess, 
        SIGNAL(error(QProcess::ProcessError)),
        SLOT(processError(QProcess::ProcessError)));

	//Typesafe signal/slot doesn't work for these
    //QObject::connect(&m_generatorProcess, static_cast<void (QProcess::*)(int exitCode, QProcess::ExitStatus exitStatus)>(&QProcess::finished),
    //     &StateEditorMainWindow::processFinished);
    //QObject::connect(&m_generatorProcess, 
    //    static_cast<void (QProcess::*)(QProcess::ProcessError error)>(&QProcess::error),
    //     &StateEditorMainWindow::processError);
}

void StateEditorMainWindow::initGenerator(const QString& fileName)
{
    QLOG_DEBUG() << "file " << fileName;
    if(fileName.isEmpty()){
        m_runner = NULL;
    } else {
        m_runner = new GeneratorInfo(fileName);
    }   
}

bool StateEditorMainWindow::askSaveFile()
{
	if(isFileToSaved() == true){
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Information);
		msgBox.setText("Save the current state machine ?");
		QPushButton *pButtonCancel = msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
		QPushButton *pButtonSave = msgBox.addButton(tr("Save"), QMessageBox::AcceptRole);
		QPushButton *pButtonDiscard = msgBox.addButton(tr("Discard"), QMessageBox::AcceptRole);
		
		msgBox.exec();
		if (msgBox.clickedButton() == pButtonSave) {
			actionSaveTriggered();
		} else if(msgBox.clickedButton() == pButtonDiscard){
		} else {
			return false;
		}
	}

	return true;
}

/////////////////
// QAction slots 
/////////////////
void StateEditorMainWindow::actionOpenTriggered()
{
	if(askSaveFile() == false){
		return;
	}

	QString defaultDir = getStateMachineAbsolutePath();
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open State Machine"), defaultDir, tr("State Machines (*.fsmcs *.fsmvb *.fsmcpp *fsmjava)"));

    loadFile(fileName);
}

void StateEditorMainWindow::actionSaveTriggered()
{
    QLOG_DEBUG();
    if(m_fileName.isEmpty()){
        actionSaveAsTriggered();
        return;
    }

    saveFile(m_fileName);
}

void StateEditorMainWindow::actionSaveAsTriggered()
{
    QLOG_DEBUG();
    if(m_runner.isNull() == true){
        QLOG_DEBUG() << "no target language set";
        return;
    }

	QString defaultDir = getStateMachineAbsolutePath();
	QString extension = m_runner->getFileExtension();
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save State Machine"), 
        defaultDir, 
        QString("State Machines (*.%1)").arg(extension));

    QLOG_DEBUG() << fileName;
    if(fileName.isEmpty()){
        return;
    }

	QString errorMessage;
	if(isFileValid(fileName, errorMessage) == false){
		 QMessageBox::critical(this, 
            QApplication::instance()->applicationName(), 
            QString ("Invalid file name: %1").arg(errorMessage));
		 actionSaveAsTriggered();
		 return;
	}

	QFileInfo fileInfo(fileName);

	if(fileInfo.suffix() != extension)
	{
		fileName.append(".").append(extension);
	}

    initFileReadWrite(fileName);

    saveFile(fileName);
}

bool StateEditorMainWindow::isFileValid(const QString& fileName, QString &errorMessage)
{
	QFileInfo fileInfo(fileName);
	QString baseName = fileInfo.baseName();
	QLOG_DEBUG() << "base name " << baseName;
	// no number for the first caracter
	QRegExp regexpNoStartNumber("^\\d");
	if(regexpNoStartNumber.indexIn(baseName) == 0){
		errorMessage = "file name cannot start by a number";
		return false;
	}

	// no space
	QRegExp regexpNoSpace("^\\S+$");
	if(regexpNoSpace.indexIn(baseName) < 0){
		errorMessage = "file name cannot contain space";
		return false;
	}

	return true;
}

bool StateEditorMainWindow::writeContentToFile(const QString& fileName)
{
    QLOG_DEBUG() << "file " << fileName;
    QString xmlContent = getJsXmlContent();

    if(xmlContent.isEmpty()){
        QLOG_WARN() << "no xml content";
        return false;
    }

    if(fileName.isEmpty()){
        QLOG_WARN() << "no file name";
        return false;
    }

    QLOG_DEBUG() << endl << xmlContent;
   
    QFile file(fileName);
    if(!file.open(QIODevice::ReadWrite)){
        QMessageBox::critical(this, 
            QApplication::instance()->applicationName(), 
            QString ("Cannot write state machine to %1").arg(file.fileName()));
        return false;
    }

    fileWatchDisable();
	QString contentIndented = indentXml(xmlContent);
    file.resize(0);
	file.write(contentIndented.toLocal8Bit());
    file.close();

    fileWatchEnable();
    return true;
}

QString StateEditorMainWindow::indentXml(const QString& content)
{
    QLOG_DEBUG();
	QString contentIndented;
	QFile indentXslFile(":/xsl/indent.xsl");
	if(indentXslFile.open(QFile::ReadOnly))
	{
		QString xsl = QLatin1String(indentXslFile.readAll());
		//QLOG_DEBUG() << xsl;
		QXmlQuery query(QXmlQuery::XSLT20);
		query.setFocus(content);
		query.setQuery(xsl);
		if(!query.evaluateTo(&contentIndented))
		{
			QLOG_ERROR() << "cannot indent";
			contentIndented = content;
		}
	}
	else
	{
		QLOG_DEBUG() << "cannot open indent.xsl";
		contentIndented = content;
	}
    
	QLOG_DEBUG() << contentIndented;
	return contentIndented;
}

void StateEditorMainWindow::saveFile(const QString& fileName)
{
    QLOG_DEBUG() << "file " << fileName;

    if(writeContentToFile(fileName)){
        setIsFileToSaved(false);
        updateWindowTitle();
        statusBar()->showMessage("Saved", m_iStatusBarMessageTimeout);
    }
}

void StateEditorMainWindow::actionQuitTriggered()
{
    QLOG_DEBUG();
    appAboutToQuit();
    QApplication::exit(0);
}

void StateEditorMainWindow::actionAboutTriggered()
{
    QLOG_DEBUG();
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}

void StateEditorMainWindow::actionHelpTriggered()
{
    if(QDesktopServices::openUrl(QUrl(STATEFORGE_HELP_URL)) == false){
        QMessageBox::critical(this, 
            QApplication::instance()->applicationName(), 
            QString("Cannot open help at %1").arg(STATEFORGE_HELP_URL));
    }
}

void StateEditorMainWindow::actionOpenLogTriggered()
{
    const QString logPath = getLogPath();
    QLOG_DEBUG() << logPath;
     if(QDesktopServices::openUrl(QUrl::fromLocalFile(logPath)) == false){
        QMessageBox::critical(this, 
            QApplication::instance()->applicationName(), 
            QString("Cannot open log file at %1").arg(logPath));
    }
}

void StateEditorMainWindow::actionGenerateCodeTriggered()
{
    QLOG_DEBUG();

    if(m_fileName.isEmpty()){
        actionSaveTriggered();
        if(m_fileName.isEmpty()){
            return;
        }  
    }

	QLOG_DEBUG();
    if(m_runner.isNull() == true){
        QLOG_ERROR() << "no code generator runner";
        return;
    }

    QLOG_DEBUG() << m_runner->getCommandLine();

    m_generatorProcess.start(
        m_runner->getProgramPath(), 
        m_runner->getProgramParameters());
}

void StateEditorMainWindow::actionNewCpp()
{
    loadFileReadOnly(STATE_MACHINE_NEW_CPP);
}

void StateEditorMainWindow::actionNewCSharp()
{
    loadFileReadOnly(STATE_MACHINE_NEW_CSHARP);
}

void StateEditorMainWindow::actionNewJava()
{
    loadFileReadOnly(STATE_MACHINE_NEW_JAVA);
}

const QString StateEditorMainWindow::getScreenShootAbsoluteFilePath() const
{
    const QString stateMachineName = getStateMachineName();
    if(stateMachineName.isEmpty()){
        QLOG_DEBUG() << "no state machine set";
        return "";
    }

    const QString baseFileNameWithExtension = QString("%1.png").arg(stateMachineName);
    const QString absolutePath = getStateMachineAbsolutePath();
    QFileInfo screenShootFileInfo(QDir(absolutePath), baseFileNameWithExtension);
    const QString screenShootAbsoluteFilePath = screenShootFileInfo.absoluteFilePath();
    QLOG_DEBUG() << screenShootAbsoluteFilePath;
    return screenShootAbsoluteFilePath;
}

void StateEditorMainWindow::actionScreenShootTriggered()
{
    int width = getWidth();
    int offsetLeft = getOffsetLeft();
    int height = getHeight();
    int offsetTop = getOffsetTop();
    QLOG_DEBUG() << width << " x " << height;
    QLOG_DEBUG() << offsetLeft << " x " << offsetTop;
    if((width <= 0) || (height <= 0)){
        return;
    }

	if(offsetLeft < 0) {
		offsetLeft = 0;
	}

    const QString screenShootAbsoluteFilePathDefault = getScreenShootAbsoluteFilePath();

    QString screenShootAbsoluteFilePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Screenshoot"), 
        screenShootAbsoluteFilePathDefault, 
        QString("(*.png)"));

    QLOG_DEBUG() << "selected " << screenShootAbsoluteFilePath;
    if(screenShootAbsoluteFilePath.isEmpty()){
        return;
    }

	QFileInfo fileInfo(screenShootAbsoluteFilePath);

    if(fileInfo.suffix() != "png")
	{
		screenShootAbsoluteFilePath.append(".").append("png");
	}
    QWebPage *pPage = webView->page();
    QWebFrame *pMainFrame = pPage->mainFrame();

	pMainFrame->evaluateJavaScript("stateMachineRemoveBackground()");

    const QRegion region(offsetLeft, offsetTop, width, height);

	QSize viewSizeOriginal = pPage->viewportSize();

    QSize contentsSize = pMainFrame->contentsSize();
    QLOG_DEBUG() << "content size " << contentsSize.width() << "x" << contentsSize.height();
	QSize viewPortSize(width + offsetLeft, height + offsetTop);
    pPage->setViewportSize(pMainFrame->contentsSize());
    QImage imageFull(width + offsetLeft, height + offsetTop, QImage::Format_ARGB32_Premultiplied);
    QPainter painter;

    painter.begin(&imageFull);
    painter.setRenderHint(QPainter::Antialiasing, false);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
	painter.setRenderHint(QPainter::TextAntialiasing, false);
    pMainFrame->render(&painter, QWebFrame::ContentsLayer, region);
    painter.end();
    QImage image = imageFull.copy(offsetLeft, offsetTop, width, height);

	pPage->setViewportSize(viewSizeOriginal);

	pMainFrame->evaluateJavaScript("stateMachineAddBackground()");

    if(image.save(screenShootAbsoluteFilePath, "png")){
        QLOG_DEBUG() << "saved";
    } else {
        QLOG_ERROR() << "cannot save screenshoot";
        QMessageBox::critical(this, 
            QApplication::instance()->applicationName(), 
            QString("Cannot save screenshoot %1").arg(screenShootAbsoluteFilePath));
    }

	if(QDesktopServices::openUrl(QUrl::fromLocalFile(screenShootAbsoluteFilePath)) == false){
		QLOG_ERROR() << "Cannot open screenshoot" << screenShootAbsoluteFilePath;
		// png cannot be opened by default on XP
        /*QMessageBox::critical(this, 
            QApplication::instance()->applicationName(), 
            QString("Cannot open screenshoot %1").arg(screenShootAbsoluteFilePath));*/
    }
}

///////////////////
// QProcess slots
///////////////////

void StateEditorMainWindow::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
   QLOG_DEBUG() << "exit code " << exitCode << ", status " << exitStatus;
   if(exitCode != 0) {
	   QString consoleOutput = QString::fromLocal8Bit(m_generatorProcess.readAllStandardError());
	   QLOG_DEBUG() << consoleOutput;
       QString errorMessage = QString("Code generator error\n%1 returns code %2\n%3")
           .arg(m_runner->getCommandLine())
           .arg(exitCode)
		   .arg(consoleOutput);
       
       QMessageBox::critical(this, 
           QApplication::instance()->applicationName(), 
           errorMessage);
       statusBar()->showMessage(QString("Generation finished, exit code %1").arg(exitCode));
   } else {
       statusBar()->showMessage("Generation finished", m_iStatusBarMessageTimeout);
   }
}

void StateEditorMainWindow::processStarted()
{
    QLOG_DEBUG();
    statusBar()->showMessage("Generation started");
}

void StateEditorMainWindow::processError(QProcess::ProcessError error)
{
    QLOG_DEBUG() << "error " << error;
    QString errorMessage;
    switch(error){
        case QProcess::FailedToStart:
            errorMessage = QString("Failed to start %1\n"
                "Either the invoked program is missing, "
                "or you may have insufficient permissions to invoke the program.")
                .arg(m_runner->getProgramPath());
            QMessageBox::critical(this, 
                QApplication::instance()->applicationName(), 
                errorMessage);
            break;
        case QProcess::Crashed:
            break;
        default:
            break;
    }
}

///////////////////
// QWebView slots 
///////////////////
void StateEditorMainWindow::viewLoadProgress(int p)
{
    QLOG_DEBUG() << p;
    m_pProgessDialog->setValue(p);
}

void StateEditorMainWindow::viewLoadFinished(bool ok)
{
    QLOG_DEBUG() << "ok " << ok;
    m_pProgessDialog->setValue(100);
    if(ok == true){
        render();
    } else {
        QMessageBox::critical(this, 
            QApplication::instance()->applicationName(), 
            "Cannot load page, check the network connection");
    }
}

void StateEditorMainWindow::viewUrlChanged(const QUrl& url)
{
    QLOG_DEBUG() << url.toString();
	QUrlQuery urlQuery(url);
	QString fsmUrl = urlQuery.queryItemValue("fsmUrl");
	if(fsmUrl.isEmpty() == false){
		if(m_fileName.isEmpty() == false){
			askSaveFile();

		}
        initFileReadOnly(fsmUrl);
    }
}

void StateEditorMainWindow::viewJavaScriptWindowObjectCleared() {
    m_pWebFrame->addToJavaScriptWindowObject(QString("stateEditorMainWindow"), this);
}

void StateEditorMainWindow::progressDialogCancel()
{
    QLOG_DEBUG();
    QApplication::exit(-1);
}

////////////////////////
// QFileSystemWatcher
////////////////////////
void StateEditorMainWindow::fileChanged(const QString &file)
{
    QLOG_DEBUG() << "fileChanged " << file;

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("File has been modified by another application");
    QPushButton *pButtonReload = msgBox.addButton(tr("Reload state machine from disk"), QMessageBox::AcceptRole);
    QPushButton *pButtonKeep = msgBox.addButton(tr("Keep current state machine"), QMessageBox::AcceptRole);

    msgBox.exec();

    if (msgBox.clickedButton() == pButtonReload) {
        // Reload
        loadFile(m_fileName);
    } else {
        m_fileWatcher.addPath(file);
    }
}

/////////////////
// Utils
/////////////////

void StateEditorMainWindow::loadFileReadOnly(const QString& fileName)
{
    QLOG_DEBUG() << fileName;
    initFileReadOnly(fileName);
    loadFileCommon(fileName);
}

void StateEditorMainWindow::loadFile(const QString& fileName)
{
    QLOG_DEBUG() << fileName;
	if(fileName.isEmpty()){
		return;
	}
    initFileReadWrite(fileName);
    loadFileCommon(fileName);
}

void StateEditorMainWindow::loadFileCommon(const QString& fileName)
{
    QLOG_DEBUG() << fileName;
    setContentFromFile(fileName);
    loadView();
}

void StateEditorMainWindow::initFileCommon(const QString& fileName)
{
    QLOG_DEBUG() << fileName;
    setIsFileToSaved(false);
    setStateMachineFromFileName(fileName);
    updateWindowTitle();
    setXmlContent("");  
    initGenerator(fileName);
}

void StateEditorMainWindow::initFileReadOnly(const QString& fileName)
{
    QLOG_DEBUG() << fileName;
    setFileName("");
    initFileCommon(fileName);
}

void StateEditorMainWindow::initFileReadWrite(const QString& fileName)
{
    QLOG_DEBUG() << fileName;
    setFileName(fileName);  
    initFileCommon(fileName);
}

void StateEditorMainWindow::setContentFromFile(const QString& fileName)
{
    QLOG_DEBUG() << "fileName " << fileName; 
    if(fileName.isEmpty() == false){
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly)){
            QMessageBox::critical(this, 
                QApplication::instance()->applicationName(), 
                QString ("Cannot open state machine %1").arg(file.fileName()));
            initFileReadWrite("");
            return;
        }

        setXmlContent(QString(file.readAll()));
    }
}

QString StateEditorMainWindow::getJsXmlContent()
{
    QLOG_DEBUG();
    return m_pWebFrame->evaluateJavaScript("stateMachineGetXmlContent()").toString();
}

void StateEditorMainWindow::setXmlContent(const QString& xmlContent)
{
    QLOG_DEBUG() << endl << xmlContent;
    m_xmlContent = xmlContent;
}

QString StateEditorMainWindow::getXmlContent()
{
    QLOG_DEBUG() << "size " << m_xmlContent.size();
    QString xmlContextCopy = m_xmlContent;
    setXmlContent("");
    return xmlContextCopy;
}

void StateEditorMainWindow::setIsFileToSaved(bool bFileToSaved)
{
    QLOG_DEBUG() << " " << bFileToSaved;
    m_bFileToSaved = bFileToSaved;
}

bool StateEditorMainWindow::isFileToSaved()
{
    QLOG_DEBUG() << " " << m_bFileToSaved;
    return m_bFileToSaved;
}

void StateEditorMainWindow::render()
{
    if(!m_xmlContent.isEmpty()){
        QLOG_DEBUG();
        m_pWebFrame->evaluateJavaScript("stateMachineSetXmlContent()");
    } else {
        QLOG_DEBUG() << "nothing to render";
    }
}

void StateEditorMainWindow::modelModified()
{
    QLOG_DEBUG();
    setIsFileToSaved(true);
    updateWindowTitle();
}

void StateEditorMainWindow::updateWindowTitle()
{
    QString fileName = m_fileName;
    QLOG_DEBUG() << fileName;
    if(fileName.isEmpty() == true){
        fileName = getStateMachineName();
    }

    if(isFileToSaved() == true){
        setWindowTitle(QString("%1 *").arg(fileName));
    } else {
        setWindowTitle(QString("%1").arg(fileName));
    }
}

void StateEditorMainWindow::setFileName(const QString& fileName)
{
    QLOG_DEBUG() << fileName;

    //Disable file watch for the old one
    fileWatchDisable();
    m_fileName = fileName;
    //Enable file watch for the new one
    fileWatchEnable();
}

void StateEditorMainWindow::fileWatchEnable()
{
    if(!m_fileName.isEmpty()){
        m_fileWatcher.addPath(m_fileName);
    } else {
        QLOG_ERROR() << "no file to watch";
    }
}

void StateEditorMainWindow::fileWatchDisable()
{
    if(!m_fileName.isEmpty()){
        m_fileWatcher.removePath(m_fileName);
    }
}
void StateEditorMainWindow::appAboutToQuit()
{
    QLOG_DEBUG();
	askSaveFile();
}

void StateEditorMainWindow::setStateMachineFromFileName(const QString& fileName)
{
    QFileInfo fileInfo(fileName);
    m_name = fileInfo.baseName();
    QLOG_DEBUG() << "file " << fileName << " => " << m_name;
}

const QString& StateEditorMainWindow::getStateMachineName() const
{
    return m_name;
}

const QString StateEditorMainWindow::getStateMachineAbsolutePath() const
{
    if(!m_fileName.isEmpty()){
        QFileInfo fileInfo(m_fileName);
        return fileInfo.absolutePath();
    } else {
        return QDir::homePath();
    }
}

int StateEditorMainWindow::getWidth()
{
    return m_pWebFrame->evaluateJavaScript("stateMachineGetWidth()").toInt();
}

int StateEditorMainWindow::getOffsetLeft()
{
    return m_pWebFrame->evaluateJavaScript("stateMachineGetOffsetLeft()").toInt();
}

int StateEditorMainWindow::getHeight()
{
    return m_pWebFrame->evaluateJavaScript("stateMachineGetHeight()").toInt();
}

int StateEditorMainWindow::getOffsetTop()
{
    return m_pWebFrame->evaluateJavaScript("stateMachineGetOffsetTop()").toInt();
}

#include "StateEditorMainWindow.moc"
