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

#ifndef ISOMODEL_FORWARDTRANSLATOR_HPP
#define ISOMODEL_FORWARDTRANSLATOR_HPP

#include "ISOModelAPI.hpp"

#include "../utilities/core/Optional.hpp"
#include "../utilities/core/Logger.hpp"
#include "../utilities/core/StringStreamLogSink.hpp"

namespace openstudio {

namespace model {
  class Model;
}

namespace isomodel {

  class UserModel;

  class ISOMODEL_API ForwardTranslator {
  public:
    
    ForwardTranslator();

    virtual ~ForwardTranslator();

    /** Translate an OpenStudio Model to an ISO UserModel. */
    // DLM: should this return an optional UserModel?
    UserModel translateModel(const openstudio::model::Model& model);

    /** Get warning messages generated by the last translation. */
    std::vector<LogMessage> warnings() const;

    /** Get error messages generated by the last translation. */
    std::vector<LogMessage> errors() const;

  private:

    StringStreamLogSink m_logSink;

    REGISTER_LOGGER("openstudio.isomodel.ForwardTranslator");
  };

} // isomodel
} // openstudio

#endif // ISOMODEL_FORWARDTRANSLATOR_HPP
