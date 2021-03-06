/****************************************************************************
**
** Copyright (C) 2014 Alexander Rössler
** License: LGPL version 2.1
**
** This file is part of QtQuickVcp.
**
** All rights reserved. This program and the accompanying materials
** are made available under the terms of the GNU Lesser General Public License
** (LGPL) version 2.1 which accompanies this distribution, and is available at
** http://www.gnu.org/licenses/lgpl-2.1.html
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Lesser General Public License for more details.
**
** Contributors:
** Alexander Rössler @ The Cool Tool GmbH <mail DOT aroessler AT gmail DOT com>
**
****************************************************************************/

#include "applicationfile.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

namespace qtquickvcp {

ApplicationFile::ApplicationFile(QObject *parent) :
    QObject(parent),
    m_editMode(false),
    m_uri(""),
    m_localFilePath(""),
    m_remoteFilePath(""),
    m_localPath(""),
    m_remotePath(""),
    m_serverDirectory(""),
    m_transferState(NoTransfer),
    m_error(NoError),
    m_errorString(""),
    m_progress(0.0),
    m_networkReady(false),
    m_model(nullptr),
    m_ready(false),
    m_networkManager(nullptr),
    m_file(nullptr),
    m_ftp(nullptr)
{
    m_localPath = generateTempPath();

    m_model = new ApplicationFileModel(this);

    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::networkAccessibleChanged,
            this, &ApplicationFile::networkAccessibleChanged);

    if (m_networkManager->networkAccessible() == QNetworkAccessManager::Accessible)
    {
        initializeFtp();
    }
}

ApplicationFile::~ApplicationFile()
{
    cleanupTempPath();
    cleanupFtp();
    cleanupFile();
    m_networkManager->deleteLater();
    m_model->deleteLater();
}

bool ApplicationFile::isFileExist(QString fileName)
{
    if (fileName.isEmpty()) return false;

    QString localFilePath = applicationFilePath(fileName, m_serverDirectory);
    QFileInfo fileInfo(localFilePath);
    qDebug("%s(%s:%d) localFilePath(%s)", __FILE__, __FUNCTION__, __LINE__, localFilePath.toLatin1().constData());
    return fileInfo.exists();
}


void ApplicationFile::startUpload()
{
    QUrl url;
    QFileInfo fileInfo(QUrl(m_localFilePath).toLocalFile());
    QString remotePath;

    if (!ready() || (m_transferState != NoTransfer))
    {
        return;
    }

    url.setUrl(m_uri);
    remotePath = QUrl(m_remotePath).toLocalFile();
    if (!m_serverDirectory.isEmpty())
    {
        remotePath.append("/");
        remotePath.append(m_serverDirectory);
    }
    m_remoteFilePath = QUrl::fromLocalFile(QDir(remotePath).filePath(fileInfo.fileName())).toString();
    emit remoteFilePathChanged(m_remoteFilePath);

    m_file = new QFile(fileInfo.filePath(), this);

    if (m_file->open(QIODevice::ReadOnly))
    {
        m_ftp->connectToHost(url.host(), url.port());
        m_ftp->login();
        if (!m_serverDirectory.isEmpty())
        {
            m_ftp->cd(m_serverDirectory);
        }
        m_ftp->put(m_file, fileInfo.fileName(), QFtp::Binary);
        m_ftp->close();

        m_progress = 0.0;
        emit progressChanged(m_progress);
        updateState(UploadRunning);
        updateError(NoError, "");
    }
    else
    {
        updateState(Error);
        updateError(FileError, m_file->errorString());
    }
}

