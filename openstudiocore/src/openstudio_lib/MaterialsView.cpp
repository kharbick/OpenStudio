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

#include "MaterialsView.hpp"

#include "MaterialAirGapInspectorView.hpp"
#include "MaterialAirWallInspectorView.hpp"
#include "MaterialInfraredTransparentInspectorView.hpp"
#include "MaterialInspectorView.hpp"
#include "MaterialNoMassInspectorView.hpp"
#include "MaterialRoofVegetationInspectorView.hpp"
#include "ModelObjectTypeListView.hpp"
#include "WindowMaterialBlindInspectorView.hpp"
#include "WindowMaterialGasInspectorView.hpp"
#include "WindowMaterialGasMixtureInspectorView.hpp"
#include "WindowMaterialGlazingGroupThermochromicInspectorView.hpp"
#include "WindowMaterialGlazingInspectorView.hpp"
#include "WindowMaterialGlazingRefractionExtinctionMethodInspectorView.hpp"
#include "WindowMaterialScreenInspectorView.hpp"
#include "WindowMaterialShadeInspectorView.hpp"
#include "WindowMaterialSimpleGlazingSystemInspectorView.hpp"

#include "../model/Model_Impl.hpp"

#include "../model/StandardOpaqueMaterial.hpp"
#include "../model/MasslessOpaqueMaterial.hpp"
#include "../model/AirGap.hpp"
#include "../model/SimpleGlazing.hpp"
#include "../model/StandardGlazing.hpp"
#include "../model/Gas.hpp"
#include "../model/GasMixture.hpp"
#include "../model/Blind.hpp"
#include "../model/Screen.hpp"
#include "../model/Shade.hpp"
#include "../model/AirWallMaterial.hpp"
#include "../model/InfraredTransparentMaterial.hpp"
#include "../model/RoofVegetation.hpp"
#include "../model/RefractionExtinctionGlazing.hpp"
#include "../model/ThermochromicGlazing.hpp"

#include "../utilities/core/Assert.hpp"

#include <QStackedWidget>

