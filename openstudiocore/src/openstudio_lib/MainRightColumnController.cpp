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

#include <openstudio_lib/MainRightColumnController.hpp>

#include <openstudio_lib/HorizontalTabWidget.hpp>
#include <openstudio_lib/ModelObjectTypeListView.hpp>
#include <openstudio_lib/InspectorController.hpp>
#include <openstudio_lib/InspectorView.hpp>
#include <openstudio_lib/OSDocument.hpp>
#include <openstudio_lib/OSAppBase.hpp>
#include <openstudio_lib/LocationTabController.hpp>
#include <openstudio_lib/SchedulesTabController.hpp>
#include <openstudio_lib/BuildingStoryInspectorView.hpp>
#include <openstudio_lib/SpaceTypeInspectorView.hpp>
#include <openstudio_lib/ThermalZonesView.hpp>
#include <openstudio_lib/OSItem.hpp>
#include <openstudio_lib/OSItemList.hpp>
#include <openstudio_lib/OSCollapsibleItem.hpp>
#include <openstudio_lib/OSCollapsibleItemHeader.hpp>
#include <openstudio_lib/ScriptFolderListView.hpp>
#include <openstudio_lib/ConstructionsTabController.hpp>

#include "../shared_gui_components/MeasureManager.hpp"
#include "../shared_gui_components/LocalLibraryController.hpp"
#include "../shared_gui_components/LocalLibraryView.hpp"
#include "../shared_gui_components/EditView.hpp"
#include "../shared_gui_components/EditController.hpp"
#include "../shared_gui_components/OSViewSwitcher.hpp"

#include <utilities/idd/IddEnums.hxx>
// TODO Avoid this include in favor of Foo_FieldEnums.hxx
// This is too large and will force a rebuild of this file 
// on all idd changes.
#include <utilities/idd/IddFieldEnums.hxx>

#include <utilities/core/Assert.hpp>

#include <QStackedWidget>
#include <QLayout>

namespace openstudio {

MainRightColumnController::MainRightColumnController(const model::Model & model,
    const openstudio::path &resourcesPath)
  : OSQObjectController(),
    m_model(model),
    m_resourcesPath(resourcesPath),
    m_measureLibraryController(new LocalLibraryController(OSAppBase::instance())),
    m_measureEditController(new EditController())
{
  m_measureLibraryController->localLibraryView->setStyleSheet("QStackedWidget { border-top: 0px; }");
  OSAppBase::instance()->measureManager().setLibraryController(m_measureLibraryController);
  OSAppBase::instance()->measureManager().updateMeasuresLists();
  m_horizontalTabWidget = new HorizontalTabWidget();
  addQObject(m_horizontalTabWidget);

  // My Model

  m_myModelView = new QStackedWidget();
  m_myModelView->setStyleSheet("QStackedWidget { border-top: 1px solid black; }");
  m_horizontalTabWidget->addTab(m_myModelView,MY_MODEL,"My Model");

  // Library

  m_libraryView = new QStackedWidget();
  m_libraryView->setStyleSheet("QStackedWidget { border-top: 1px solid black; }");
  m_horizontalTabWidget->addTab(m_libraryView,LIBRARY,"Library");


  // Editor 
  m_editView = new QStackedWidget();
  m_editView->setStyleSheet("QStackedWidget { border-top: 1px solid black; }");
  m_horizontalTabWidget->addTab(m_editView,EDIT,"Edit");

  // Inspector, we're keeping it around to be able to follow the units toggled
  m_inspectorController = boost::shared_ptr<InspectorController>( new InspectorController() );
  bool isConnected = connect(this, SIGNAL(toggleUnitsClicked(bool)),
                             m_inspectorController.get(), SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);
}

void MainRightColumnController::registerSystemItem(const Handle & systemHandle, SystemItem * systemItem)
{
  m_systemItemMap[systemHandle] = systemItem;
}

void MainRightColumnController::unregisterSystemItem(const Handle & systemHandle)
{
  std::map<Handle,SystemItem *>::iterator it = m_systemItemMap.find(systemHandle);
  if( it != m_systemItemMap.end() )
  {
    m_systemItemMap.erase(it);
  }
}

SystemItem * MainRightColumnController::systemItem(const Handle & systemHandle) const
{
  std::map<Handle,SystemItem *>::const_iterator it = m_systemItemMap.find(systemHandle);
  if( it != m_systemItemMap.end() )
  {
    return it->second;
  }

  return NULL;
}

void MainRightColumnController::inspectModelObject(model::OptionalModelObject & modelObject, bool readOnly)
{
  if( modelObject )
  {
    m_horizontalTabWidget->setCurrentId(EDIT);
    setEditView(m_inspectorController->inspectorView());
    m_inspectorController->layoutModelObject(modelObject, readOnly);
  }
  else
  {
    m_inspectorController->layoutModelObject(modelObject, readOnly);
  }
}

HorizontalTabWidget * MainRightColumnController::mainRightColumnView() const
{
  return m_horizontalTabWidget;
}

void MainRightColumnController::setEditView(QWidget *widget)
{
  if( QWidget * oldwidget = m_editView->currentWidget() )
  {
    LOG(Debug, "Removing old edit widget: " << oldwidget);
    m_editView->removeWidget(oldwidget);

    if (oldwidget != m_inspectorController->inspectorView()
        && oldwidget != m_measureEditController->editView.data())
    {
      LOG(Debug, "Deleting old edit widget: " << oldwidget);
      delete oldwidget;
    } else {
      boost::optional<model::ModelObject> nomodelobject;
      m_inspectorController->layoutModelObject(nomodelobject, false);
    }
  }

  if( widget )
  {
    LOG(Debug, "Setting new edit widget: " << widget);
    m_editView->addWidget(widget);
  }
}

void MainRightColumnController::setMyModelView(QWidget * widget)
{
  if( QWidget * oldwidget = m_myModelView->currentWidget() )
  {
    m_myModelView->removeWidget(oldwidget);

    delete oldwidget;
  }

  if( widget )
  {
    m_myModelView->addWidget(widget);
  }
}

void MainRightColumnController::setLibraryView(QWidget * widget)
{
  if( QWidget * oldwidget = m_libraryView->currentWidget() )
  {
    m_libraryView->removeWidget(oldwidget);

    if (oldwidget != m_measureLibraryController->localLibraryView)
    {
      delete oldwidget;
    }
  }

  if( widget )
  {
    m_libraryView->addWidget(widget);
  }
}

void MainRightColumnController::configureForSiteSubTab(int subTabID)
{
  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  setLibraryView(NULL);
  setMyModelView(NULL);
  setEditView(NULL);

  doc->openSidebar();
  //doc->closeSidebar();
}

void MainRightColumnController::configureForSchedulesSubTab(int subTabID)
{
  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  setLibraryView(NULL);
  setMyModelView(NULL);
  setEditView(NULL);

  switch( subTabID )
  {
    case SchedulesTabController::YEAR_SETTINGS:
    {
      doc->openSidebar();
      //doc->closeSidebar();

      break;
    }
    case SchedulesTabController::SCHEDULE_SETS:
    {
      //std::vector<std::pair<IddObjectType, std::string> > typeList;

      //typeList.push_back(std::make_pair(iddobjectname::OS_DefaultScheduleSet,"Default Schedule Sets"));

      //QWidget * myModelWidget = new ModelObjectTypeListView(typeList,m_model,true,OSItem::COLLAPSIBLE_LIST_HEADER);

      //setMyModelView(myModelWidget);

      model::Model lib = doc->componentLibrary();

      //QWidget * libraryWidget = new ModelObjectTypeListView(typeList,lib,true,OSItem::COLLAPSIBLE_LIST_HEADER);

      //setLibraryView(libraryWidget);


      // my model
      ModelObjectTypeListView* myModelList = new ModelObjectTypeListView(m_model, true, OSItem::COLLAPSIBLE_LIST_HEADER);
      myModelList->setItemsType(OSItem::LIBRARY_ITEM);
      myModelList->setItemsDraggable(true);
      myModelList->setItemsRemoveable(false);
      myModelList->setShowFilterLayout(true);

      myModelList->addModelObjectType(iddobjectname::OS_Schedule_VariableInterval, "Variable Interval Schedules");
      myModelList->addModelObjectType(iddobjectname::OS_Schedule_FixedInterval, "Fixed Interval Schedules");
      myModelList->addModelObjectType(iddobjectname::OS_Schedule_Constant, "Constant Schedules");
      myModelList->addModelObjectType(iddobjectname::OS_Schedule_Compact, "Compact Schedules");
      myModelList->addModelObjectType(iddobjectname::OS_Schedule_Ruleset, "Ruleset Schedules");

      setMyModelView(myModelList);

      // my library
      ModelObjectTypeListView* myLibraryList = new ModelObjectTypeListView(lib,true,OSItem::COLLAPSIBLE_LIST_HEADER);
      myLibraryList->setItemsDraggable(true);
      myLibraryList->setItemsRemoveable(false);
      myLibraryList->setItemsType(OSItem::LIBRARY_ITEM);

      myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_VariableInterval, "Variable Interval Schedules");
      myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_FixedInterval, "Fixed Interval Schedules");
      myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Constant, "Constant Schedules");
      myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Compact, "Compact Schedules");
      myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Ruleset, "Ruleset Schedules");
      myLibraryList->addModelObjectType(iddobjectname::OS_DefaultScheduleSet, "Schedule Sets");

      setLibraryView(myLibraryList);

      doc->openSidebar();
      break;
    }
    case SchedulesTabController::SCHEDULES:
    {
      model::Model lib = doc->componentLibrary();

      // my library
      ModelObjectTypeListView* myLibraryList = new ModelObjectTypeListView(lib,true,OSItem::COLLAPSIBLE_LIST_HEADER);
      myLibraryList->setItemsDraggable(true);
      myLibraryList->setItemsRemoveable(false);
      myLibraryList->setItemsType(OSItem::LIBRARY_ITEM);
      myLibraryList->setShowFilterLayout(true);

      myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Ruleset, "Schedule Rulesets");

      setLibraryView(myLibraryList);
      doc->openSidebar();
      //doc->closeSidebar();

      break;
    }
    default:
      break;
  }
}

