/*
 * MassEffectModder
 *
 * Copyright (C) 2019-2022 Pawel Kolodziejski
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

#include <Gui/LayoutMain.h>
#include <Gui/MainWindow.h>
#include <Gui/MessageWindow.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <GameData/GameData.h>
#include <GameData/TOCFile.h>
#include <Misc/Misc.h>

void LayoutMain::CheckGameFilesSelected(MeType gameType)
{
    LockGui(true);

    mainWindow->statusBar()->showMessage("Detecting game data...");
    QApplication::processEvents();
    ConfigIni configIni{};
    g_GameData->Init(gameType, configIni, true);
    if (!Misc::CheckGamePath())
    {
        mainWindow->statusBar()->clearMessage();
        QMessageBox::critical(this, "No installation found", STR_GAME_DATA_NOT_FOUND);
        LockGui(false);
        return;
    }

    QString errors;
    QStringList modList;
    Resources resources;

    resources.loadMD5Tables();

    bool vanilla = Misc::checkGameFiles(gameType, resources,
                                        errors, modList,
                                        &LayoutMain::CheckCallback,
                                        mainWindow);

    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);
    if (modList.count() != 0)
    {
        PERROR("\n------- Detected mods --------\n");
        for (int l = 0; l < modList.count(); l++)
        {
            PERROR(modList[l] + "\n");
        }
        PERROR("------------------------------\n\n");
    }

    if (!vanilla)
    {
        PERROR("===========================================================================\n");
        PERROR("WARNING: The following files(s) are not vanilla or not recognized\n");
        PERROR("===========================================================================\n\n");
        PERROR(errors);
    }
    g_logs->BufferEnableErrors(false);

    mainWindow->statusBar()->clearMessage();

    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(mainWindow, "Checked game files", g_logs->BufferGetErrors());
    }
    else
    {
        QMessageBox::information(this, "Checking game", "All game files checked.");
    }

    LockGui(false);
}

void LayoutMain::CheckCallback(void *handle, int progress, const QString &stage)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Total progress: ") + QString::number(progress) + "% - " + stage);
    QApplication::processEvents();
}

void LayoutMain::ChangeGamePathSelected(MeType gameType)
{
    LockGui(true);

    mainWindow->statusBar()->showMessage("Detecting game data...");
    QApplication::processEvents();
    ConfigIni configIni{};
    g_GameData->Init(gameType, configIni, true);
    QString meGameString = QString::number(static_cast<int>(gameType));
    QString caption = "Please select the Mass Effect " + meGameString + " executable file";
    QString filter = "ME" + meGameString + " executable file (MassEffect" + meGameString + ".exe)";
    QString path, exeSuffix = "/Binaries/Win64";
    QFileDialog dialog = QFileDialog(this, caption, g_GameData->GamePath() + exeSuffix, filter);
    dialog.setFileMode(QFileDialog::ExistingFile);
    if (dialog.exec())
    {
        path = dialog.selectedFiles().first();
    }
    if (path.length() != 0 && QFile(path).exists())
    {
        bool properVersion = false;
#if defined(_WIN32)
        auto exeVersion = getVersionString(path);
        switch (gameType)
        {
        case MeType::ME1_TYPE:
            if (exeVersion == "2.0.0.48602")
                properVersion = true;
            break;
        case MeType::ME2_TYPE:
            if (exeVersion == "2.0.0.48602")
                properVersion = true;
            break;
        case MeType::ME3_TYPE:
            if (exeVersion == "2.0.0.48602")
                properVersion = true;
            break;
        case MeType::UNKNOWN_TYPE:
            CRASH();
        }
#else
        properVersion = true;
#endif
        if (properVersion)
        {
            path = DirName(DirName(DirName(DirName(DirName(path)))));
            QString key = QString("MELE");
#if defined(_WIN32)
            configIni.Write(key, QString(path).replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameDataPath");
#else
            configIni.Write(key, path, "GameDataPath");
#endif
            g_GameData->Init(gameType, configIni, true);
            QMessageBox::information(this, "Changing game path", "Game path changed to\n" + path);
        }
        else
        {
            QMessageBox::information(this, "Changing game path", "Game path NOT changed. The selected executable is not a supported version. Is this a Legendary Edition executable?");
        }
    }
    else
    {
        QMessageBox::information(this, "Changing game path", "Game path was NOT changed.");
    }
    mainWindow->statusBar()->clearMessage();
    LockGui(false);
}

void LayoutMain::UpdateTOCsSelected(MeType gameType)
{
    LockGui(true);

    mainWindow->statusBar()->showMessage("Detecting game data...");
    QApplication::processEvents();
    ConfigIni configIni{};
    g_GameData->Init(gameType, configIni, true);
    if (!Misc::CheckGamePath())
    {
        mainWindow->statusBar()->clearMessage();
        QMessageBox::critical(this, "Updating TOC files", STR_GAME_DATA_NOT_FOUND);
        LockGui(false);
        return;
    }

    if (!Misc::checkWriteAccessDir(g_GameData->MainData()))
    {
        mainWindow->statusBar()->clearMessage();
        QMessageBox::critical(this, "Updating TOC files",
                              QString("The current user does not have write access to the game folder.") +
              "\nGrant write access to the game directory and then try again.");
        LockGui(false);
        return;
    }

    TOCBinFile::UpdateAllTOCBinFiles(gameType);
    mainWindow->statusBar()->clearMessage();
    QMessageBox::information(this, "Updating TOC files", "All TOC files updated.");
    LockGui(false);
}
