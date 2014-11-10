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

  IddObjectType ThermochromicGlazing_Impl::iddObjectType() const {
    return ThermochromicGlazing::iddObjectType();
  }

  OptionalDouble ThermochromicGlazing_Impl::getVisibleTransmittance() const {
    LOG_AND_THROW("Visible transmittance not yet supported for ThermochromicGlazings.");
  }

  const std::vector<std::string>& ThermochromicGlazing_Impl::outputVariableNames() const
  {
    static std::vector<std::string> result;
    if (result.empty()){
    }
    return result;
  }

  bool ThermochromicGlazing_Impl::setVisibleTransmittance(double value) {
    return false;
  }

  std::vector<Glazing> ThermochromicGlazing_Impl::mf_glazings() const {
    GlazingVector result;
    for (const IdfExtensibleGroup& idfGroup : extensibleGroups()) {
      ModelExtensibleGroup group = idfGroup.cast<ModelExtensibleGroup>();
      OptionalWorkspaceObject owo = group.getTarget(OS_WindowMaterial_GlazingGroup_ThermochromicExtensibleFields::WindowMaterialGlazingName);
      if (owo) {
        OptionalGlazing og = owo->optionalCast<Glazing>();
        OS_ASSERT(og);
        result.push_back(*og);
      }
    }
    return result;
  }

} // detail

ThermochromicGlazing::ThermochromicGlazing(const Model& model,double opticalDataTemperature)
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
/// @endcond

} // model
} // openstudio