void MainRightColumnController::configureForConstructionsSubTab(int subTabID)
{
  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  setLibraryView(NULL);
  setMyModelView(NULL);
  setEditView(NULL);

  switch( subTabID )
  {
    case ConstructionsTabController::DEFAULT_CONSTRUCTIONS:
    {
      model::Model lib = doc->componentLibrary();

      // my model
      ModelObjectTypeListView* myModelList = new ModelObjectTypeListView(m_model, true, OSItem::COLLAPSIBLE_LIST_HEADER);
      myModelList->setItemsDraggable(true);
      myModelList->setItemsRemoveable(false);
      myModelList->setItemsType(OSItem::LIBRARY_ITEM);
      myModelList->setShowFilterLayout(true);

      myModelList->addModelObjectType(iddobjectname::OS_Construction_WindowDataFile, "Window Data File Constructions");
      myModelList->addModelObjectType(iddobjectname::OS_Construction_FfactorGroundFloor, "F-factor Ground Floor Constructions");
      myModelList->addModelObjectType(iddobjectname::OS_Construction_CfactorUndergroundWall, "C-factor Underground Wall Constructions");
      myModelList->addModelObjectType(iddobjectname::OS_Construction_InternalSource, "Internal Source Constructions");
      myModelList->addModelObjectType(iddobjectname::OS_Construction, "Constructions");

      setMyModelView(myModelList);

      // my library
      ModelObjectTypeListView* myLibraryList = new ModelObjectTypeListView(lib,true,OSItem::COLLAPSIBLE_LIST_HEADER,true);
      myLibraryList->setItemsDraggable(true);
      myLibraryList->setItemsRemoveable(false);
      myLibraryList->setItemsType(OSItem::LIBRARY_ITEM);
      myLibraryList->setShowFilterLayout(true);

      myLibraryList->addModelObjectType(iddobjectname::OS_Construction_WindowDataFile, "Window Data File Constructions");
      myLibraryList->addModelObjectType(iddobjectname::OS_Construction_FfactorGroundFloor, "F-factor Ground Floor Constructions");
      myLibraryList->addModelObjectType(iddobjectname::OS_Construction_CfactorUndergroundWall, "C-factor Underground Wall Constructions");
      myLibraryList->addModelObjectType(iddobjectname::OS_Construction_InternalSource, "Internal Source Constructions");
      myLibraryList->addModelObjectType(iddobjectname::OS_Construction, "Constructions");
      myLibraryList->addModelObjectType(iddobjectname::OS_DefaultConstructionSet, "Default Construction Sets");

      setLibraryView(myLibraryList);

      doc->openSidebar();
      break;
    }
    case ConstructionsTabController::CONSTRUCTIONS:
    {
      model::Model lib = doc->componentLibrary();

      // my model
      ModelObjectTypeListView* myModelList = new ModelObjectTypeListView(m_model, true, OSItem::COLLAPSIBLE_LIST_HEADER);
      myModelList->setItemsDraggable(true);
      myModelList->setItemsRemoveable(false);
      myModelList->setItemsType(OSItem::LIBRARY_ITEM);
      myModelList->setShowFilterLayout(true);

      myModelList->addModelObjectType(iddobjectname::OS_WindowMaterial_GlazingGroup_Thermochromic, "Glazing Group Thermochromic Window Materials");
      myModelList->addModelObjectType(iddobjectname::OS_WindowMaterial_Glazing_RefractionExtinctionMethod, "Refraction Extinction Method Glazing Window Materials");
      myModelList->addModelObjectType(iddobjectname::OS_WindowMaterial_Shade, "Shade Window Materials");
      myModelList->addModelObjectType(iddobjectname::OS_WindowMaterial_Screen, "Screen Window Materials");
      myModelList->addModelObjectType(iddobjectname::OS_WindowMaterial_Blind, "Blind Window Materials");
      myModelList->addModelObjectType(iddobjectname::OS_WindowMaterial_GasMixture, "Gas Mixture Window Materials");
      myModelList->addModelObjectType(iddobjectname::OS_WindowMaterial_Gas, "Gas Window Materials");
      myModelList->addModelObjectType(iddobjectname::OS_WindowMaterial_Glazing, "Glazing Window Materials");
      myModelList->addModelObjectType(iddobjectname::OS_WindowMaterial_SimpleGlazingSystem, "Simple Glazing System Window Materials");

      myModelList->addModelObjectType(iddobjectname::OS_Material_RoofVegetation, "Roof Vegetation Materials");
      myModelList->addModelObjectType(iddobjectname::OS_Material_InfraredTransparent, "Infrared Transparent Materials");
      myModelList->addModelObjectType(iddobjectname::OS_Material_AirWall, "Air Wall Materials");
      myModelList->addModelObjectType(iddobjectname::OS_Material_AirGap, "Air Gap Materials");
      myModelList->addModelObjectType(iddobjectname::OS_Material_NoMass, "No Mass Materials");
      myModelList->addModelObjectType(iddobjectname::OS_Material, "Materials");

      setMyModelView(myModelList);

      // my library
      ModelObjectTypeListView* myLibraryList = new ModelObjectTypeListView(lib,true,OSItem::COLLAPSIBLE_LIST_HEADER, true);
      myLibraryList->setItemsDraggable(true);
      myLibraryList->setItemsRemoveable(false);
      myLibraryList->setItemsType(OSItem::LIBRARY_ITEM);
      myLibraryList->setShowFilterLayout(true);

      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_GlazingGroup_Thermochromic, "Glazing Group Thermochromic Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Glazing_RefractionExtinctionMethod, "Refraction Extinction Method Glazing Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Shade, "Shade Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Screen, "Screen Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Blind, "Blind Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_GasMixture, "Gas Mixture Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Gas, "Gas Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Glazing, "Glazing Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_SimpleGlazingSystem, "Simple Glazing System Window Materials");

      myLibraryList->addModelObjectType(iddobjectname::OS_Material_RoofVegetation, "Roof Vegetation Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_Material_InfraredTransparent, "Infrared Transparent Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_Material_AirWall, "Air Wall Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_Material_AirGap, "Air Gap Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_Material_NoMass, "No Mass Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_Material, "Materials");

      myLibraryList->addModelObjectType(iddobjectname::OS_Construction_WindowDataFile, "Window Data File Constructions");
      myLibraryList->addModelObjectType(iddobjectname::OS_Construction_FfactorGroundFloor, "F-factor Ground Floor Constructions");
      myLibraryList->addModelObjectType(iddobjectname::OS_Construction_CfactorUndergroundWall, "C-factor Underground Wall Constructions");
      myLibraryList->addModelObjectType(iddobjectname::OS_Construction_InternalSource, "Internal Source Constructions");
      myLibraryList->addModelObjectType(iddobjectname::OS_Construction, "Constructions");

      setLibraryView(myLibraryList);

      doc->openSidebar();
      break;
    }
    case ConstructionsTabController::MATERIALS:
    {
      model::Model lib = doc->componentLibrary();

      // my model
      ModelObjectTypeListView* myModelList = new ModelObjectTypeListView(m_model, true, OSItem::COLLAPSIBLE_LIST_HEADER);
      myModelList->setItemsDraggable(true);
      myModelList->setItemsRemoveable(false);
      myModelList->setItemsType(OSItem::LIBRARY_ITEM);
      myModelList->setShowFilterLayout(true);

      setMyModelView(myModelList);

      // my library
      ModelObjectTypeListView* myLibraryList = new ModelObjectTypeListView(lib,true,OSItem::COLLAPSIBLE_LIST_HEADER,true);
      myLibraryList->setItemsDraggable(true);
      myLibraryList->setItemsRemoveable(false);
      myLibraryList->setItemsType(OSItem::LIBRARY_ITEM);
      myLibraryList->setShowFilterLayout(true);

      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_GlazingGroup_Thermochromic, "Glazing Group Thermochromic Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Glazing_RefractionExtinctionMethod, "Refraction Extinction Method Glazing Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Shade, "Shade Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Screen, "Screen Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Blind, "Blind Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_GasMixture, "Gas Mixture Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Gas, "Gas Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_Glazing, "Glazing Window Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_WindowMaterial_SimpleGlazingSystem, "Simple Glazing System Window Materials");

      myLibraryList->addModelObjectType(iddobjectname::OS_Material_RoofVegetation, "Roof Vegetation Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_Material_InfraredTransparent, "Infrared Transparent Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_Material_AirWall, "Air Wall Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_Material_AirGap, "Air Gap Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_Material_NoMass, "No Mass Materials");
      myLibraryList->addModelObjectType(iddobjectname::OS_Material, "Materials");

      setLibraryView(myLibraryList);

      doc->openSidebar();
      break;
    }
    default:
      break;
  }
}

