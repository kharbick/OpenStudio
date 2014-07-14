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

#include "MaterialsController.hpp"

#include "MaterialsView.hpp"

#include "../model/AirGap.hpp"
#include "../model/AirWallMaterial.hpp"
#include "../model/Blind.hpp"
#include "../model/InfraredTransparentMaterial.hpp"
#include "../model/MasslessOpaqueMaterial.hpp"
#include "../model/Material.hpp"
#include "../model/Material_Impl.hpp"
#include "../model/Gas.hpp"
#include "../model/GasMixture.hpp"
#include "../model/RefractionExtinctionGlazing.hpp"
#include "../model/RoofVegetation.hpp"
#include "../model/Screen.hpp"
#include "../model/Shade.hpp"
#include "../model/SimpleGlazing.hpp"
#include "../model/StandardGlazing.hpp"
#include "../model/StandardOpaqueMaterial.hpp"
#include "../model/ThermochromicGlazing.hpp"

#include "../utilities/core/Logger.hpp"

namespace openstudio {

MaterialsController::MaterialsController(bool isIP, const model::Model& model)
  : ModelSubTabController(new MaterialsView(isIP, model, "Materials", false), model)
{
}

MaterialsController::~MaterialsController()
{
}

void MaterialsController::onAddObject(const openstudio::IddObjectType& iddObjectType)
{
  model::Model model = this->model();
  boost::optional<model::Material> mat;

  if( model::StandardOpaqueMaterial::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::StandardOpaqueMaterial(model);
  }
  else if( model::MasslessOpaqueMaterial::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::MasslessOpaqueMaterial(model);
  }
  else if( model::AirGap::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::AirGap(model);
  }
  else if( model::AirWallMaterial::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::AirWallMaterial(model);
  }
  else if( model::InfraredTransparentMaterial::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::InfraredTransparentMaterial(model);
  }
  else if( model::RoofVegetation::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::RoofVegetation(model);
  }
  else if( model::SimpleGlazing::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::SimpleGlazing(model);
  }
  else if( model::StandardGlazing::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::StandardGlazing(model);
  }
  else if( model::Gas::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::Gas(model);
  }
  else if( model::GasMixture::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::GasMixture(model);
  }
  else if( model::Blind::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::Blind(model);
  }
  else if( model::Screen::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::Screen(model);
  }
  else if( model::Shade::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::Shade(model);
  }
  else if( model::RefractionExtinctionGlazing::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::RefractionExtinctionGlazing(model);
  }
  else if( model::ThermochromicGlazing::iddObjectType() == iddObjectType )
  {
    mat = openstudio::model::ThermochromicGlazing(model);
  }
  else
  {
    LOG_FREE_AND_THROW("MaterialsController", "Unknown IddObjectType '" << iddObjectType.valueName() << "'");
  }
}

void MaterialsController::onCopyObject(const openstudio::model::ModelObject& modelObject)
{
  modelObject.clone(this->model());
}

void MaterialsController::onRemoveObject(openstudio::model::ModelObject modelObject)
{
  modelObject.remove();
}

void MaterialsController::onReplaceObject(openstudio::model::ModelObject modelObject, const OSItemId& replacementItemId)
{
  // not yet implemented
}

void MaterialsController::onPurgeObjects(const openstudio::IddObjectType& iddObjectType)
{
  this->model().purgeUnusedResourceObjects(iddObjectType);
}

void MaterialsController::onDrop(const OSItemId& itemId)
{
  boost::optional<model::ModelObject> modelObject = this->getModelObject(itemId);
  if(modelObject){
    if(modelObject->optionalCast<model::Material>()){
      if (this->fromComponentLibrary(itemId)){
        modelObject = modelObject->clone(this->model());
      }
    }
  }
}

void MaterialsController::onInspectItem(OSItem* item)
{
}

} // openstudio

