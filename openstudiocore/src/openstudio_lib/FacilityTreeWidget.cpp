/**********************************************************************
*  Copyright (c) 2008-2014, Alliance for Sustainable Energy.
*  All rights reserved.
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**********************************************************************/

#include "FacilityTreeWidget.hpp"
#include "ModelObjectTreeItems.hpp"
#include "ModelObjectItem.hpp"

#include "../model/Model.hpp"
#include "../model/Model_Impl.hpp"
#include "../model/Facility.hpp"
#include "../model/Building.hpp"
#include "../model/Building_Impl.hpp"
#include "../model/BuildingStory.hpp"
#include "../model/BuildingStory_Impl.hpp"
#include "../model/ThermalZone.hpp"
#include "../model/ThermalZone_Impl.hpp"
#include "../model/SpaceType.hpp"
#include "../model/SpaceType_Impl.hpp"
#include "../model/Space.hpp"
#include "../model/Surface.hpp"
#include "../model/SubSurface.hpp"
#include "../model/ShadingSurfaceGroup.hpp"
#include "../model/ShadingSurface.hpp"
#include "../model/InteriorPartitionSurface.hpp"
#include "../model/InteriorPartitionSurfaceGroup.hpp"
#include "../model/DaylightingControl.hpp"
#include "../model/IlluminanceMap.hpp"
#include "../model/InternalMass.hpp"
#include "../model/People.hpp"
#include "../model/Lights.hpp"
#include "../model/Luminaire.hpp"
#include "../model/ElectricEquipment.hpp"
#include "../model/GasEquipment.hpp"
#include "../model/SteamEquipment.hpp"
#include "../model/OtherEquipment.hpp"
#include "../model/SpaceInfiltrationDesignFlowRate.hpp"
#include "../model/SpaceInfiltrationEffectiveLeakageArea.hpp"
#include "../model/DesignSpecificationOutdoorAir.hpp"
#include "../model/Facility.hpp"

#include "../utilities/core/Assert.hpp"

#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QScrollArea>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QButtonGroup>
#include <QResizeEvent>
#include <QTimer>
#include <QMouseEvent>
#include <QCalendarWidget>
#include <QGridLayout>
#include <QRadioButton>
#include <QHeaderView>
#include <QComboBox>

#include <iostream>

