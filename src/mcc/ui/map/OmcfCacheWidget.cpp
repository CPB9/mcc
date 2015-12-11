/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/OmcfCacheWidget.h"
#include "mcc/ui/map/MercatorProjection.h"
#include "mcc/misc/Crc.h"

#include "bmcl/MemWriter.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QTextEdit>
#include <QProgressBar>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>

#include <thread>

namespace mcc {
namespace ui {
namespace map {

using mcc::misc::Option;
using mcc::misc::crc32;

static const uint32_t headerMagic = 0x5a5a5a5a;

struct NaturalSort {
    bool operator()(const QString& left, const QString& right)
    {
        if (left.size() == right.size()) {
            return left < right;
        }
        return left.size() < right.size();
    }
};

void OmcfCacheWidget::getFiles(int baseLen, QDir& startDir, OrderedTilePosCache<QString>& tilePaths, const char* format)
{
    QStringList files = startDir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::NoSort);
    std::sort(files.begin(), files.end(), NaturalSort());
    for (auto& file : files) {
        QString relPath = startDir.absoluteFilePath(file).mid(baseLen);
        Option<TilePosition> pos = FileCache::createPosition(relPath, format);
        if (pos.isSome()) {
            tilePaths.addValue(pos.unwrap(), relPath);
        }
    }
    QStringList dirs = startDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
    std::sort(dirs.begin(), dirs.end(), NaturalSort());
    for (const QString& dir : dirs) {
        startDir.cd(dir);
        getFiles(baseLen, startDir, tilePaths, format);
        startDir.cdUp();
    }
}

void OmcfCacheWidget::createCache(const QString& dirPath, const QString& outputPath, const QString& name,
                                  const QString& description, const QByteArray& format, const MercatorProjection& proj)
{
    OrderedTilePosCache<QString> tiles;
    QDir inputDir(dirPath);

    getFiles(inputDir.absolutePath().length() + 1, inputDir, tiles, format.constData());
    int count = tiles.count();

    QFile output(outputPath);
    if (!output.open(QIODevice::WriteOnly)) {
        return;
    }

    uint32_t headerSize = 4 + 4 + 2 * sizeof(double) + 4 + name.size() * sizeof(QChar) + 4
        + description.size() * sizeof(QChar) + 4 + count * (2 + 4 + 4 + 8 + 8) + 4;
    int64_t currentOffset = headerSize;
    double a = proj.majorAxis();
    double b = proj.minorAxis();
    output.seek(currentOffset);
    std::unique_ptr<uint8_t[]> data(new uint8_t[headerSize]);
    bmcl::MemWriter header(data.get(), headerSize);
    header.writeUint32Le(headerMagic);
    header.writeUint32Le(headerSize);
    header.write(&a, sizeof(double));
    header.write(&b, sizeof(double));
    header.writeUint32Le(name.size());
    header.write(name.constData(), name.size() * sizeof(QChar));
    header.writeUint32Le(description.size());
    header.write(description.constData(), description.size() * sizeof(QChar));
    header.writeUint32Le(count);
    auto writer = [this, &header, &inputDir, &output, &currentOffset, &count](const TilePosition& pos,
                                                                              const QString& path, std::size_t index) {
        QFile input(inputDir.absoluteFilePath(path));
        if (!input.open(QIODevice::ReadOnly)) {
            return;
        }

        QByteArray data = input.readAll();
        if (data.isNull()) {
            return;
        }
        int64_t fileSize = data.size();

        header.writeUint16Le(pos.zoomLevel);
        header.writeUint32Le(pos.globalOffsetX);
        header.writeUint32Le(pos.globalOffsetY);
        header.writeUint64Le(currentOffset);
        header.writeUint64Le(fileSize);
        currentOffset += fileSize;

        output.write(data);
        emit progressChanged(double(index + 1) / double(count) * 100);
    };
    tiles.map(writer);
    uint32_t crc = crc32(header.start(), header.sizeUsed());
    header.writeUint32Le(crc);
    assert(header.writableSize() == 0);
    output.seek(0);
    output.write((char*)header.start(), header.sizeUsed());
    emit finished();
}

OmcfCacheWidget::OmcfCacheWidget(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("Создание кеша карт");
    QLineEdit* openDirEdit = new QLineEdit;
    QLineEdit* saveFileEdit = new QLineEdit;
    QToolButton* browseDirButton = new QToolButton;
    browseDirButton->setText("...");
    QToolButton* browseFileButton = new QToolButton;
    browseFileButton->setText("...");
    QGridLayout* pathsLayout = new QGridLayout;
    pathsLayout->addWidget(new QLabel("Директория с кешем"), 0, 0);
    pathsLayout->addWidget(openDirEdit, 0, 1);
    pathsLayout->addWidget(browseDirButton, 0, 2);
    pathsLayout->addWidget(new QLabel("Файл для сохранения"), 1, 0);
    pathsLayout->addWidget(saveFileEdit, 1, 1);
    pathsLayout->addWidget(browseFileButton, 1, 2);

    QLineEdit* nameEdit = new QLineEdit;
    QComboBox* imageFormatBox = new QComboBox;
    imageFormatBox->addItems({"jpg", "jpeg", "png"});
    QComboBox* projectionBox = new QComboBox;
    projectionBox->addItem("Сферический меркатор");
    projectionBox->addItem("Эллиптический меркатор");

    QGroupBox* settingsBox = new QGroupBox("Параметры карты");
    QGridLayout* settingsLayout = new QGridLayout;
    settingsLayout->addWidget(new QLabel("Расширение изображений"), 0, 0);
    settingsLayout->addWidget(imageFormatBox, 0, 1);
    settingsLayout->addWidget(new QLabel("Проекция"), 1, 0);
    settingsLayout->addWidget(projectionBox, 1, 1);
    settingsLayout->addWidget(new QLabel("Название"), 2, 0);
    settingsLayout->addWidget(nameEdit, 2, 1);
    settingsBox->setLayout(settingsLayout);

    QGroupBox* descriptionBox = new QGroupBox("Описание");
    QTextEdit* descriptionEdit = new QTextEdit;
    QVBoxLayout* descriptionLayout = new QVBoxLayout;
    descriptionLayout->addWidget(descriptionEdit);
    descriptionBox->setLayout(descriptionLayout);

    QPushButton* createButton = new QPushButton("Создать");
    QPushButton* readSettingsButton = new QPushButton("Открыть файл конфигурации");
    createButton->setEnabled(false);
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(readSettingsButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(createButton);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(pathsLayout);
    mainLayout->addWidget(settingsBox);
    mainLayout->addWidget(descriptionBox);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    _progressBar = new QProgressBar;
    _progressBar->setMinimum(0);
    _progressBar->setMaximum(100);
    _progressBar->setWindowModality(Qt::ApplicationModal);

    auto enableCreateButton = [this, openDirEdit, saveFileEdit, createButton]() {
        bool isEnabled = !saveFileEdit->text().isEmpty();
        isEnabled &= !openDirEdit->text().isEmpty();
        createButton->setEnabled(isEnabled);
    };

    connect(openDirEdit, &QLineEdit::textChanged, this, enableCreateButton);
    connect(saveFileEdit, &QLineEdit::textChanged, this, enableCreateButton);

    connect(browseDirButton, &QPushButton::clicked, this, [this, openDirEdit]() {
        QString path = QFileDialog::getExistingDirectory(this, "Выбрать папку с картами");
        if (path.isEmpty()) {
            return;
        }
        openDirEdit->setText(path);
    });

    connect(browseFileButton, &QPushButton::clicked, this, [this, nameEdit, saveFileEdit]() {
        QString path = QFileDialog::getSaveFileName(this, "Выбрать файл для сохранения", "", "Кеш карт (*.omcf)");
        if (path.isEmpty()) {
            return;
        }
        if (nameEdit->text().isEmpty()) {
            nameEdit->setText(QFileInfo(path).baseName());
        }
        saveFileEdit->setText(path);
    });

    connect(readSettingsButton, &QPushButton::clicked, this, [this, nameEdit, projectionBox, imageFormatBox]() {
        QString path = QFileDialog::getOpenFileName(this, "Выбрать файл описания", "", "Файл описания (params.txt)");
        if (path.isEmpty()) {
            return;
        }
        QSettings reader(path, QSettings::IniFormat);
        reader.setIniCodec("Windows-1251");
        reader.beginGroup("PARAMS");
        QString name = reader.value("name").toString();
        nameEdit->setText(name);
        int projNum = reader.value("projection").toInt();
        projectionBox->setCurrentIndex(projNum - 1);
        QString ext = reader.value("Ext").toString();
        if (ext.size() < 2) {
            return;
        }
        imageFormatBox->setCurrentText(ext.mid(1, ext.size() - 1));
    });

    connect(this, &OmcfCacheWidget::progressChanged, _progressBar, &QProgressBar::setValue, Qt::QueuedConnection);
    connect(this, &OmcfCacheWidget::finished, _progressBar, &QProgressBar::hide, Qt::QueuedConnection);

    connect(createButton, &QPushButton::clicked, this,
            [this, openDirEdit, saveFileEdit, imageFormatBox, projectionBox, nameEdit, descriptionEdit]() {
                _progressBar->setValue(0);
                _progressBar->show();
                QString openDir = openDirEdit->text();
                QString saveFile = saveFileEdit->text();
                QString name = nameEdit->text();
                QString description = descriptionEdit->toPlainText();
                QByteArray format = imageFormatBox->currentText().toLocal8Bit();
                MercatorProjection proj;
                if (projectionBox->currentIndex() == 0) {
                    proj = MercatorProjection(MercatorProjection::SphericalMercator);
                } else {
                    proj = MercatorProjection(MercatorProjection::EllipticalMercator);
                }
                std::thread thread(&OmcfCacheWidget::createCache, this, openDir, saveFile, name, description,
                                   format.constData(), proj);
                thread.detach();
            });
}
}
}
}