void MainRightColumnController::configureForLoadsSubTab(int subTabID)
{
  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  model::Model lib = doc->componentLibrary();


  setEditView(NULL);

  // my model

  ModelObjectTypeListView* myModelList = new ModelObjectTypeListView(m_model, true, OSItem::COLLAPSIBLE_LIST_HEADER);
  myModelList->setItemsDraggable(true);
  myModelList->setItemsRemoveable(false);
  myModelList->setItemsType(OSItem::LIBRARY_ITEM);
  myModelList->setShowFilterLayout(true);

  myModelList->addModelObjectType(iddobjectname::OS_Construction_WindowDataFile, "Window Data File Constructions");
  myModelList->addModelObjectType(iddobjectname::OS_Construction_FfactorGroundFloor, "F-factor Ground Floor Constructions");
  myModelList->addModelObjectType(iddobjectname::OS_Construction_CfactorUndergroundWall, "C-factor Underground Wall Constructions");
  myModelList->addModelObjectType(iddobjectname::OS_Construction_InternalSource, "Internal Source Constructions");
  myModelList->addModelObjectType(iddobjectname::OS_Construction, "Constructions");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_VariableInterval, "Variable Interval Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_FixedInterval, "Fixed Interval Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Constant, "Constant Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Compact, "Compact Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Ruleset, "Ruleset Schedules");

  setMyModelView(myModelList);

  // my library

  ModelObjectTypeListView* myLibraryList = new ModelObjectTypeListView(lib,true,OSItem::COLLAPSIBLE_LIST_HEADER, true);
  myLibraryList->setItemsDraggable(true);
  myLibraryList->setItemsRemoveable(false);
  myLibraryList->setItemsType(OSItem::LIBRARY_ITEM);
  myLibraryList->setShowFilterLayout(true);

  myLibraryList->addModelObjectType(iddobjectname::OS_Construction_WindowDataFile, "Window Data File Constructions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Construction_FfactorGroundFloor, "F-factor Ground Floor Constructions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Construction_CfactorUndergroundWall, "C-factor Underground Wall Constructions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Construction_InternalSource, "Internal Source Constructions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Construction, "Constructions");
  myLibraryList->addModelObjectType(iddobjectname::OS_InternalMass_Definition, "Internal Mass Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_OtherEquipment_Definition, "Other Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_SteamEquipment_Definition, "Steam Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_HotWaterEquipment_Definition, "Hot Water Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_GasEquipment_Definition, "Gas Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_ElectricEquipment_Definition, "Electric Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Luminaire_Definition, "Luminaire Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Lights_Definition, "Lights Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_People_Definition, "People Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_VariableInterval, "Variable Interval Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_FixedInterval, "Fixed Interval Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Constant, "Constant Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Compact, "Compact Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Ruleset, "Ruleset Schedules");

  setLibraryView(myLibraryList);

  doc->openSidebar();
}

