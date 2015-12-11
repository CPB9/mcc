/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "DevicesListWidget.h"

#include <QPropertyAnimation>

#include <QMenu>

namespace mcc {
    namespace ui {
        namespace flightcontrols {

            DevicesListWidget::DevicesListWidget(QWidget *parent /*= 0*/)
                : QTreeWidget(parent)
            {
                 connect(this, &QTreeWidget::itemClicked, [this](QTreeWidgetItem* current, int column)
                    {
                        if (current->parent() == nullptr)
                            return;

                        auto device = current->data(0, Qt::UserRole).value<mcc::ui::core::FlyingDevice*>();
                        emit deviceSelected(device);
                    }
                 );

                 connect(this, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem* current, int column)
                 {
                     if (current->parent() == nullptr)
                         return;

                     auto device = current->data(0, Qt::UserRole).value<mcc::ui::core::FlyingDevice*>();
                     emit deviceCentered(device);
                 }
                 );

                setHeaderHidden(true);
                setIndentation(5);

                _ungroupedDevicesNode = new QTreeWidgetItem(QStringList() << "Одиночные устройства");

                addTopLevelItem(_ungroupedDevicesNode);

                _ungroupedDevicesNode->setExpanded(true);
                setSelectionMode(QAbstractItemView::ExtendedSelection);

                setContextMenuPolicy(Qt::CustomContextMenu);

                connect(this, &QTreeWidget::customContextMenuRequested, this, &DevicesListWidget::createContextMenu);
            }

            DevicesListWidget::~DevicesListWidget()
            {

            }

            void DevicesListWidget::addDevice(mcc::ui::core::FlyingDevice* aircraft)
            {
                addDeviceWidget(aircraft, _ungroupedDevicesNode);
                _manager->selectDevice(aircraft);
            }

            void DevicesListWidget::setDeviceManager(mcc::ui::core::DeviceManager* manager)
            {
                using mcc::ui::core::DeviceManager;

                _manager = manager;

                connect(_manager, &DeviceManager::deviceAdded,        this, &DevicesListWidget::addDevice);
                connect(_manager, &DeviceManager::deviceRemoved,      this, &DevicesListWidget::removeDevice);
                connect(_manager, &DeviceManager::deviceSignalGood,   this, &DevicesListWidget::deviceSignalGood);
                connect(_manager, &DeviceManager::deviceSignalBad,    this, &DevicesListWidget::deviceSignalBad);

                connect(_manager, &DeviceManager::groupAdded, this, &DevicesListWidget::addGroup);
                connect(_manager, &DeviceManager::groupRemoved, this, &DevicesListWidget::removeGroup);
                connect(_manager, &DeviceManager::selectionChanged, this, &DevicesListWidget::selectDevice);

                connect(this, &DevicesListWidget::requestDeviceActivate, _manager, &DeviceManager::requestDeviceActivate);
            }

            void DevicesListWidget::deviceSignalGood(mcc::ui::core::FlyingDevice* device)
            {
//                 widget->setEnabled(true);
            }

            void DevicesListWidget::deviceSignalBad(mcc::ui::core::FlyingDevice* device)
            {
//                widget->setEnabled(false);
            }

            void DevicesListWidget::selectDevice(mcc::ui::core::FlyingDevice* device)
            {
                QTreeWidgetItem* item = nullptr;
                 for (int i = 0; i < _ungroupedDevicesNode->childCount(); ++i)
                 {
                     DeviceStateWidget* currentDevice = static_cast<DeviceStateWidget*>(this->itemWidget(_ungroupedDevicesNode->child(i), 0));

                     if (currentDevice->device() == device)
                     {
                         item = _ungroupedDevicesNode->child(i);
                         break;
                     }
                 }

                 if (item)
                 {
                     setCurrentItem(item, 0, QItemSelectionModel::SelectionFlag::ClearAndSelect);
                 }
            }

            void DevicesListWidget::removeDevice(mcc::ui::core::FlyingDevice* aircraft)
            {
                removeDeviceWidget(aircraft, _ungroupedDevicesNode);
            }