void ApplicationFile::startDownload()
{
    QUrl url;
    QDir dir;
    QString localFilePath;
    QString remoteFilePath;
    QString remotePath;
    QString fileName;

    if (!ready() || (m_transferState != NoTransfer))
    {
        return;
    }

    remoteFilePath = QUrl(m_remoteFilePath).toLocalFile();
    remotePath = QUrl(m_remotePath).toLocalFile();
    fileName = remoteFilePath.mid(remotePath.length() + 1);

    int i = fileName.indexOf("/");
    if (i > -1) // not in root directory
    {
        m_serverDirectory = fileName.left(i);
        emit serverDirectoryChanged(m_serverDirectory);
        fileName = fileName.mid(i + 1);
    }
    m_fileName = fileName;
    emit fileNameChanged(m_fileName);

    url.setUrl(m_uri);

    localFilePath = applicationFilePath(fileName, m_serverDirectory);
    m_localFilePath = QUrl::fromLocalFile(localFilePath).toString();
    emit localFilePathChanged(m_localFilePath);

    QFileInfo fileInfo(localFilePath);

    if (!dir.mkpath(fileInfo.absolutePath()))
    {
        updateState(Error);
        updateError(FileError, tr("Cannot create directory: %1").arg(fileInfo.absolutePath()));
        return;
    }

    m_file = new QFile(localFilePath, this);

    if (m_file->open(QIODevice::WriteOnly))
    {
        m_ftp->connectToHost(url.host(), url.port());
        m_ftp->login();
        if (!m_serverDirectory.isEmpty())
        {
            m_ftp->cd(m_serverDirectory);
        }
        m_ftp->get(fileName, m_file, QFtp::Binary);
        m_ftp->close();
        m_progress = 0.0;
        emit progressChanged(m_progress);
        updateState(DownloadRunning);
        updateError(NoError, "");
    }
    else
    {
        updateState(Error);
        updateError(FileError, m_file->errorString());
    }
}

void ApplicationFile::refreshFiles()
{
    QUrl url(m_uri);

    if (!ready() || (m_transferState != NoTransfer))
    {
        return;
    }

    m_model->clear();
    m_ftp->connectToHost(url.host(), url.port());
    m_ftp->login();
    if (!m_serverDirectory.isEmpty())
    {
        m_ftp->cd(m_serverDirectory);
    }
    m_ftp->list();
    m_ftp->close();
    updateState(RefreshRunning);
}

void ApplicationFile::removeFile(const QString &name)
{
    QUrl url(m_uri);

    if (!ready() || (m_transferState != NoTransfer))
    {
        return;
    }

    m_ftp->connectToHost(url.host(), url.port());
    m_ftp->login();
    if (!m_serverDirectory.isEmpty())
    {
        m_ftp->cd(m_serverDirectory);
    }
    m_ftp->remove(name);
    m_ftp->close();
    updateState(RemoveRunning);
}

void ApplicationFile::removeDirectory(const QString &name)
{
    QUrl url(m_uri);

    if (!ready() || (m_transferState != NoTransfer))
    {
        return;
    }

    m_ftp->connectToHost(url.host(), url.port());
    m_ftp->login();
    if (!m_serverDirectory.isEmpty())
    {
        m_ftp->cd(m_serverDirectory);
    }
    m_ftp->rmdir(name);
    m_ftp->close();
    updateState(RemoveDirectoryRunning);
}

void ApplicationFile::createDirectory(const QString &name)
{
    QUrl url(m_uri);

    if (!ready() || (m_transferState != NoTransfer))
    {
        return;
    }

    m_ftp->connectToHost(url.host(), url.port());
    m_ftp->login();
    if (!m_serverDirectory.isEmpty())
    {
        m_ftp->cd(m_serverDirectory);
    }
    m_ftp->mkdir(name);
    m_ftp->close();
    updateState(CreateDirectoryRunning);
}

void ApplicationFile::abort()
{
    if (!m_ftp)
    {
        return;
    }
    updateState(NoTransfer);
    m_ftp->abort();
    m_ftp->close();
}

void ApplicationFile::clearError()
{
    updateState(NoTransfer);
    updateError(NoError, "");
}

void ApplicationFile::updateState(ApplicationFile::TransferState state)
{
    if (state != m_transferState)
    {
        m_transferState = state;
        emit transferStateChanged(m_transferState);
    }
}

void ApplicationFile::updateError(ApplicationFile::TransferError error, const QString &errorString)
{
    if (m_errorString != errorString)
    {
        m_errorString = errorString;
        emit errorStringChanged(m_errorString);
    }

    if (m_error != error)
    {
        m_error = error;
        emit errorChanged(m_error);
    }
}