void MainRightColumnController::configureForSpaceTypesSubTab(int subTabID)
{
  // no sub tabs
  OS_ASSERT(subTabID == -1);

  setEditView(NULL);

  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  // my model
  ModelObjectTypeListView* myModelList = new ModelObjectTypeListView(m_model, true, OSItem::COLLAPSIBLE_LIST_HEADER);
  myModelList->setItemsDraggable(true);
  myModelList->setItemsRemoveable(false);
  myModelList->setItemsType(OSItem::LIBRARY_ITEM);
  myModelList->setShowFilterLayout(true);

  myModelList->addModelObjectType(iddobjectname::OS_Schedule_VariableInterval, "Variable Interval Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_FixedInterval, "Fixed Interval Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Constant, "Constant Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Compact, "Compact Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Ruleset, "Ruleset Schedules");

  myModelList->addModelObjectType(iddobjectname::OS_InternalMass_Definition, "Internal Mass Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_OtherEquipment_Definition, "Other Equipment Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_SteamEquipment_Definition, "Steam Equipment Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_HotWaterEquipment_Definition, "Hot Water Equipment Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_GasEquipment_Definition, "Gas Equipment Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_ElectricEquipment_Definition, "Electric Equipment Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_Luminaire_Definition, "Luminaire Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_Lights_Definition, "Lights Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_People_Definition, "People Definitions");

  //OSCollapsibleItemHeader* unassignedSpacesCollapsibleHeader = new OSCollapsibleItemHeader("Unassigned Spaces", OSItemId("",""), OSItem::COLLAPSIBLE_LIST_HEADER);
  //unassignedSpacesCollapsibleHeader->setRemoveable(false);
  //SpaceTypeUnassignedSpacesVectorController* unassignedSpacesVectorController = new SpaceTypeUnassignedSpacesVectorController();
  //unassignedSpacesVectorController->attachModel(m_model);
  //OSItemList* unassignedSpacesList = new OSItemList(unassignedSpacesVectorController, false);
  //OSCollapsibleItem* unassignedSpacesCollapsibleItem = new OSCollapsibleItem(unassignedSpacesCollapsibleHeader, unassignedSpacesList);
  //myModelList->addCollapsibleItem(unassignedSpacesCollapsibleItem);

  //myModelList->addModelObjectType(iddobjectname::OS_SpaceInfiltration_DesignFlowRate, "Space Infiltration Design Flow Rates"); // do not show in my model because these are not shareable
  myModelList->addModelObjectType(iddobjectname::OS_DesignSpecification_OutdoorAir, "Design Specification Outdoor Air");
  myModelList->addModelObjectType(iddobjectname::OS_DefaultScheduleSet, "Default Schedule Sets");
  myModelList->addModelObjectType(iddobjectname::OS_DefaultConstructionSet, "Default Construction Sets");

  setMyModelView(myModelList);

  // my library
  model::Model lib = doc->componentLibrary();

  ModelObjectTypeListView* myLibraryList = new ModelObjectTypeListView(lib,true,OSItem::COLLAPSIBLE_LIST_HEADER,true);
  myLibraryList->setItemsDraggable(true);
  myLibraryList->setItemsRemoveable(false);
  myLibraryList->setItemsType(OSItem::LIBRARY_ITEM);
  myLibraryList->setShowFilterLayout(true);

  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_VariableInterval, "Variable Interval Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_FixedInterval, "Fixed Interval Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Constant, "Constant Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Compact, "Compact Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Ruleset, "Ruleset Schedules");

  myLibraryList->addModelObjectType(iddobjectname::OS_InternalMass_Definition, "Internal Mass Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_OtherEquipment_Definition, "Other Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_SteamEquipment_Definition, "Steam Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_HotWaterEquipment_Definition, "Hot Water Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_GasEquipment_Definition, "Gas Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_ElectricEquipment_Definition, "Electric Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Luminaire_Definition, "Luminaire Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Lights_Definition, "Lights Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_People_Definition, "People Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_SpaceInfiltration_DesignFlowRate, "Space Infiltration Design Flow Rates");
  myLibraryList->addModelObjectType(iddobjectname::OS_SpaceInfiltration_EffectiveLeakageArea, "Space Infiltration Effective Leakage Areas");
  myLibraryList->addModelObjectType(iddobjectname::OS_DesignSpecification_OutdoorAir, "Design Specification Outdoor Air");
  myLibraryList->addModelObjectType(iddobjectname::OS_DefaultScheduleSet, "Default Schedule Sets");
  myLibraryList->addModelObjectType(iddobjectname::OS_DefaultConstructionSet, "Default Construction Sets");
  myLibraryList->addModelObjectType(iddobjectname::OS_SpaceType, "Space Types");
  setLibraryView(myLibraryList);

  doc->openSidebar();
}

