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

#ifndef MODEL_THERMOCHROMICGLAZING_IMPL_HPP
#define MODEL_THERMOCHROMICGLAZING_IMPL_HPP

#include "ModelAPI.hpp"
#include "Glazing.hpp"
#include "Glazing_Impl.hpp"

namespace openstudio {
namespace model {

namespace detail {

  /** ThermochromicGlazing_Impl is a Glazing_Impl that is the implementation class for ThermochromicGlazing.*/
  class MODEL_API ThermochromicGlazing_Impl : public Glazing_Impl {
   public:
    /** @name Constructors and Destructors */
    //@{

    ThermochromicGlazing_Impl(const IdfObject& idfObject,
                              Model_Impl* model,
                              bool keepHandle);

    ThermochromicGlazing_Impl(const openstudio::detail::WorkspaceObject_Impl& other,
                              Model_Impl* model,
                              bool keepHandle);

    ThermochromicGlazing_Impl(const ThermochromicGlazing_Impl& other,
                              Model_Impl* model,
                              bool keepHandle);

    virtual ~ThermochromicGlazing_Impl() {}

    //@}

    /** @name Virtual Methods */
    //@{

    // required by modelObject
    virtual const std::vector<std::string>& outputVariableNames() const;

    virtual IddObjectType iddObjectType() const;

    //@}
    /** @name Getters */
    //@{

    // required by Material
    virtual boost::optional<double> getVisibleTransmittance() const;

    //@}
    /** @name Setters */
    //@{

    virtual bool setVisibleTransmittance(double value);

    //@}
    /** @name Other */
    //@{

    //@}
   protected:
   private:
    REGISTER_LOGGER("openstudio.model.ThermochromicGlazing");

    std::vector<Glazing> mf_glazings() const;

  };

} // detail

} // model
} // openstudio

#endif // MODEL_THERMOCHROMICGLAZING_IMPL_HPP