QString ApplicationFile::generateTempPath() const
{
    return QUrl::fromLocalFile(QString("%1/machinekit-%2").arg(QDir::tempPath())
            .arg(QCoreApplication::applicationPid())).toString();
}

void ApplicationFile::cleanupTempPath()
{
    QDir dir(m_localPath);

    dir.removeRecursively();
}

QString ApplicationFile::applicationFilePath(const QString &fileName, const QString &serverDirectory) const
{
    return QDir(QUrl(m_localPath).toLocalFile() + "/" + serverDirectory).filePath(fileName);
}

void ApplicationFile::initializeFtp()
{
    m_ftp = new QFtp(this);
    connect(m_ftp, &QFtp::commandFinished,
    this, &ApplicationFile::ftpCommandFinished);
    connect(m_ftp, &QFtp::listInfo,
    this, &ApplicationFile::addToList);
    connect(m_ftp, &QFtp::listUpdate,
    this, &ApplicationFile::updateList);
    connect(m_ftp, &QFtp::dataTransferProgress,
    this, &ApplicationFile::transferProgress);

    m_networkReady = true;
    emit readyChanged(m_networkReady);
}

void ApplicationFile::cleanupFtp()
{
    if (m_ftp == nullptr)
    {
        return;
    }

    m_ftp->abort();
    m_ftp->deleteLater();
    m_ftp = nullptr;

    m_networkReady = false;
    emit readyChanged(m_networkReady);
}

void ApplicationFile::cleanupFile()
{
    if (m_file == nullptr)
    {
        return;
    }

    m_file->close();
    m_file->deleteLater();
    m_file = nullptr;
}

void ApplicationFile::transferProgress(qint64 bytesSent, qint64 bytesTotal)
{
    m_progress = static_cast<double>(bytesSent) / static_cast<double>(bytesTotal);
    emit progressChanged(m_progress);
}

void ApplicationFile::networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accesible)
{
    if (accesible == QNetworkAccessManager::Accessible)
    {
        initializeFtp();
    }
    else
    {
        cleanupFtp();
    }
}

void ApplicationFile::addToList(const QUrlInfo &urlInfo)
{
    ApplicationFileItem *item;

    item = new ApplicationFileItem();
    item->setName(urlInfo.name());
    item->setSize(urlInfo.size());
    item->setOwner(urlInfo.owner());
    item->setGroup(urlInfo.group());
    item->setLastModified(urlInfo.lastModified());
    item->setDir(urlInfo.isDir());

    m_model->addItem(item);
}

void ApplicationFile::updateList()
{
    // qDebug("%s(%s:%d) begin", __FILE__, __FUNCTION__, __LINE__);
    // quint64 baseTime = QDateTime::currentMSecsSinceEpoch();
    m_model->beginUpdate();
    m_model->endUpdate(); // this takes hundreds of ms
    // qDebug("%s(%s:%d) end time (%llu)", __FILE__, __FUNCTION__, __LINE__, QDateTime::currentMSecsSinceEpoch() - baseTime);
}


void ApplicationFile::ftpCommandFinished(int, bool error)
{
    if (error)
    {
        cleanupFile();
        m_ftp->close();

        if (m_transferState != NoTransfer) // may be a user abort operation
        {
            updateState(Error);
            updateError(FtpError, m_ftp->errorString());
        }

        return;
    }

    switch (m_ftp->currentCommand())
    {
    case QFtp::Get:
        cleanupFile();
        emit downloadFinished();
        updateState(NoTransfer);
        return;
    case QFtp::List:
        emit refreshFinished();
        updateState(NoTransfer);
        return;
    case QFtp::Put:
        cleanupFile();
        emit uploadFinished();
        break;
    case QFtp::Remove:
        emit removeFinished();
        break;
    case QFtp::Rmdir:
        emit removeDirectoryFinished();
        break;
    case QFtp::Mkdir:
        emit createDirectoryFinished();
        break;
    default:
        return;
    }

    updateState(NoTransfer);
    refreshFiles();
}
} // namespace qtquickvcp