void MainRightColumnController::configureForBuildingStoriesSubTab(int subTabID)
{
  // no sub tabs
  OS_ASSERT(subTabID == -1);

  setEditView(NULL);

  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  // my model
  ModelObjectTypeListView* myModelList = new ModelObjectTypeListView(m_model, true, OSItem::COLLAPSIBLE_LIST_HEADER);
  myModelList->setItemsDraggable(true);
  myModelList->setItemsRemoveable(false);
  myModelList->setItemsType(OSItem::LIBRARY_ITEM);
  myModelList->setShowFilterLayout(true);

  myModelList->addModelObjectType(iddobjectname::OS_DefaultScheduleSet, "Default Schedule Sets");

  myModelList->addModelObjectType(iddobjectname::OS_DefaultConstructionSet, "Default Construction Sets");

  //OSCollapsibleItemHeader* unassignedSpacesCollapsibleHeader = new OSCollapsibleItemHeader("Unassigned Spaces", OSItemId("",""), OSItem::COLLAPSIBLE_LIST_HEADER);
  //unassignedSpacesCollapsibleHeader->setRemoveable(false);
  //BuildingStoryUnassignedSpacesVectorController* unassignedSpacesVectorController = new BuildingStoryUnassignedSpacesVectorController();
  //unassignedSpacesVectorController->attachModel(m_model);
  //OSItemList* unassignedSpacesList = new OSItemList(unassignedSpacesVectorController, false);
  //OSCollapsibleItem* unassignedSpacesCollapsibleItem = new OSCollapsibleItem(unassignedSpacesCollapsibleHeader, unassignedSpacesList);
  //myModelList->addCollapsibleItem(unassignedSpacesCollapsibleItem);

  setMyModelView(myModelList);

  // my library
  model::Model lib = doc->componentLibrary();

  ModelObjectTypeListView* myLibraryList = new ModelObjectTypeListView(lib,true,OSItem::COLLAPSIBLE_LIST_HEADER);
  myLibraryList->setItemsDraggable(true);
  myLibraryList->setItemsRemoveable(false);
  myLibraryList->setItemsType(OSItem::LIBRARY_ITEM);
  myLibraryList->setShowFilterLayout(true);

  myLibraryList->addModelObjectType(iddobjectname::OS_DefaultScheduleSet, "Default Schedule Sets");

  myLibraryList->addModelObjectType(iddobjectname::OS_DefaultConstructionSet, "Default Construction Sets");

  setLibraryView(myLibraryList);

  //doc->openSidebar();
  doc->closeSidebar();

}