namespace openstudio {

FacilityTreeWidget::FacilityTreeWidget(const model::Model& model, QWidget* parent )
  : ModelObjectTreeWidget(model, parent), m_sortByType(model::BuildingStory::iddObjectType())
{ 
  this->setObjectName("GrayWidget"); 

  QVBoxLayout* vLayout = new QVBoxLayout();
  vLayout->setContentsMargins(0,0,0,0);

  QHBoxLayout* hLayout = new QHBoxLayout();
  hLayout->setContentsMargins(10,0,10,0);

  QLabel* comboLabel = new QLabel();
  comboLabel->setText("Sort Building by: ");
  comboLabel->setStyleSheet("QLabel { font: bold; }");
  hLayout->addWidget(comboLabel);

  QComboBox* comboBox = new QComboBox();
  comboBox->addItem("Building Story");
  comboBox->addItem("Thermal Zone");
  comboBox->addItem("Space Type");
  hLayout->addWidget(comboBox);

  vLayout->addLayout(hLayout);

  bool isConnected = false;

  isConnected = connect(comboBox, SIGNAL(currentIndexChanged(const QString&)),
                        this, SLOT(onSortByChanged(const QString&)));
  OS_ASSERT(isConnected);

  this->vLayout()->insertLayout(0, vLayout);
  this->treeWidget()->header()->close();
  this->treeWidget()->setMinimumWidth(320);
  this->treeWidget()->setIndentation(10);
  this->treeWidget()->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->treeWidget()->setSelectionMode(QAbstractItemView::SingleSelection);
  this->treeWidget()->setAlternatingRowColors(true);
  this->treeWidget()->setColumnCount(1);

  isConnected = connect(this->treeWidget(), SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
                        this, SLOT(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
  OS_ASSERT(isConnected);

  // allow time for signals to be connected between this and inspector
  QTimer::singleShot(0, this, SLOT(initialize()));
}

IddObjectType FacilityTreeWidget::currentIddObjectType() const
{
  return m_iddObjectType;
}

boost::optional<openstudio::model::ModelObject> FacilityTreeWidget::selectedModelObject() const
{
  return m_selectedObject;
}

void FacilityTreeWidget::onObjectAdded(const openstudio::model::ModelObject& modelObject, const openstudio::IddObjectType& iddObjectType, const openstudio::Handle& handle)
{
  if ((iddObjectType == model::Facility::iddObjectType()) ||
      (iddObjectType == model::ShadingSurfaceGroup::iddObjectType()) ||
      (iddObjectType == model::ShadingSurface::iddObjectType()) ||
      (iddObjectType == model::Building::iddObjectType()) ||
      (iddObjectType == model::BuildingStory::iddObjectType()) ||
      (iddObjectType == model::ThermalZone::iddObjectType()) ||
      (iddObjectType == model::SpaceType::iddObjectType()) ||
      (iddObjectType == model::Space::iddObjectType()) ||
      (iddObjectType == model::InteriorPartitionSurfaceGroup::iddObjectType()) ||
      (iddObjectType == model::InteriorPartitionSurface::iddObjectType()) ||
      (iddObjectType == model::DaylightingControl::iddObjectType()) ||
      (iddObjectType == model::IlluminanceMap::iddObjectType()) ||
      (iddObjectType == model::InternalMass::iddObjectType()) ||
      (iddObjectType == model::People::iddObjectType()) ||
      (iddObjectType == model::Lights::iddObjectType()) ||
      (iddObjectType == model::Luminaire::iddObjectType()) ||
      (iddObjectType == model::ElectricEquipment::iddObjectType()) ||
      (iddObjectType == model::GasEquipment::iddObjectType()) ||
      (iddObjectType == model::SteamEquipment::iddObjectType()) ||
      (iddObjectType == model::OtherEquipment::iddObjectType()) ||
      (iddObjectType == model::SpaceInfiltrationDesignFlowRate::iddObjectType()) ||
      (iddObjectType == model::SpaceInfiltrationEffectiveLeakageArea::iddObjectType()) ||
      (iddObjectType == model::DesignSpecificationOutdoorAir::iddObjectType())){

    refresh();
  }
}

void FacilityTreeWidget::onObjectRemoved(const openstudio::model::ModelObject& modelObject, const openstudio::IddObjectType& iddObjectType, const openstudio::Handle& handle)
{
  if ((iddObjectType == model::Facility::iddObjectType()) ||
      (iddObjectType == model::ShadingSurfaceGroup::iddObjectType()) ||
      (iddObjectType == model::ShadingSurface::iddObjectType()) ||
      (iddObjectType == model::Building::iddObjectType()) ||
      (iddObjectType == model::BuildingStory::iddObjectType()) ||
      (iddObjectType == model::ThermalZone::iddObjectType()) ||
      (iddObjectType == model::SpaceType::iddObjectType()) ||
      (iddObjectType == model::Space::iddObjectType()) ||
      (iddObjectType == model::InteriorPartitionSurfaceGroup::iddObjectType()) ||
      (iddObjectType == model::InteriorPartitionSurface::iddObjectType()) ||
      (iddObjectType == model::DaylightingControl::iddObjectType()) ||
      (iddObjectType == model::IlluminanceMap::iddObjectType()) ||
      (iddObjectType == model::InternalMass::iddObjectType()) ||
      (iddObjectType == model::People::iddObjectType()) ||
      (iddObjectType == model::Lights::iddObjectType()) ||
      (iddObjectType == model::Luminaire::iddObjectType()) ||
      (iddObjectType == model::ElectricEquipment::iddObjectType()) ||
      (iddObjectType == model::GasEquipment::iddObjectType()) ||
      (iddObjectType == model::SteamEquipment::iddObjectType()) ||
      (iddObjectType == model::OtherEquipment::iddObjectType()) ||
      (iddObjectType == model::SpaceInfiltrationDesignFlowRate::iddObjectType()) ||
      (iddObjectType == model::SpaceInfiltrationEffectiveLeakageArea::iddObjectType()) ||
      (iddObjectType == model::DesignSpecificationOutdoorAir::iddObjectType())){

    refresh();
  }
}

void FacilityTreeWidget::paintEvent ( QPaintEvent * event )
{
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void FacilityTreeWidget::onSortByChanged(const QString& text)
{
  if (text == "Building Story"){
    if (m_sortByType != model::BuildingStory::iddObjectType()){
      m_sortByType = model::BuildingStory::iddObjectType();
      this->treeWidget()->clear();
      QTimer::singleShot(0, this, SLOT(initialize()));
    }
  }else if (text == "Thermal Zone"){
    if (m_sortByType != model::ThermalZone::iddObjectType()){
      m_sortByType = model::ThermalZone::iddObjectType();
      this->treeWidget()->clear();
      QTimer::singleShot(0, this, SLOT(initialize()));
    }
  }else if (text == "Space Type"){
    if (m_sortByType != model::SpaceType::iddObjectType()){
      m_sortByType = model::SpaceType::iddObjectType();
      this->treeWidget()->clear();
      QTimer::singleShot(0, this, SLOT(initialize()));
    }
  }else{
    OS_ASSERT(false);
  }
}

void FacilityTreeWidget::currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
  QTreeWidget* treeWidget = this->treeWidget();

  ModelObjectTreeItem* modelObjectTreeItem = dynamic_cast<ModelObjectTreeItem*>(current);
  if(modelObjectTreeItem){
    boost::optional<model::ModelObject> modelObject = modelObjectTreeItem->modelObject();
    if (modelObject){
      if (!modelObject->handle().isNull()){
        if (m_selectedObjectHandle != modelObject->handle()){
          m_selectedObject = modelObject;
          m_selectedObjectHandle = modelObject->handle();
          emit itemSelected(modelObjectTreeItem->item());
        }
      }
    }else{
      current->setSelected(false);
      if (previous){
        bool wasBlocked = treeWidget->blockSignals(true);
        treeWidget->setCurrentItem(previous);
        previous->setSelected(true);
        treeWidget->blockSignals(wasBlocked);
      }
    }
  }
}

void FacilityTreeWidget::initialize()
{
  openstudio::model::Model model = this->model();
  model::Building building = model.getUniqueModelObject<model::Building>();
  QTreeWidget* treeWidget = this->treeWidget();

  // add site shading
  SiteShadingTreeItem* siteShadingTreeItem = new SiteShadingTreeItem(model);
  treeWidget->addTopLevelItem(siteShadingTreeItem);

  // add building
  BuildingTreeItem* buildingTreeItem = new BuildingTreeItem(building, m_sortByType);
  treeWidget->addTopLevelItem(buildingTreeItem);

  // set the selected object
  treeWidget->setCurrentItem(buildingTreeItem);
  m_selectedObject = building;
  m_selectedObjectHandle = building.handle();
  emit itemSelected(buildingTreeItem->item());
}

} // openstudio

