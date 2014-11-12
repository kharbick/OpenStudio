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

#include "ThermochromicGlazing.hpp"
#include "ThermochromicGlazing_Impl.hpp"

#include "Glazing.hpp"
#include "Glazing_Impl.hpp"
#include "Model.hpp"
#include "ModelExtensibleGroup.hpp"

#include <utilities/idd/OS_WindowMaterial_GlazingGroup_Thermochromic_FieldEnums.hxx>
#include <utilities/idd/IddEnums.hxx>

#include "../utilities/core/Assert.hpp"
#include "../utilities/math/FloatCompare.hpp"

namespace openstudio {
namespace model {

namespace detail {

  ThermochromicGlazing_Impl::ThermochromicGlazing_Impl(const IdfObject& idfObject,
                                                       Model_Impl* model,
                                                       bool keepHandle)
    : Glazing_Impl(idfObject,model,keepHandle)
  {
    OS_ASSERT(idfObject.iddObject().type() == ThermochromicGlazing::iddObjectType());
  }

  ThermochromicGlazing_Impl::ThermochromicGlazing_Impl(const openstudio::detail::WorkspaceObject_Impl& other,
                                                       Model_Impl* model,
                                                       bool keepHandle)
    : Glazing_Impl(other,model,keepHandle)
  {
    OS_ASSERT(other.iddObject().type() == ThermochromicGlazing::iddObjectType());
  }

  ThermochromicGlazing_Impl::ThermochromicGlazing_Impl(const ThermochromicGlazing_Impl& other,
                                                       Model_Impl* model,
                                                       bool keepHandle)
    : Glazing_Impl(other,model,keepHandle)
  {}

  IddObjectType ThermochromicGlazing_Impl::iddObjectType() const
  {
    return ThermochromicGlazing::iddObjectType();
  }

  std::vector<ThermochromicGlazingDataPoint > ThermochromicGlazing_Impl::thermochromicGlazingDataPoints() const
  {
    std::vector<ThermochromicGlazingDataPoint > result;

    std::vector<Glazing> glazings(glazings());

    std::vector<double> temperatures;// (this->temperatures()); TODO

    OS_ASSERT(glazings.size() == temperatures.size());

    for (unsigned index = 0; index < glazings.size(); ++index) {
      ThermochromicGlazingDataPoint thermochromicGlazingDataPoint;
      thermochromicGlazingDataPoint.m_glazingMaterial = glazings[index];
      thermochromicGlazingDataPoint.m_opticalDataTemperature = temperatures[index];
      result.push_back(thermochromicGlazingDataPoint);
    }

    return result;
  }

  bool ThermochromicGlazing_Impl::setThermochromicGlazingDataPoints(const std::vector<ThermochromicGlazingDataPoint> thermochromicGlazingDataPoints)
  {
    std::vector<Glazing> glazings;
    std::vector<double> temperatures;

    for (const ThermochromicGlazingDataPoint & thermochromicGlazingDataPoint : thermochromicGlazingDataPoints) {
      glazings.push_back(thermochromicGlazingDataPoint.m_glazingMaterial.get());
      temperatures.push_back(thermochromicGlazingDataPoint.m_opticalDataTemperature);
    }

    setGlazings(glazings);
    setTemperatures(temperatures);

    this->emitChangeSignals();

    return true;
  }

  //std::vector<Glazing> ThermochromicGlazing_Impl::mf_glazings() const {
  //  GlazingVector result;
  //  for (const IdfExtensibleGroup& idfGroup : extensibleGroups()) {
  //    ModelExtensibleGroup group = idfGroup.cast<ModelExtensibleGroup>();
  //    OptionalWorkspaceObject owo = group.getTarget(OS_WindowMaterial_GlazingGroup_ThermochromicExtensibleFields::WindowMaterialGlazingName);
  //    if (owo) {
  //      OptionalGlazing og = owo->optionalCast<Glazing>();
  //      OS_ASSERT(og);
  //      result.push_back(*og);
  //    }
  //  }
  //  return result;
  //}