void MainRightColumnController::configureForFacilitySubTab(int subTabID)
{
  // no sub tabs
  OS_ASSERT(subTabID == -1);

  setEditView(NULL);

  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  // my model
  ModelObjectTypeListView* myModelList = new ModelObjectTypeListView(m_model, true, OSItem::COLLAPSIBLE_LIST_HEADER);
  myModelList->setItemsDraggable(true);
  myModelList->setItemsRemoveable(false);
  myModelList->setItemsType(OSItem::LIBRARY_ITEM);
  myModelList->setShowFilterLayout(true);

  myModelList->addModelObjectType(iddobjectname::OS_SubSurface, "Sub Surfaces");
  myModelList->addModelObjectType(iddobjectname::OS_Surface, "Surfaces");
  myModelList->addModelObjectType(iddobjectname::OS_Construction_WindowDataFile, "Window Data File Constructions");
  myModelList->addModelObjectType(iddobjectname::OS_Construction_FfactorGroundFloor, "F-factor Ground Floor Constructions");
  myModelList->addModelObjectType(iddobjectname::OS_Construction_CfactorUndergroundWall, "C-factor Underground Wall Constructions");
  myModelList->addModelObjectType(iddobjectname::OS_Construction_InternalSource, "Internal Source Constructions");
  myModelList->addModelObjectType(iddobjectname::OS_Construction, "Constructions");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_VariableInterval, "Variable Interval Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_FixedInterval, "Fixed Interval Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Constant, "Constant Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Compact, "Compact Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Ruleset, "Ruleset Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_InternalMass_Definition, "Internal Mass Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_OtherEquipment_Definition, "Other Equipment Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_SteamEquipment_Definition, "Steam Equipment Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_HotWaterEquipment_Definition, "Hot Water Equipment Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_GasEquipment_Definition, "Gas Equipment Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_ElectricEquipment_Definition, "Electric Equipment Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_Luminaire_Definition, "Luminaire Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_Lights_Definition, "Lights Definitions");
  myModelList->addModelObjectType(iddobjectname::OS_People_Definition, "People Definitions");
  //myModelList->addModelObjectType(iddobjectname::OS_SpaceInfiltration_DesignFlowRate, "Space Infiltration Design Flow Rates"); // do not show in my model because these are not shareable
  myModelList->addModelObjectType(iddobjectname::OS_DesignSpecification_OutdoorAir, "Design Specification Outdoor Air");
  myModelList->addModelObjectType(iddobjectname::OS_DefaultScheduleSet, "Default Schedule Sets");
  myModelList->addModelObjectType(iddobjectname::OS_DefaultConstructionSet, "Default Construction Sets");
  myModelList->addModelObjectType(iddobjectname::OS_SpaceType, "Space Types");
  myModelList->addModelObjectType(iddobjectname::OS_ThermalZone, "Thermal Zones");
  myModelList->addModelObjectType(iddobjectname::OS_BuildingStory, "Building Stories");

  setMyModelView(myModelList);

  // Library
  model::Model lib = doc->combinedComponentLibrary();

  ModelObjectTypeListView* myLibraryList = new ModelObjectTypeListView(lib,true,OSItem::COLLAPSIBLE_LIST_HEADER,true);
  myLibraryList->setItemsDraggable(true);
  myLibraryList->setItemsRemoveable(false);
  myLibraryList->setItemsType(OSItem::LIBRARY_ITEM);
  myLibraryList->setShowFilterLayout(true);
  
  myLibraryList->addModelObjectType(iddobjectname::OS_ZoneHVAC_FourPipeFanCoil,"Four Pipe Fan Coil");
  myLibraryList->addModelObjectType(iddobjectname::OS_Fan_ZoneExhaust,"Fan Zone Exhaust");
  myLibraryList->addModelObjectType(iddobjectname::OS_ZoneHVAC_PackagedTerminalHeatPump,"PTHP");
  myLibraryList->addModelObjectType(iddobjectname::OS_ZoneHVAC_PackagedTerminalAirConditioner,"PTAC");
  myLibraryList->addModelObjectType(iddobjectname::OS_ZoneHVAC_WaterToAirHeatPump,"Water To Air HP");
  myLibraryList->addModelObjectType(iddobjectname::OS_ZoneHVAC_LowTemperatureRadiant_ConstantFlow,"Low Temp Radiant Constant Flow");
  myLibraryList->addModelObjectType(iddobjectname::OS_ZoneHVAC_LowTemperatureRadiant_VariableFlow,"Low Temp Radiant Variable Flow");  
  myLibraryList->addModelObjectType(iddobjectname::OS_ZoneHVAC_LowTemperatureRadiant_Electric,"Low Temp Radiant Electric");
  myLibraryList->addModelObjectType(iddobjectname::OS_Construction_WindowDataFile, "Window Data File Constructions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Construction_FfactorGroundFloor, "F-factor Ground Floor Constructions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Construction_CfactorUndergroundWall, "C-factor Underground Wall Constructions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Construction_InternalSource, "Internal Source Constructions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Construction, "Constructions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_VariableInterval, "Variable Interval Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_FixedInterval, "Fixed Interval Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Constant, "Constant Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Compact, "Compact Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_Schedule_Ruleset, "Ruleset Schedules");
  myLibraryList->addModelObjectType(iddobjectname::OS_InternalMass_Definition, "Internal Mass Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_OtherEquipment_Definition, "Other Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_SteamEquipment_Definition, "Steam Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_HotWaterEquipment_Definition, "Hot Water Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_GasEquipment_Definition, "Gas Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_ElectricEquipment_Definition, "Electric Equipment Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Luminaire_Definition, "Luminaire Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_Lights_Definition, "Lights Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_People_Definition, "People Definitions");
  myLibraryList->addModelObjectType(iddobjectname::OS_SpaceInfiltration_DesignFlowRate, "Space Infiltration Design Flow Rates");
  myLibraryList->addModelObjectType(iddobjectname::OS_SpaceInfiltration_EffectiveLeakageArea, "Space Infiltration Effective Leakage Areas");
  myLibraryList->addModelObjectType(iddobjectname::OS_DesignSpecification_OutdoorAir, "Design Specification Outdoor Air");
  myLibraryList->addModelObjectType(iddobjectname::OS_DefaultScheduleSet, "Default Schedule Sets");
  myLibraryList->addModelObjectType(iddobjectname::OS_DefaultConstructionSet, "Default Construction Sets");
  myLibraryList->addModelObjectType(iddobjectname::OS_SpaceType, "Space Types");
  //myLibraryList->addModelObjectType(iddobjectname::OS_AirTerminal_SingleDuct_ConstantVolume_CooledBeam, "Chilled Beam");

  setLibraryView(myLibraryList);

  doc->openSidebar();
  //doc->closeSidebar();

}

void MainRightColumnController::configureForThermalZonesSubTab(int subTabID)
{

  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  setLibraryView(NULL);
  setMyModelView(NULL);
  setEditView(NULL);

  // My Model

  ModelObjectTypeListView * myModelList = new ModelObjectTypeListView(m_model, true, OSItem::COLLAPSIBLE_LIST_HEADER);
  myModelList->setItemsDraggable(true);
  myModelList->setItemsRemoveable(false);
  myModelList->setItemsType(OSItem::LIBRARY_ITEM);
  myModelList->setShowFilterLayout(true);

  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Compact,"Compact Schedules");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Ruleset,"Schedule Rulesets");

  setMyModelView(myModelList);

  // Library
  model::Model lib = doc->combinedComponentLibrary();

  ModelObjectTypeListView * libraryWidget = new ModelObjectTypeListView(lib,true,OSItem::COLLAPSIBLE_LIST_HEADER);
  libraryWidget->setItemsDraggable(true);
  libraryWidget->setItemsRemoveable(false);
  libraryWidget->setItemsType(OSItem::LIBRARY_ITEM);
  libraryWidget->setShowFilterLayout(true);
 
  libraryWidget->addModelObjectType(iddobjectname::OS_ZoneHVAC_Baseboard_Convective_Electric,"Baseboard Convective Electric");
  libraryWidget->addModelObjectType(iddobjectname::OS_ZoneHVAC_Baseboard_Convective_Water,"Baseboard Convective Water");
  libraryWidget->addModelObjectType(iddobjectname::OS_ZoneHVAC_FourPipeFanCoil,"Four Pipe Fan Coil");
  libraryWidget->addModelObjectType(iddobjectname::OS_Fan_ZoneExhaust,"Fan Zone Exhaust");  
  libraryWidget->addModelObjectType(iddobjectname::OS_ZoneHVAC_PackagedTerminalHeatPump,"PTHP");
  libraryWidget->addModelObjectType(iddobjectname::OS_ZoneHVAC_WaterToAirHeatPump,"Water To Air HP");
  libraryWidget->addModelObjectType(iddobjectname::OS_ZoneHVAC_PackagedTerminalAirConditioner,"PTAC");
  libraryWidget->addModelObjectType(iddobjectname::OS_ZoneHVAC_LowTemperatureRadiant_ConstantFlow,"Low Temp Radiant Constant Flow");
  libraryWidget->addModelObjectType(iddobjectname::OS_ZoneHVAC_LowTemperatureRadiant_VariableFlow,"Low Temp Radiant Variable Flow");  
  libraryWidget->addModelObjectType(iddobjectname::OS_ZoneHVAC_LowTemperatureRadiant_Electric,"Low Temp Radiant Electric");
  libraryWidget->addModelObjectType(iddobjectname::OS_ZoneHVAC_UnitHeater,"Unit Heater");
  libraryWidget->addModelObjectType(iddobjectname::OS_Schedule_Compact,"Compact Schedules");
  libraryWidget->addModelObjectType(iddobjectname::OS_Schedule_Ruleset,"Schedule Rulesets");

  setLibraryView(libraryWidget);

  m_horizontalTabWidget->setCurrentId(LIBRARY);

  doc->openSidebar();
}

