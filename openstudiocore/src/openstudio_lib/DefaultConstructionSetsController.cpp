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

#include <openstudio_lib/DefaultConstructionSetsController.hpp>
#include <openstudio_lib/DefaultConstructionSetsView.hpp>

#include <model/DefaultConstructionSet.hpp>
#include <model/DefaultConstructionSet_Impl.hpp>
#include <model/DefaultSurfaceConstructions.hpp>
#include <model/DefaultSubSurfaceConstructions.hpp>

#include <utilities/core/Logger.hpp>

namespace openstudio {

DefaultConstructionSetsController::DefaultConstructionSetsController(const model::Model& model)
  : ModelSubTabController(new DefaultConstructionSetsView(model, "Default Constructions", false), model)
{
}

DefaultConstructionSetsController::~DefaultConstructionSetsController()
{
}

void DefaultConstructionSetsController::onAddObject(const openstudio::IddObjectType& iddObjectType)
{
    if( model::DefaultConstructionSet::iddObjectType() == iddObjectType )
    {
      openstudio::model::DefaultConstructionSet(this->model());
    }
    else
    {
      LOG_FREE_AND_THROW("DefaultConstructionSetsController", "Unknown IddObjectType '" << iddObjectType.valueName() << "'");
    }
}

void DefaultConstructionSetsController::onCopyObject(const openstudio::model::ModelObject& modelObject)
{
  modelObject.clone(this->model());
}

void DefaultConstructionSetsController::onRemoveObject(openstudio::model::ModelObject modelObject)
{
  modelObject.remove();
}

void DefaultConstructionSetsController::onReplaceObject(openstudio::model::ModelObject modelObject, const OSItemId& replacementItemId)
{
  // not yet implemented
}

void DefaultConstructionSetsController::onPurgeObjects(const openstudio::IddObjectType& iddObjectType)
{
  if (iddObjectType == model::DefaultConstructionSet::iddObjectType()){
    this->model().purgeUnusedResourceObjects(model::DefaultConstructionSet::iddObjectType());
    this->model().purgeUnusedResourceObjects(model::DefaultSurfaceConstructions::iddObjectType());
    this->model().purgeUnusedResourceObjects(model::DefaultSubSurfaceConstructions::iddObjectType());
  }else{
    this->model().purgeUnusedResourceObjects(iddObjectType);
  }
}

void DefaultConstructionSetsController::onDrop(const OSItemId& itemId)
{
  boost::optional<model::ModelObject> modelObject = this->getModelObject(itemId);
  if(modelObject){
    if(modelObject->optionalCast<model::DefaultConstructionSet>()){
      if (this->fromComponentLibrary(itemId)){
        modelObject = modelObject->clone(this->model());
      }
    }
  }
}

void DefaultConstructionSetsController::onInspectItem(OSItem* item)
{
}

} // openstudio

