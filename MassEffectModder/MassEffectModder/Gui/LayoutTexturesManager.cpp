/*
 * MassEffectModder
 *
 * Copyright (C) 2019 Pawel Kolodziejski
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

#include <Gui/LayoutMeSelect.h>
#include <Gui/LayoutTexturesManager.h>
#include <Gui/MainWindow.h>
#include <Helpers/Exception.h>

LayoutTexturesManager::LayoutTexturesManager(MainWindow *window)
    : mainWindow(window)
{
    layoutId = MainWindow::kLayoutTexturesManager;

    listLeft = new QListWidget();
    listMiddle = new QListWidget();

    labelImage = new QLabel();
    labelImage->setAutoFillBackground(true);
    textRight = new QPlainTextEdit;
    listRight = new QListWidget;
    rightView = new QStackedWidget();
    rightView->addWidget(labelImage);
    rightView->addWidget(textRight);
    rightView->addWidget(listRight);

    splitter = new QSplitter();
    splitter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    splitter->addWidget(listLeft);
    splitter->addWidget(listMiddle);
    splitter->addWidget(rightView);

    buttonReplace = new QPushButton("Replace");
    buttonReplace->setMinimumWidth(kButtonMinWidth);
    buttonReplace->setMinimumHeight(kButtonMinHeight);
    auto ButtonFont = buttonReplace->font();
    ButtonFont.setPointSize(kFontSize);
    buttonReplace->setFont(ButtonFont);
    connect(buttonReplace, &QPushButton::clicked, this, &LayoutTexturesManager::ReplaceSelected);

    buttonReplaceConvert = new QPushButton("Replace (Force Convert)");
    buttonReplaceConvert->setMinimumWidth(kButtonMinWidth);
    buttonReplaceConvert->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonReplaceConvert->font();
    ButtonFont.setPointSize(kFontSize);
    buttonReplaceConvert->setFont(ButtonFont);
    connect(buttonReplaceConvert, &QPushButton::clicked, this, &LayoutTexturesManager::ReplaceConvertSelected);

    auto VerticalLayoutListReplace = new QVBoxLayout();
    VerticalLayoutListReplace->addWidget(buttonReplace);
    VerticalLayoutListReplace->addWidget(buttonReplaceConvert);
    VerticalLayoutListReplace->setAlignment(Qt::AlignTop);
    auto GroupBoxReplace = new QGroupBox("Replace Texture");
    GroupBoxReplace->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    GroupBoxReplace->setAlignment(Qt::AlignBottom);
    GroupBoxReplace->setLayout(VerticalLayoutListReplace);

    buttonExtractToDDS = new QPushButton("Extract to DDS file");
    buttonExtractToDDS->setMinimumWidth(kButtonMinWidth);
    buttonExtractToDDS->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonExtractToDDS->font();
    ButtonFont.setPointSize(kFontSize);
    buttonExtractToDDS->setFont(ButtonFont);
    connect(buttonExtractToDDS, &QPushButton::clicked, this, &LayoutTexturesManager::ExtractDDSSelected);

    buttonExtractToPNG = new QPushButton("Extract to PNG file");
    buttonExtractToPNG->setMinimumWidth(kButtonMinWidth);
    buttonExtractToPNG->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonExtractToPNG->font();
    ButtonFont.setPointSize(kFontSize);
    buttonExtractToPNG->setFont(ButtonFont);
    connect(buttonExtractToPNG, &QPushButton::clicked, this, &LayoutTexturesManager::ExtractPNGSelected);

    auto VerticalLayoutListExtract = new QVBoxLayout();
    VerticalLayoutListExtract->addWidget(buttonExtractToDDS);
    VerticalLayoutListExtract->addWidget(buttonExtractToPNG);
    VerticalLayoutListExtract->setAlignment(Qt::AlignTop);
    auto GroupBoxExtract = new QGroupBox("Extract Texture");
    GroupBoxExtract->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    GroupBoxExtract->setAlignment(Qt::AlignBottom);
    GroupBoxExtract->setLayout(VerticalLayoutListExtract);

    buttonViewImage = new QPushButton("View - Image");
    buttonViewImage->setMinimumWidth(kButtonMinWidth);
    buttonViewImage->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonViewImage->font();
    ButtonFont.setPointSize(kFontSize);
    buttonViewImage->setFont(ButtonFont);
    connect(buttonViewImage, &QPushButton::clicked, this, &LayoutTexturesManager::ViewImageSelected);

    buttonInfoSingle = new QPushButton("Info - Single");
    buttonInfoSingle->setMinimumWidth(kButtonMinWidth);
    buttonInfoSingle->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonInfoSingle->font();
    ButtonFont.setPointSize(kFontSize);
    buttonInfoSingle->setFont(ButtonFont);
    connect(buttonInfoSingle, &QPushButton::clicked, this, &LayoutTexturesManager::ViewSingleSelected);

    buttonInfoAll = new QPushButton("Info - All");
    buttonInfoAll->setMinimumWidth(kButtonMinWidth);
    buttonInfoAll->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonInfoAll->font();
    ButtonFont.setPointSize(kFontSize);
    buttonInfoAll->setFont(ButtonFont);
    connect(buttonInfoAll, &QPushButton::clicked, this, &LayoutTexturesManager::ViewMultiSelected);

    buttonPackageSingle = new QPushButton("Single Package");
    buttonPackageSingle->setMinimumWidth(kButtonMinWidth);
    buttonPackageSingle->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonPackageSingle->font();
    ButtonFont.setPointSize(kFontSize);
    buttonPackageSingle->setFont(ButtonFont);
    connect(buttonPackageSingle, &QPushButton::clicked, this, &LayoutTexturesManager::PackageSingleSelected);

    buttonPackageMulti = new QPushButton("Multi Package");
    buttonPackageMulti->setMinimumWidth(kButtonMinWidth);
    buttonPackageMulti->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonPackageMulti->font();
    ButtonFont.setPointSize(kFontSize);
    buttonPackageMulti->setFont(ButtonFont);
    connect(buttonPackageMulti, &QPushButton::clicked, this, &LayoutTexturesManager::PackageMutiSelected);

    auto VerticalLayoutListView = new QVBoxLayout();
    VerticalLayoutListView->addWidget(buttonViewImage);
    VerticalLayoutListView->addWidget(buttonInfoSingle);
    VerticalLayoutListView->addWidget(buttonInfoAll);
    VerticalLayoutListView->addWidget(buttonPackageSingle);
    VerticalLayoutListView->addWidget(buttonPackageMulti);
    VerticalLayoutListView->setAlignment(Qt::AlignTop);
    auto GroupBoxView = new QGroupBox("Right Panel View");
    GroupBoxView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    GroupBoxView->setAlignment(Qt::AlignBottom);
    GroupBoxView->setLayout(VerticalLayoutListView);

    buttonSearch = new QPushButton("Search Texture");
    buttonSearch->setMinimumWidth(kButtonMinWidth);
    buttonSearch->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonSearch->font();
    ButtonFont.setPointSize(kFontSize);
    buttonSearch->setFont(ButtonFont);
    connect(buttonSearch, &QPushButton::clicked, this, &LayoutTexturesManager::SearchSelected);

    buttonExit = new QPushButton("Exit Texture Manager");
    buttonExit->setMinimumWidth(kButtonMinWidth);
    buttonExit->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonExit->font();
    ButtonFont.setPointSize(kFontSize);
    buttonExit->setFont(ButtonFont);
    connect(buttonExit, &QPushButton::clicked, this, &LayoutTexturesManager::ExitSelected);

    auto VerticalLayoutListMisc = new QVBoxLayout();
    VerticalLayoutListMisc->addWidget(buttonSearch);
    VerticalLayoutListMisc->addWidget(buttonExit);
    VerticalLayoutListMisc->setAlignment(Qt::AlignTop);
    auto GroupBoxMisc = new QGroupBox("Miscellaneous");
    GroupBoxMisc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    GroupBoxMisc->setAlignment(Qt::AlignBottom);
    GroupBoxMisc->setLayout(VerticalLayoutListMisc);

    auto HorizontalLayoutList = new QHBoxLayout();
    HorizontalLayoutList->addWidget(GroupBoxReplace, 1);
    HorizontalLayoutList->addWidget(GroupBoxExtract, 1);
    HorizontalLayoutList->addWidget(GroupBoxView, 1);
    HorizontalLayoutList->addWidget(GroupBoxMisc, 1);
    auto WidgetBottom = new QWidget();
    WidgetBottom->setLayout(HorizontalLayoutList);

    auto VerticalLayout = new QVBoxLayout(this);
    VerticalLayout->addWidget(splitter);
    VerticalLayout->addWidget(WidgetBottom);

    UpdateGui();

    mainWindow->SetTitle("Texture Manager");
}

void LayoutTexturesManager::LockGui(bool enable)
{
    listLeft->setEnabled(enable);
    listMiddle->setEnabled(enable);
    listRight->setEnabled(enable);
    textRight->setEnabled(enable);
    rightView->setEnabled(enable);
    splitter->setEnabled(enable);
    buttonReplace->setEnabled(enable);
    buttonReplaceConvert->setEnabled(enable);
    buttonExtractToDDS->setEnabled(enable);
    buttonExtractToPNG->setEnabled(enable);
    buttonViewImage->setEnabled(enable);
    buttonInfoSingle->setEnabled(enable);
    buttonInfoAll->setEnabled(enable);
    buttonPackageSingle->setEnabled(enable);
    buttonPackageMulti->setEnabled(enable);
    buttonSearch->setEnabled(enable);
    buttonExit->setEnabled(enable);
    mainWindow->LockClose(enable);
}

void LayoutTexturesManager::UpdateGui()
{
    buttonReplace->setEnabled(textureSelected);
    buttonReplaceConvert->setEnabled(textureSelected);
    buttonExtractToDDS->setEnabled(textureSelected);
    buttonExtractToPNG->setEnabled(textureSelected);
    if (imageViewMode)
    {
        rightView->setCurrentIndex(kWidgetImage);
        buttonViewImage->setEnabled(false);
        buttonInfoSingle->setEnabled(packageSelected);
        buttonInfoAll->setEnabled(packageSelected);
        buttonPackageSingle->setEnabled(packageSelected);
        buttonPackageMulti->setEnabled(packageSelected);
    }
    else
    {
        if (singlePackageMode)
            rightView->setCurrentIndex(kWidgetImage);
        else
            rightView->setCurrentIndex(kWidgetText);
        buttonViewImage->setEnabled(packageSelected);
        buttonInfoSingle->setEnabled(!packageSelected);
        buttonInfoAll->setEnabled(!packageSelected);
        buttonPackageSingle->setEnabled(!packageSelected);
        buttonPackageMulti->setEnabled(!packageSelected);
    }
}

void LayoutTexturesManager::ReplaceSelected()
{
}

void LayoutTexturesManager::ReplaceConvertSelected()
{
}

void LayoutTexturesManager::ExtractDDSSelected()
{
}

void LayoutTexturesManager::ExtractPNGSelected()
{
}

void LayoutTexturesManager::ViewImageSelected()
{
}

void LayoutTexturesManager::ViewSingleSelected()
{
}

void LayoutTexturesManager::ViewMultiSelected()
{
}

void LayoutTexturesManager::PackageSingleSelected()
{
}

void LayoutTexturesManager::PackageMutiSelected()
{
}

void LayoutTexturesManager::SearchSelected()
{
}

void LayoutTexturesManager::ExitSelected()
{
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModules);
    mainWindow->GetLayout()->removeWidget(this);
    mainWindow->SetTitle("Modules Selection");
}