void MainRightColumnController::configureForHVACSystemsSubTab(int subTabID)
{

  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  setLibraryView(NULL);
  setMyModelView(NULL);
  setEditView(NULL);

  // my model
  ModelObjectTypeListView* myModelList = new ModelObjectTypeListView(m_model, true, OSItem::COLLAPSIBLE_LIST_HEADER);
  myModelList->setItemsDraggable(true);
  myModelList->setItemsRemoveable(false);
  myModelList->setItemsType(OSItem::LIBRARY_ITEM);
  myModelList->setShowFilterLayout(true);
  
  myModelList->addModelObjectType(iddobjectname::OS_WaterUse_Equipment_Definition,"Water Use Equipment Definition");  
  myModelList->addModelObjectType(iddobjectname::OS_WaterUse_Connections,"Water Use Connections");  
  myModelList->addModelObjectType(iddobjectname::OS_ThermalZone,"Thermal Zone");  
  myModelList->addModelObjectType(iddobjectname::OS_Refrigeration_System,"Refrigeration System");
  myModelList->addModelObjectType(iddobjectname::OS_Refrigeration_Condenser_WaterCooled,"Refrigeration Condenser Water Cooled");  
  myModelList->addModelObjectType(iddobjectname::OS_HeatExchanger_FluidToFluid,"Heat Exchanger Fluid To Fluid");
  myModelList->addModelObjectType(iddobjectname::OS_Coil_Heating_Water,"Coil Heating Water");
  myModelList->addModelObjectType(iddobjectname::OS_Coil_Cooling_Water,"Coil Cooling Water");
  myModelList->addModelObjectType(iddobjectname::OS_Chiller_Electric_EIR,"Chiller Electric EIR");
  myModelList->addModelObjectType(iddobjectname::OS_Schedule_Ruleset,"Schedules");

  setMyModelView(myModelList);

  // Library

  model::Model lib = doc->hvacComponentLibrary();

  ModelObjectTypeListView * libraryWidget = new ModelObjectTypeListView(lib,true,OSItem::COLLAPSIBLE_LIST_HEADER);
  libraryWidget->setItemsDraggable(true);
  libraryWidget->setItemsRemoveable(false);
  libraryWidget->setItemsType(OSItem::LIBRARY_ITEM);
  libraryWidget->setShowFilterLayout(true);

  libraryWidget->addModelObjectType(iddobjectname::OS_WaterUse_Equipment,"Water Use Equipment");
  libraryWidget->addModelObjectType(iddobjectname::OS_WaterUse_Connections,"Water Use Connections");
  libraryWidget->addModelObjectType(iddobjectname::OS_WaterHeater_Mixed,"Water Heater Mixed");
  libraryWidget->addModelObjectType(iddobjectname::OS_SetpointManager_SingleZone_Reheat,"Setpoint Manager Single Zone Reheat");
  libraryWidget->addModelObjectType(iddobjectname::OS_SetpointManager_Scheduled,"Setpoint Manager Scheduled");
  libraryWidget->addModelObjectType(iddobjectname::OS_SetpointManager_MixedAir,"Setpoint Manager Mixed Air");
  libraryWidget->addModelObjectType(iddobjectname::OS_SetpointManager_FollowOutdoorAirTemperature,"Setpoint Manager Follow Outdoor Air Temperature");
  libraryWidget->addModelObjectType(iddobjectname::OS_SetpointManager_OutdoorAirReset,"Setpoint Manager Outdoor Air Reset");
  libraryWidget->addModelObjectType(iddobjectname::OS_SetpointManager_Warmest,"Setpoint Manager Warmest");
  libraryWidget->addModelObjectType(iddobjectname::OS_Refrigeration_WalkIn,"Refrigeration Walkin");
  libraryWidget->addModelObjectType(iddobjectname::OS_Refrigeration_System,"Refrigeration System");
  libraryWidget->addModelObjectType(iddobjectname::OS_Refrigeration_Subcooler_Mechanical,"Refrigeration Subcooler Mechanical");
  libraryWidget->addModelObjectType(iddobjectname::OS_Refrigeration_Subcooler_LiquidSuction,"Refrigeration Subcooler Liquid Suction");
  libraryWidget->addModelObjectType(iddobjectname::OS_Refrigeration_Compressor,"Refrigeration Compressor");
  libraryWidget->addModelObjectType(iddobjectname::OS_Refrigeration_Condenser_Cascade,"Refrigeration Condenser Cascade");
  libraryWidget->addModelObjectType(iddobjectname::OS_Refrigeration_Condenser_WaterCooled,"Refrigeration Condenser Water Cooled");
  libraryWidget->addModelObjectType(iddobjectname::OS_Refrigeration_Condenser_EvaporativeCooled,"Refrigeration Condenser Evaporative Cooled");
  libraryWidget->addModelObjectType(iddobjectname::OS_Refrigeration_Condenser_AirCooled,"Refrigeration Condenser Air Cooled");
  libraryWidget->addModelObjectType(iddobjectname::OS_Refrigeration_Case,"Refrigeration Case");
  libraryWidget->addModelObjectType(iddobjectname::OS_Pump_ConstantSpeed,"Pump Constant Speed");
  libraryWidget->addModelObjectType(iddobjectname::OS_Pump_VariableSpeed,"Pump Variable Speed");
  libraryWidget->addModelObjectType(iddobjectname::OS_Pipe_Adiabatic, "Pipes");
  libraryWidget->addModelObjectType(iddobjectname::OS_HeatExchanger_FluidToFluid,"Heat Exchanger Fluid To Fluid");
  libraryWidget->addModelObjectType(iddobjectname::OS_HeatExchanger_AirToAir_SensibleAndLatent,"Heat Exchanger Air To Air Sensible and Latent");
  libraryWidget->addModelObjectType(iddobjectname::OS_Fan_VariableVolume,"Fan Variable Volume");
  libraryWidget->addModelObjectType(iddobjectname::OS_Fan_ConstantVolume,"Fan Constant Volume");
  libraryWidget->addModelObjectType(iddobjectname::OS_EvaporativeCooler_Direct_ResearchSpecial,"Evaporative Cooler Direct Research Special");
  libraryWidget->addModelObjectType(iddobjectname::OS_EvaporativeFluidCooler_SingleSpeed,"Evaporative Fluid Cooler SingleSpeed");
  libraryWidget->addModelObjectType(iddobjectname::OS_DistrictCooling,"District Cooling");
  libraryWidget->addModelObjectType(iddobjectname::OS_DistrictHeating,"District Heating");
  libraryWidget->addModelObjectType(iddobjectname::OS_GroundHeatExchanger_Vertical, "Vertical Ground Heat Exchanger");
  libraryWidget->addModelObjectType(iddobjectname::OS_CoolingTower_SingleSpeed, "Cooling Tower Single Speed");
  libraryWidget->addModelObjectType(iddobjectname::OS_CoolingTower_VariableSpeed, "Cooling Tower Variable Speed");
  libraryWidget->addModelObjectType(iddobjectname::OS_Chiller_Electric_EIR,"Chiller Electric EIR");
  libraryWidget->addModelObjectType(iddobjectname::OS_Coil_Heating_Gas,"Coil Heating Gas");
  libraryWidget->addModelObjectType(iddobjectname::OS_Coil_Heating_DX_SingleSpeed,"Coil Heating DX SingleSpeed");
  libraryWidget->addModelObjectType(iddobjectname::OS_Coil_Heating_Electric,"Coil Heating Electric");
  libraryWidget->addModelObjectType(iddobjectname::OS_Coil_Heating_Water,"Coil Heating Water");
  libraryWidget->addModelObjectType(iddobjectname::OS_Coil_Heating_WaterToAirHeatPump_EquationFit,"Coil Heating Water To Air HP");
  libraryWidget->addModelObjectType(iddobjectname::OS_Coil_Cooling_Water,"Coil Cooling Water");
  libraryWidget->addModelObjectType(iddobjectname::OS_Coil_Cooling_DX_TwoSpeed,"Coil Cooling DX TwoSpeed");
  libraryWidget->addModelObjectType(iddobjectname::OS_Coil_Cooling_DX_SingleSpeed,"Coil Cooling DX SingleSpeed");
  libraryWidget->addModelObjectType(iddobjectname::OS_Coil_Cooling_WaterToAirHeatPump_EquationFit,"Coil Cooling Water To Air HP");
  libraryWidget->addModelObjectType(iddobjectname::OS_Boiler_HotWater,"Boiler Hot Water");
  libraryWidget->addModelObjectType(iddobjectname::OS_AirTerminal_SingleDuct_ConstantVolume_CooledBeam, "Air Terminal Chilled Beam");
  libraryWidget->addModelObjectType(iddobjectname::OS_AirTerminal_SingleDuct_VAV_Reheat,"AirTerminal Single Duct VAV Reheat");
  libraryWidget->addModelObjectType(iddobjectname::OS_AirTerminal_SingleDuct_ConstantVolume_Reheat,"AirTerminal Single Duct Constant Volume Reheat");
  libraryWidget->addModelObjectType(iddobjectname::OS_AirTerminal_SingleDuct_VAV_Reheat,"AirTerminal Single Duct VAV Reheat");
  libraryWidget->addModelObjectType(iddobjectname::OS_AirTerminal_SingleDuct_Uncontrolled,"AirTerminal Single Duct Uncontrolled");
  libraryWidget->addModelObjectType(iddobjectname::OS_AirLoopHVAC_OutdoorAirSystem,"AirLoopHVAC Outdoor Air System");
  libraryWidget->addModelObjectType(iddobjectname::OS_AirTerminal_SingleDuct_VAV_NoReheat,"AirTerminal Single Duct VAV NoReheat");
  libraryWidget->addModelObjectType(iddobjectname::OS_AirLoopHVAC_UnitarySystem, "AirLoopHVAC Unitary System");
  libraryWidget->addModelObjectType(iddobjectname::OS_AirConditioner_VariableRefrigerantFlow,"VRF System");
  libraryWidget->addModelObjectType(iddobjectname::OS_ZoneHVAC_TerminalUnit_VariableRefrigerantFlow,"VRF Terminal");

  setLibraryView(libraryWidget);

  m_horizontalTabWidget->setCurrentId(LIBRARY);

  doc->openSidebar();
}