namespace openstudio {


MaterialsView::MaterialsView(bool isIP,
                             const openstudio::model::Model& model,
                             const QString& tabLabel,
                             bool hasSubTabs,
                             QWidget * parent)
: ModelSubTabView(new ModelObjectTypeListView(MaterialsView::modelObjectTypesAndNames(), model, true, OSItemType::CollapsibleListHeader, parent),
  new MaterialsInspectorView(isIP, model, parent),
  parent)
{
  bool isConnected = connect(this,SIGNAL(toggleUnitsClicked(bool)),
                             modelObjectInspectorView(),SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);
}

std::vector<std::pair<IddObjectType, std::string> > MaterialsView::modelObjectTypesAndNames()
{
  std::vector<std::pair<IddObjectType, std::string> > result;
  result.push_back(std::make_pair<IddObjectType, std::string>(model::StandardOpaqueMaterial::iddObjectType(), "Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::MasslessOpaqueMaterial::iddObjectType(), "No Mass Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::AirGap::iddObjectType(), "Air Gap Materials"));

  result.push_back(std::make_pair<IddObjectType, std::string>(model::SimpleGlazing::iddObjectType(), "Simple Glazing System Window Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::StandardGlazing::iddObjectType(), "Glazing Window Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::Gas::iddObjectType(), "Gas Window Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::GasMixture::iddObjectType(), "Gas Mixture Window Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::Blind::iddObjectType(), "Blind Window Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::Screen::iddObjectType(), "Screen Window Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::Shade::iddObjectType(), "Shade Window Materials"));

  // Oddballs to be listed at the bottom of the list
  result.push_back(std::make_pair<IddObjectType, std::string>(model::AirWallMaterial::iddObjectType(), "Air Wall Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::InfraredTransparentMaterial::iddObjectType(), "Infrared Transparent Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::RoofVegetation::iddObjectType(), "Roof Vegetation Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::RefractionExtinctionGlazing::iddObjectType(), "Refraction Extinction Method Glazing Window Materials"));
  result.push_back(std::make_pair<IddObjectType, std::string>(model::ThermochromicGlazing::iddObjectType(), "Glazing Group Thermochromic Window Materials"));
  
  return result;
}

MaterialsInspectorView::MaterialsInspectorView(bool isIP,
                                               const model::Model& model,
                                               QWidget * parent )
  : ModelObjectInspectorView(model, false, parent),
    m_isIP(isIP)
{
  //// Hack code to remove when tab active
  //QLabel * underConstructionLabel = new QLabel();
  //underConstructionLabel->setPixmap(QPixmap(":/images/coming_soon_building_summary.png"));
  //underConstructionLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  //int index = this->stackedWidget()->addWidget(underConstructionLabel);
  //OS_ASSERT(index == 0);

  //// index of hidden widget is 0
  ////QWidget* hiddenWidget = new QWidget();
  ////int index = this->stackedWidget()->addWidget(hiddenWidget);
  ////OS_ASSERT(index == 0);

  //// index of the default is 1
  //DefaultInspectorView* defaultInspectorView = new DefaultInspectorView(model, parent);
  //index = this->stackedWidget()->addWidget(defaultInspectorView);
  //OS_ASSERT(index == 1);

  ////StandardOpaqueMaterialInspectorView* standardOpaqueMaterialInspectorView = new StandardOpaqueMaterialInspectorView(model, parent);
  ////index = this->stackedWidget()->addWidget(standardOpaqueMaterialInspectorView);
  ////m_inspectorIndexMap[IddObjectType::OS_Material] = index;
}

void MaterialsInspectorView::onClearSelection()
{
  QWidget* widget = this->stackedWidget()->currentWidget();
  ModelObjectInspectorView* modelObjectInspectorView = qobject_cast<ModelObjectInspectorView*>(widget);
  if(modelObjectInspectorView){
    modelObjectInspectorView->clearSelection();
  }

  this->stackedWidget()->setCurrentIndex(0);
}

void MaterialsInspectorView::onUpdate()
{
}

void MaterialsInspectorView::onSelectModelObject(const openstudio::model::ModelObject& modelObject)
{
  IddObjectType type = modelObject.iddObjectType();

  if( model::StandardOpaqueMaterial::iddObjectType() == type )
  {
    this->showMaterialInspectorView(modelObject);
  }
  else if( model::AirGap::iddObjectType() == type )
  {
    this->showMaterialAirGapInspectorView(modelObject);
  }
  else if( model::AirWallMaterial::iddObjectType() == type )
  {
    this->showMaterialAirWallInspectorView(modelObject);
  }
  else if( model::InfraredTransparentMaterial::iddObjectType() == type )
  {
    this->showMaterialInfraredTransparentInspectorView(modelObject);
  }
  else if( model::MasslessOpaqueMaterial::iddObjectType() == type )
  {
    this->showMaterialNoMassInspectorView(modelObject);
  }
  else if( model::RoofVegetation::iddObjectType() == type )
  {
    this->showMaterialRoofVegetationInspectorView(modelObject);
  }
  else if( model::Blind::iddObjectType() == type )
  {
    this->showWindowMaterialBlindInspectorView(modelObject);
  }
  else if( model::Gas::iddObjectType() == type )
  {
    this->showWindowMaterialGasInspectorView(modelObject);
  }
  else if( model::GasMixture::iddObjectType() == type )
  {
    this->showWindowMaterialGasMixtureInspectorView(modelObject);
  }
  else if( model::StandardGlazing::iddObjectType() == type )
  {
    this->showWindowMaterialGlazingInspectorView(modelObject);
  }
  else if( model::RefractionExtinctionGlazing::iddObjectType() == type )
  {
    this->showWindowMaterialGlazingRefractionExtinctionMethodInspectorView(modelObject);
  }
  else if( model::ThermochromicGlazing::iddObjectType() == type )
  {
    this->showWindowMaterialGlazingGroupThermochromicInspectorView(modelObject);
  }
  else if( model::Screen::iddObjectType() == type )
  {
    this->showWindowMaterialScreenInspectorView(modelObject);
  }
  else if( model::Shade::iddObjectType() == type )
  {
    this->showWindowMaterialShadeInspectorView(modelObject);
  }
  else if( model::SimpleGlazing::iddObjectType() == type )
  {
    this->showWindowMaterialSimpleGlazingSystemInspectorView(modelObject);
  }
  else
  {
    showDefaultView();      
  }
}

void MaterialsInspectorView::showMaterialAirGapInspectorView(const openstudio::model::ModelObject & modelObject)
{
  MaterialAirGapInspectorView * view = new MaterialAirGapInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showMaterialAirWallInspectorView(const openstudio::model::ModelObject & modelObject)
{
  MaterialAirWallInspectorView * view = new MaterialAirWallInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showMaterialInfraredTransparentInspectorView(const openstudio::model::ModelObject & modelObject)
{
  MaterialInfraredTransparentInspectorView * view = new MaterialInfraredTransparentInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showMaterialInspectorView(const openstudio::model::ModelObject & modelObject)
{
  MaterialInspectorView * view = new MaterialInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showMaterialNoMassInspectorView(const openstudio::model::ModelObject & modelObject)
{
  MaterialNoMassInspectorView * view = new MaterialNoMassInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showMaterialRoofVegetationInspectorView(const openstudio::model::ModelObject & modelObject)
{
  MaterialRoofVegetationInspectorView * view = new MaterialRoofVegetationInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showWindowMaterialBlindInspectorView(const openstudio::model::ModelObject & modelObject)
{
  WindowMaterialBlindInspectorView * view = new WindowMaterialBlindInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showWindowMaterialGasInspectorView(const openstudio::model::ModelObject & modelObject)
{
  WindowMaterialGasInspectorView * view = new WindowMaterialGasInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showWindowMaterialGasMixtureInspectorView(const openstudio::model::ModelObject & modelObject)
{
  WindowMaterialGasMixtureInspectorView * view = new WindowMaterialGasMixtureInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showWindowMaterialGlazingGroupThermochromicInspectorView(const openstudio::model::ModelObject & modelObject)
{
  WindowMaterialGlazingGroupThermochromicInspectorView * view = new WindowMaterialGlazingGroupThermochromicInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showWindowMaterialGlazingInspectorView(const openstudio::model::ModelObject & modelObject)
{
  WindowMaterialGlazingInspectorView * view = new WindowMaterialGlazingInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showWindowMaterialGlazingRefractionExtinctionMethodInspectorView(const openstudio::model::ModelObject & modelObject)
{
  WindowMaterialGlazingRefractionExtinctionMethodInspectorView * view = new WindowMaterialGlazingRefractionExtinctionMethodInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showWindowMaterialScreenInspectorView(const openstudio::model::ModelObject & modelObject)
{
  WindowMaterialScreenInspectorView * view = new WindowMaterialScreenInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showWindowMaterialShadeInspectorView(const openstudio::model::ModelObject & modelObject)
{
  WindowMaterialShadeInspectorView * view = new WindowMaterialShadeInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showWindowMaterialSimpleGlazingSystemInspectorView(const openstudio::model::ModelObject & modelObject)
{
  WindowMaterialSimpleGlazingSystemInspectorView * view = new WindowMaterialSimpleGlazingSystemInspectorView(m_isIP, m_model);
  bool isConnected = connect(this, 
                             SIGNAL(toggleUnitsClicked(bool)),
                             view, 
                             SIGNAL(toggleUnitsClicked(bool)));
  OS_ASSERT(isConnected);

  view->selectModelObject(modelObject);

  this->showInspector(view);
}

void MaterialsInspectorView::showInspector(QWidget * widget)
{
  if( QWidget * _widget = this->stackedWidget()->currentWidget() )
  {
    this->stackedWidget()->removeWidget(_widget);

    delete _widget;
  }

  this->stackedWidget()->addWidget(widget);
}

void MaterialsInspectorView::showDefaultView()
{
  if( QWidget * widget = this->stackedWidget()->currentWidget() )
  {
    this->stackedWidget()->removeWidget(widget);

    delete widget;
  }
}

/****************** SLOTS ******************/

void MaterialsInspectorView::toggleUnits(bool displayIP)
{
  m_isIP = displayIP;
}

} // openstudio
