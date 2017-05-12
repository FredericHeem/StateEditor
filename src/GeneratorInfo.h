#ifndef GENERATOR_RUNNER_H
#define GENERATOR_RUNNER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>


class GeneratorInfo : public QObject
{
    Q_OBJECT

public:
    GeneratorInfo(const QString& fileName);
    ~GeneratorInfo(void);

    const QString& getFileExtension() const {return m_fileExtension;}
    const QString& getProgramPath() const {return m_programPath;}
    const QStringList& getProgramParameters() const {return m_parameterList;}

    const QString getCommandLine() const;

private:
    void setupDotNet();
    void setupCpp();
    void setupJava();

    void initProgramPath();

    QString m_name;
    QString m_fileName;
    QString m_basePath;
    QString m_fileExtension;
    QString m_programPath;
    QStringList m_parameterList;
};

#endif