  // LayeredConstruction_Impl
  std::vector<Glazing> ThermochromicGlazing_Impl::glazings() const
  {
    std::vector<Glazing> result;

    // loop through extensible groups
    for (const IdfExtensibleGroup& idfGroup : extensibleGroups()) {
      ModelExtensibleGroup group = idfGroup.cast<ModelExtensibleGroup>();
      // get object pointed to by extensible group
      OptionalGlazing oGlazing = group.getModelObjectTarget<Glazing>(0);
      if (!oGlazing) {
        LOG(Warn, "Skipping glazing " << group.groupIndex() << " in " << briefDescription()
          << ", as there is no Glazing object referenced by the corresponding field.");
        continue;
      }
      result.push_back(*oGlazing);
    }

    return result;
  }

  bool ThermochromicGlazing_Impl::setGlazings(const std::vector<Glazing> glazings)
  {
    clearExtensibleGroups();
    for (const Glazing & glazing : glazings) {
      OS_ASSERT(glazing.model() == model());
      ModelExtensibleGroup group = pushExtensibleGroup(StringVector()).cast<ModelExtensibleGroup>();
      OS_ASSERT(!group.empty());
      bool ok = group.setPointer(0, glazing.handle());
      OS_ASSERT(ok);
    }
    return true;
  }

  // PlanarSurface_Impl
  std::vector<double> ThermochromicGlazing_Impl::temperatures()
  {
    std::vector<double> results;
    
    for (const ModelExtensibleGroup& group : castVector<ModelExtensibleGroup>(extensibleGroups())) {
      OptionalDouble temperature = group.getDouble(0);

      if ( temperature ){
        results.push_back(*temperature);
      } else {
        LOG(Error, "Could not read temperature " << group.groupIndex() << " in " << briefDescription() << ".");
      }
    }

    return results;
  }

  bool ThermochromicGlazing_Impl::setTemperatures(const std::vector<double> temperatures)
  {
    unsigned n = temperatures.size();

    bool result = true;

    clearExtensibleGroups(false);

    for (unsigned index = 0; index < n; ++index) {
      std::vector<std::string> values;
      values.push_back(toString(temperatures[index]));
      ModelExtensibleGroup group = pushExtensibleGroup(values, false).cast<ModelExtensibleGroup>();
      OS_ASSERT(!group.empty());
    }

    return result;
  }

  OptionalDouble ThermochromicGlazing_Impl::getVisibleTransmittance() const {
    LOG_AND_THROW("Visible transmittance not yet supported for ThermochromicGlazings.");
  }

  const std::vector<std::string>& ThermochromicGlazing_Impl::outputVariableNames() const
  {
    static std::vector<std::string> result;
    return result;
  }

  bool ThermochromicGlazing_Impl::setVisibleTransmittance(double value) {
    return false;
  }

} // detail

ThermochromicGlazing::ThermochromicGlazing(const Model& model,double opticalDataTemperature) // TODO
  : Glazing(ThermochromicGlazing::iddObjectType(),model)
{
  OS_ASSERT(getImpl<detail::ThermochromicGlazing_Impl>());

  // TODO: Appropriately handle the following required object-list fields.
  bool ok = true;
  // ok = setHandle();
  OS_ASSERT(ok);
}

IddObjectType ThermochromicGlazing::iddObjectType() {
  return IddObjectType(IddObjectType::OS_WindowMaterial_GlazingGroup_Thermochromic);
}

/// @cond
ThermochromicGlazing::ThermochromicGlazing(std::shared_ptr<detail::ThermochromicGlazing_Impl> impl)
  : Glazing(impl)
{}

std::vector<ThermochromicGlazingDataPoint > ThermochromicGlazing::thermochromicGlazingDataPoints() const {
  return getImpl<detail::ThermochromicGlazing_Impl>()->thermochromicGlazingDataPoints();
}

bool ThermochromicGlazing::setThermochromicGlazingDataPoints(const std::vector<ThermochromicGlazingDataPoint> thermochromicGlazingDataPoints) {
  return getImpl<detail::ThermochromicGlazing_Impl>()->setThermochromicGlazingDataPoints(thermochromicGlazingDataPoints);
}

/// @endcond

} // model
} // openstudio

