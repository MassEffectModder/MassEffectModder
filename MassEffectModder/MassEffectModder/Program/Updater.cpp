/*
 * MassEffectModder
 *
 * Copyright (C) 2019-2021 Pawel Kolodziejski
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <Gui/MainWindow.h>
#include <Helpers/Exception.h>
#include <Helpers/Logs.h>
#include <Helpers/MiscHelpers.h>
#include <Program/Updater.h>
#include <Wrappers.h>

Updater::Updater(QMainWindow *window) :
    replyRelease(nullptr), parentWindow(window)
{
    connect(&managerListReleases, SIGNAL(finished(QNetworkReply *)), SLOT(finishedDownloadRelasesList(QNetworkReply *)));
    connect(&managerRelease, SIGNAL(finished(QNetworkReply *)), SLOT(finishedDownloadRelase(QNetworkReply *)));
}

Updater::~Updater()
{
    delete file;
    delete tmpDir;
    file = nullptr;
    tmpDir = nullptr;
}

void Updater::ButtonSelected()
{
    if (!replyRelease->isFinished())
        replyRelease->abort();
    dialog.close();
}

void Updater::finishedDownloadRelase(QNetworkReply *reply)
{
    QApplication::processEvents();

    if (reply->error())
    {
        textWidget.setText(QString("Downloading failed! Error: ") + reply->errorString());
        g_logs->PrintError(QString("Updater: Failed download releases list! Error: ") + reply->errorString());
        reply->abort();
        file->close();
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 || statusCode == 307 || statusCode == 308)
    {
        QNetworkRequest request;
        request.setUrl(QUrl(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()));
        request.setRawHeader("User-Agent", "MEM");
        replyRelease = managerRelease.get(request);
        connect(replyRelease, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
        connect(replyRelease, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
        return;
    }

    button.setText("Close");
    QApplication::processEvents();
    file->close();

    auto userDownloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (!QDir(userDownloadPath).exists())
    {
        textWidget.setText(QString("User download path not exists! Path: ") + userDownloadPath);
        g_logs->PrintError(QString("Updater: User download path not exists! Path: ") + userDownloadPath);
        return;
    }
    const QString dstPath = QDir::cleanPath(userDownloadPath + "/" + BaseName(filePath));
    if (QFile(dstPath).exists())
    {
        if (!QFile(dstPath).remove())
        {
            textWidget.setText(QString("Failed to override existing file! Path: ") + dstPath);
            g_logs->PrintError(QString("Updater: Failed to override existing file! Path: " + dstPath));
            return;
        }
    }
    if (!QFile::copy(filePath, dstPath))
    {
        textWidget.setText(QString("Failed copy downloaded file to user download path!"));
        g_logs->PrintError(QString("Updater: Failed copy downloaded file to user download path"));
        return;
    }

    textWidget.setText("Unpacking archive...");
    QApplication::processEvents();
    const QString unpackPath = QDir::cleanPath(userDownloadPath + "/" + BaseNameWithoutExt(filePath));
    if (!dstPath.endsWith(".dmg", Qt::CaseInsensitive) && !QDir(unpackPath).exists(unpackPath))
        QDir(unpackPath).mkpath(unpackPath);
    int status = 0;
    if (dstPath.endsWith(".7z", Qt::CaseInsensitive))
    {
#if defined(_WIN32)
        status = SevenZipUnpack(dstPath.toStdWString().c_str(), unpackPath.toStdWString().c_str(), "", true);
#else
        status = SevenZipUnpack(dstPath.toStdString().c_str(), unpackPath.toStdString().c_str(), "", false);
#endif
    }
    else if (dstPath.endsWith(".zip", Qt::CaseInsensitive))
    {
#if defined(_WIN32)
        status = ZipUnpack(dstPath.toStdWString().c_str(), unpackPath.toStdWString().c_str(), "", false);
#else
        status = ZipUnpack(dstPath.toStdString().c_str(), unpackPath.toStdString().c_str(), "", false);
#endif
    }
    else if (dstPath.endsWith(".dmg", Qt::CaseInsensitive))
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(dstPath));
    }
    else
        CRASH_MSG("Updater: Not supported archive!");
    if (status != 0)
    {
        textWidget.setText("Failed unpack archive!");
        g_logs->PrintError(QString("Updater: Failed unpack archive!"));
        return;
    }

    if (!dstPath.endsWith(".dmg", Qt::CaseInsensitive))
        QDesktopServices::openUrl(QUrl::fromLocalFile(unpackPath));

    textWidget.setText("Downloading and unpacking archive completed successfully.");
}

void Updater::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (replyRelease->error())
    {
        textWidget.setText(QString("Downloading failed! Error: ") + replyRelease->errorString());
        g_logs->PrintError(QString("Updater: Failed download releases list! Error: ") + replyRelease->errorString());
        replyRelease->abort();
        return;
    }

    int statusCode = replyRelease->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 || statusCode == 307 || statusCode == 308)
        return;

    int percent = (bytesReceived * 100.0) / bytesTotal;
    file->write(replyRelease->readAll());
    if (file->error() != QFileDevice::NoError)
    {
        textWidget.setText(QString("Failed write to temporary file! Error: ") + file->errorString());
        g_logs->PrintError(QString("Updater: Failed write to temporary file! Error: ") + file->errorString());
        replyRelease->abort();
        return;
    }

    if (percent > 0)
    {
        progressBar.setValue(percent);
        QApplication::processEvents();
    }
}

void Updater::downloadRelease(const QString &downLoadUrl)
{
    tmpDir = new QTemporaryDir;
    if (!tmpDir->isValid())
    {
        g_logs->PrintError(QString("Updater: Failed create temporary dir! Error: ") + tmpDir->errorString());
        return;
    }
    filePath = tmpDir->path() + "/" + BaseName(downLoadUrl);
    file = new QFile(filePath);
    file->open(QIODevice::WriteOnly | QIODevice::Truncate);
    if (!file->isOpen())
    {
        g_logs->PrintError(QString("Updater: Failed create temporary file! Error: ") + file->errorString());
        delete file;
        file = nullptr;
        delete tmpDir;
        tmpDir = nullptr;
        return;
    }

    QNetworkRequest request;
    request.setUrl(QUrl(downLoadUrl));
    request.setRawHeader("User-Agent", "MEM");
    replyRelease = managerRelease.get(request);
    connect(replyRelease, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
    connect(replyRelease, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));

    QVBoxLayout layout;
    dialog.setBaseSize(500, 100);
    dialog.setMinimumSize(500, 100);
    dialog.setWindowTitle("Updater");
    textWidget.setText("Downloading...");
    button.setText("Cancel");
    button.setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    button.setMinimumWidth(kButtonMinWidth);
    button.setMinimumHeight(kButtonMinHeight / 2);
    button.setAutoDefault(false);
    QFont ButtonFont = button.font();
    ButtonFont.setPointSize(kFontSize);
    button.setFont(ButtonFont);
    QObject::connect(&button, &QPushButton::clicked, this, &Updater::ButtonSelected);
    progressBar.setRange(0, 100);
    progressBar.setValue(0);
    layout.addWidget(&textWidget);
    layout.addWidget(&progressBar);
    layout.addWidget(&button, 0, Qt::AlignCenter);
    dialog.setLayout(&layout);
    dialog.exec();
}

void Updater::sslErrors(const QList<QSslError> &sslErrors)
{
    for (const QSslError &error : sslErrors)
        g_logs->PrintError(QString("Updater: SSL connection error: ") + error.errorString());
}

void Updater::finishedDownloadRelasesList(QNetworkReply *reply)
{
    if (reply->error())
    {
        g_logs->PrintError(QString("Updater: Failed download releases list! Error: ") + reply->errorString());
        return;
    }

    QJsonParseError jsonError{};
    QJsonDocument jsonRelases = QJsonDocument::fromJson(reply->readAll(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
    {
        g_logs->PrintError(QString("Updater: Failed parse releases list! Error: ") + jsonError.errorString());
        return;
    }

#if defined(_WIN32)
    QString pattern = "MassEffectModder-v";
#elif defined(__linux__)
    QString pattern = "MassEffectModder-Linux-v";
#else
    QString pattern = "MassEffectModder-macOS-v";
#endif
    QList<QVariant> listLateterReleases;
    int latestVersion = MEM_VERSION;
    QList<QVariant> listReleases = jsonRelases.toVariant().toList();
    for (const QVariant &releaseNode : listReleases)
    {
        QMap<QString, QVariant> mapReleases = releaseNode.toMap();
        int releaseVersion = mapReleases["tag_name"].toInt();
        if (releaseVersion <= latestVersion || releaseVersion >= 500)
            continue;
        for (const QVariant &assetNode : mapReleases["assets"].toList())
        {
            QMap<QString, QVariant> mapAssets = assetNode.toMap();
            QString assetState = mapAssets["state"].toString();
            if (assetState != "uploaded")
                continue;
            QString assetName = mapAssets["name"].toString();
            if (assetName.startsWith(pattern))
            {
                latestVersion = releaseVersion;
                listLateterReleases.append(releaseNode);
                break;
            }
        }
    }

    int latestRelease = 0;
    QString downLoadUrl;
    for (const QVariant &releaseNode : listLateterReleases)
    {
        QMap<QString, QVariant> mapReleases = releaseNode.toMap();
        int releaseVersion = mapReleases["tag_name"].toInt();
        if (releaseVersion < latestVersion)
            continue;
        latestRelease = releaseVersion;
        for (const QVariant &assetNode : mapReleases["assets"].toList())
        {
            QMap<QString, QVariant> mapAssets = assetNode.toMap();
            QString assetName = mapAssets["name"].toString();
            if (assetName.startsWith(pattern))
            {
                downLoadUrl = mapAssets["browser_download_url"].toString();
                break;
            }
        }
    }

    if (downLoadUrl != "")
    {
        int result = QMessageBox::information(parentWindow->window(), "Updater",
                                              "New version of MEM is available: " +
                                              QString::number(latestRelease),
                                              "Download and unpack",
                                              "Download with browser", "Skip it");
        if (result == 1)
        {
            QDesktopServices::openUrl(QUrl(downLoadUrl));
        }
        else if (result == 0)
        {
            downloadRelease(downLoadUrl);
        }
    }
}

void Updater::processUpdate()
{
    QNetworkRequest request;
    request.setUrl(QUrl("https://api.github.com/repos/MassEffectModder/MassEffectModder/releases"));
    request.setRawHeader("Accept", "application/vnd.github.v3+json");
    request.setRawHeader("User-Agent", "MEM");
    auto reply = managerListReleases.get(request);
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
}
