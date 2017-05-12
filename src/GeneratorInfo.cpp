#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QsLog.h>

#include "GeneratorInfo.h"
#include "StateEditorDefs.h"

GeneratorInfo::GeneratorInfo(const QString& fileName):
m_fileName(fileName)
{
    QFileInfo fileInfo(fileName);
    m_fileExtension = fileInfo.suffix();
    QLOG_DEBUG() << fileName;
    if(m_fileExtension == "fsmcs"){
        setupDotNet();
    } else if (m_fileExtension == "fsmcpp"){
        setupCpp();
    } else if(m_fileExtension == "fsmjava"){
        setupJava();
    }
}

GeneratorInfo::~GeneratorInfo(void)
{
}

void GeneratorInfo::setupDotNet()
{
#ifdef Q_OS_WIN
    QLOG_DEBUG();
    QSettings settings(
        "HKEY_CURRENT_USER\\SOFTWARE\\StateForge\\StateBuilderDotNet",
        QSettings::NativeFormat);
    QFileInfo appFileInfo(
        settings.value( "InstallDir").toString(),
        "Bin/StateBuilderDotNet.exe");
    m_programPath = appFileInfo.absoluteFilePath();
    m_parameterList << "-b" << m_fileName ;
#endif /* #ifdef Q_OS_WIN */
}

void GeneratorInfo::setupJava()
{
    QLOG_DEBUG();
    m_name = "StateBuilderJava";
    m_basePath = "bin";

    #ifdef _WINDOWS
    QSettings settings(
        "HKEY_CURRENT_USER\\SOFTWARE\\StateForge\\StateBuilderJava",
        QSettings::NativeFormat);
    QFileInfo appFileInfo(
        settings.value( "InstallLocation").toString(),
        QString("lib/statebuilder-java-%1.jar").arg(STATEBUILDER_JAVA_VERSION));

    m_programPath = "java";

    m_parameterList << "-jar" << appFileInfo.absoluteFilePath() << m_fileName;
#else
#error
#endif /* #ifdef Q_OS_WIN */

}

void GeneratorInfo::setupCpp()
{
    QLOG_DEBUG();
#ifdef _WINDOWS
    QSettings settings(
        "HKEY_CURRENT_USER\\SOFTWARE\\StateForge\\StateBuilderCpp",
        QSettings::NativeFormat);
    QFileInfo appFileInfo(
        settings.value( "InstallLocation").toString(),
        "bin/StateBuilderCpp.exe");

    m_programPath = appFileInfo.absoluteFilePath();
#else
#error
#endif /* #ifdef Q_OS_WIN */

    m_parameterList << m_fileName ;
}

const QString GeneratorInfo::getCommandLine() const
{
    QString commandLine =  m_programPath + " " + m_parameterList.join(" ");
    return commandLine;
}

#include "GeneratorInfo.moc"