void MainRightColumnController::configureForBuildingSummarySubTab(int subTabID)
{
  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  setLibraryView(NULL);
  setMyModelView(NULL);
  setEditView(NULL);

  //doc->openSidebar();
  doc->closeSidebar();
}

void MainRightColumnController::configureForOutputVariablesSubTab(int subTabID)
{
  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  setLibraryView(NULL);
  setMyModelView(NULL);
  setEditView(NULL);

  //doc->openSidebar();
  doc->closeSidebar();
}

void MainRightColumnController::configureForSimulationSettingsSubTab(int subTabID)
{
  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  setLibraryView(NULL);
  setMyModelView(NULL);
  setEditView(NULL);

  //doc->openSidebar();
  doc->closeSidebar();
}

void MainRightColumnController::configureForScriptsSubTab(int subTabID)
{
  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  setLibraryView(m_measureLibraryController->localLibraryView.data());
  setMyModelView(NULL);
  setEditView(m_measureEditController->editView.data());

  doc->openSidebar();
}

void MainRightColumnController::configureForRunSimulationSubTab(int subTabID)
{
  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  setLibraryView(NULL);
  setMyModelView(NULL);
  setEditView(NULL);

  //doc->openSidebar();
  doc->closeSidebar();
}

void MainRightColumnController::configureForResultsSummarySubTab(int subTabID)
{
  boost::shared_ptr<OSDocument> doc = OSAppBase::instance()->currentDocument();

  setLibraryView(NULL);
  setMyModelView(NULL);
  setEditView(NULL);

  //doc->openSidebar();
  doc->closeSidebar();
}

void MainRightColumnController::toggleUnits(bool displayIP)
{
}


QSharedPointer<LocalLibraryController> MainRightColumnController::measuresLibraryController()
{
  return m_measureLibraryController;
}

QSharedPointer<EditController> MainRightColumnController::measuresEditController()
{
  return m_measureEditController;
}

void MainRightColumnController::chooseEditTab()
{
  m_horizontalTabWidget->setCurrentId(EDIT);

  OSAppBase::instance()->currentDocument()->openSidebar();
}

} // openstudio