            void DevicesListWidget::createContextMenu(const QPoint & pos)
            {
                QTreeWidgetItem* clickedItem = itemAt(pos);

                auto selectedItems = this->selectedItems();
                if (selectedItems.empty() && clickedItem == nullptr)
                    return;

                bool ifGroupClicked = (clickedItem->parent() == nullptr);

                QMenu menu(this);

                if (ifGroupClicked && clickedItem != _ungroupedDevicesNode)
                {
                    auto action = menu.addAction("Удалить группу");
                    connect(action, &QAction::triggered, this,
                        [this, clickedItem]()
                        {
                            auto group = clickedItem->data(0, Qt::UserRole).value<mcc::ui::core::DevicesGroup*>();
                            _manager->removeGroup(group);
                        }
                    );
                }
                else
                {
                    auto addGroupAction = menu.addAction("Создать группу");
                    connect(addGroupAction, &QAction::triggered, this,
                        [this] ()
                        {
                            auto group = _manager->addGroup("Тестовая группа");
                            auto selectedItems = this->selectedItems();
                            for (auto item : selectedItems)
                            {
                                auto device = item->data(0, Qt::UserRole).value<mcc::ui::core::FlyingDevice*>();

                                if (device != nullptr)
                                {
                                    group->addDevice(device);
                                }
                            }
                        }
                    );

                    if (clickedItem->parent() != _ungroupedDevicesNode)
                    {
                        auto removeDeviceAction = menu.addAction("Удалить из группы");
                        connect(removeDeviceAction, &QAction::triggered, this,
                            [this, clickedItem]()
                        {
                            auto group = clickedItem->parent()->data(0, Qt::UserRole).value<mcc::ui::core::DevicesGroup*>();

                            auto selectedItems = this->selectedItems();
                            for (auto item : selectedItems)
                            {
                                auto device = item->data(0, Qt::UserRole).value<mcc::ui::core::FlyingDevice*>();

                                if (device != nullptr)
                                {
                                    group->removeDevice(device);
                                }
                            }
                        }
                        );
                    }

                    menu.addSeparator();

                    auto unregisterDeviceAction = menu.addAction("Удалить устройство");
                    connect(unregisterDeviceAction, &QAction::triggered, this,
                        [this, clickedItem]()
                        {
                            auto selectedItems = this->selectedItems();
                            for (auto item : selectedItems)
                            {
                                auto device = item->data(0, Qt::UserRole).value<mcc::ui::core::FlyingDevice*>();

                                if (device != nullptr)
                                {
                                    emit _manager->requestDeviceUnregister(device->name());
                                }
                            }
                        }
                    );
                }

                menu.exec(mapToGlobal(pos));
            }

            void DevicesListWidget::addGroup(mcc::ui::core::DevicesGroup* group)
            {
                QTreeWidgetItem* node = new QTreeWidgetItem(QStringList() << group->name());
                node->setData(0, Qt::UserRole, QVariant::fromValue(group));
                addTopLevelItem(node);
                node->setExpanded(true);
                connect(group, &mcc::ui::core::DevicesGroup::deviceAdded, this,
                    [this, node](mcc::ui::core::FlyingDevice* device)
                    {
                        addDeviceWidget(device, node);
                    }
                );

                connect(group, &mcc::ui::core::DevicesGroup::deviceRemoved, this,
                    [this, node](mcc::ui::core::FlyingDevice* device)
                {
                    removeDeviceWidget(device, node);
                }
                );
            }

            void DevicesListWidget::removeGroup(mcc::ui::core::DevicesGroup* group)
            {
                if (group == nullptr)
                    return;

                QList<QTreeWidgetItem*> toDelete;

                for (int i = 0; i < topLevelItemCount(); ++i)
                {
                    QTreeWidgetItem* parentItem = topLevelItem(i);
                    if (parentItem->data(0, Qt::UserRole).value<mcc::ui::core::DevicesGroup*>() == group)
                    {
                        toDelete.append(parentItem);
                    }
                }

                for (auto del : toDelete)
                {
                    delete del;
                }
            }

            void DevicesListWidget::addDeviceWidget(mcc::ui::core::FlyingDevice* device, QTreeWidgetItem* parent)
            {
                auto deviceName = device->name();
                auto deviceWidget = new DeviceStateWidget();

                deviceWidget->setDeviceName(deviceName);
                deviceWidget->setPixmap(device->pixmap());

                deviceWidget->setModel(device);
                auto listItem = new QTreeWidgetItem();
                listItem->setSizeHint(0, deviceWidget->sizeHint());
                listItem->setData(0, Qt::UserRole, QVariant::fromValue(device));

                parent->addChild(listItem);

                setItemWidget(listItem, 0, deviceWidget);

                connect(deviceWidget, &DeviceStateWidget::requestActivateDevice, this, &DevicesListWidget::requestDeviceActivate);
            }

            void DevicesListWidget::removeDeviceWidget(mcc::ui::core::FlyingDevice* device, QTreeWidgetItem* parent)
            {
                QList<QTreeWidgetItem*> toDelete;

                for (int i = 0; i < parent->childCount(); ++i)
                {
                    QTreeWidgetItem* item = parent->child(i);
                    if (item->data(0, Qt::UserRole).value<mcc::ui::core::FlyingDevice*>() == device)
                    {
                        toDelete.append(item);
                    }
                }

                for (auto del : toDelete)
                {
                    delete del;
                }
            }
        }
    }
}

