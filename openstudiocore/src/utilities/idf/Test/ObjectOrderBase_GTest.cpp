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

#include <gtest/gtest.h>
#include "IdfFixture.hpp"

#include "../ObjectOrderBase.hpp"

#include <utilities/idd/Building_FieldEnums.hxx>
#include <utilities/idd/AirflowNetwork_Distribution_Node_FieldEnums.hxx>
#include <utilities/idd/AirflowNetwork_Distribution_Component_Coil_FieldEnums.hxx>
#include <utilities/idd/Lights_FieldEnums.hxx>
#include <utilities/idd/Branch_FieldEnums.hxx>
#include <utilities/idd/Output_Diagnostics_FieldEnums.hxx>
#include <utilities/idd/Zone_FieldEnums.hxx>
#include <utilities/idd/RunPeriod_FieldEnums.hxx>
#include <utilities/idd/Schedule_Compact_FieldEnums.hxx>
#include <utilities/idd/Schedule_Day_Hourly_FieldEnums.hxx>
#include <utilities/idd/DesignSpecification_OutdoorAir_FieldEnums.hxx>
#include <utilities/idd/Ceiling_Adiabatic_FieldEnums.hxx>
#include <utilities/idd/Building_FieldEnums.hxx>
#include <utilities/idd/Daylighting_Controls_FieldEnums.hxx>
#include <utilities/idd/ThermalStorage_ChilledWater_Mixed_FieldEnums.hxx>
#include <utilities/idd/Refrigeration_CompressorList_FieldEnums.hxx>
#include <utilities/idd/ElectricLoadCenter_Generators_FieldEnums.hxx>
#include <utilities/idd/ZoneControl_Humidistat_FieldEnums.hxx>
#include <utilities/idd/Daylighting_Controls_FieldEnums.hxx>

using openstudio::ObjectOrderBase;
using openstudio::IddObjectTypeVector;
using openstudio::OptionalIddObjectTypeVector;
using openstudio::IddObjectType;

TEST_F(IdfFixture,ObjectOrderBase_Constructors) {
  // default
  ObjectOrderBase defaultOrderer;
  EXPECT_TRUE(defaultOrderer.orderByIddEnum());
  EXPECT_FALSE(defaultOrderer.iddOrder());
  // order is taken from IddObjectType enum order
  EXPECT_TRUE(defaultOrderer.less(openstudio::iddobjectname::Building,
              openstudio::iddobjectname::AirflowNetwork_Distribution_Component_Coil));
  EXPECT_FALSE(defaultOrderer.less(openstudio::iddobjectname::Output_Diagnostics,openstudio::iddobjectname::Lights));
  EXPECT_TRUE(defaultOrderer.indexInOrder(openstudio::iddobjectname::Branch));

  // specified order of IddObjectTypes
  IddObjectTypeVector order;
  order.push_back(openstudio::iddobjectname::Lights);
  order.push_back(openstudio::iddobjectname::Zone);
  order.push_back(openstudio::iddobjectname::RunPeriod);
  order.push_back(openstudio::iddobjectname::Building);
  ObjectOrderBase userEnumOrder(order);
  EXPECT_FALSE(userEnumOrder.orderByIddEnum());
  ASSERT_TRUE(userEnumOrder.iddOrder());
  EXPECT_TRUE(order == *(userEnumOrder.iddOrder()));
  ASSERT_TRUE(userEnumOrder.indexInOrder(openstudio::iddobjectname::RunPeriod));
  EXPECT_EQ(static_cast<unsigned>(2),*(userEnumOrder.indexInOrder(openstudio::iddobjectname::RunPeriod)));
  ASSERT_TRUE(userEnumOrder.indexInOrder(openstudio::iddobjectname::Branch));
  EXPECT_EQ(static_cast<unsigned>(4),*(userEnumOrder.indexInOrder(openstudio::iddobjectname::Branch)));

  // derived class is handling order
  ObjectOrderBase cededControl(true);
  EXPECT_FALSE(cededControl.orderByIddEnum());
  EXPECT_FALSE(cededControl.iddOrder());
  EXPECT_FALSE(cededControl.indexInOrder(openstudio::iddobjectname::Building));
}

// test that when new type of order is set, others are disabled
TEST_F(IdfFixture,ObjectOrderBase_OrderSetters) {
  ObjectOrderBase orderer;

  IddObjectTypeVector order;
  order.push_back(openstudio::iddobjectname::Lights);
  order.push_back(openstudio::iddobjectname::Zone);
  order.push_back(openstudio::iddobjectname::RunPeriod);
  order.push_back(openstudio::iddobjectname::Building);

  orderer.setIddOrder(order);
  EXPECT_FALSE(orderer.orderByIddEnum());
  EXPECT_TRUE(orderer.less(openstudio::iddobjectname::Lights,openstudio::iddobjectname::Building));

  orderer.setOrderByIddEnum();
  EXPECT_TRUE(orderer.orderByIddEnum());
  EXPECT_FALSE(orderer.iddOrder());
  EXPECT_TRUE(orderer.less(openstudio::iddobjectname::Building,openstudio::iddobjectname::Lights));

  orderer.setDirectOrder();
  EXPECT_FALSE(orderer.orderByIddEnum());
  EXPECT_FALSE(orderer.iddOrder());
}

TEST_F(IdfFixture,ObjectOrderBase_ManipulateIddObjectTypeOrder) {
  IddObjectTypeVector order;
  order.push_back(openstudio::iddobjectname::Lights);    // 0
  order.push_back(openstudio::iddobjectname::Zone);      // 1
  order.push_back(openstudio::iddobjectname::RunPeriod); // 2
  order.push_back(openstudio::iddobjectname::Building);  // 3

  ObjectOrderBase orderer(order);
  bool success;

  // push_back
  success = orderer.push_back(openstudio::iddobjectname::Schedule_Compact); // 4
  EXPECT_TRUE(success);
  EXPECT_EQ(static_cast<unsigned>(4),*(orderer.indexInOrder(openstudio::iddobjectname::Schedule_Compact)));
  EXPECT_EQ(static_cast<unsigned>(5),*(orderer.indexInOrder(openstudio::iddobjectname::Schedule_Day_Hourly)));
  EXPECT_TRUE(orderer.less(openstudio::iddobjectname::Schedule_Compact,
                           openstudio::iddobjectname::DesignSpecification_OutdoorAir));
  EXPECT_FALSE(orderer.less(openstudio::iddobjectname::DesignSpecification_OutdoorAir,
                            openstudio::iddobjectname::Schedule_Day_Hourly));

  // insert behind IddObjectType
  success = orderer.insert(openstudio::iddobjectname::Ceiling_Adiabatic,IddObjectType(openstudio::iddobjectname::Building));
  EXPECT_TRUE(success);
  EXPECT_EQ(static_cast<unsigned>(3),*(orderer.indexInOrder(openstudio::iddobjectname::Ceiling_Adiabatic)));
  success = orderer.insert(openstudio::iddobjectname::Daylighting_Controls,
                           IddObjectType(openstudio::iddobjectname::AirflowNetwork_Distribution_Node));
  EXPECT_TRUE(success);
  EXPECT_EQ(orderer.iddOrder()->size() - 1,
            *(orderer.indexInOrder(openstudio::iddobjectname::Daylighting_Controls)));

  // insert at index
  success = orderer.insert(openstudio::iddobjectname::ThermalStorage_ChilledWater_Mixed,2);
  EXPECT_TRUE(success);
  EXPECT_EQ(static_cast<unsigned>(2),
            *(orderer.indexInOrder(openstudio::iddobjectname::ThermalStorage_ChilledWater_Mixed)));
  success = orderer.insert(openstudio::iddobjectname::Refrigeration_CompressorList,37891);
  EXPECT_TRUE(success);
  EXPECT_EQ(orderer.iddOrder()->size() - 1,
            *(orderer.indexInOrder(openstudio::iddobjectname::Refrigeration_CompressorList)));

  // move before IddObjectType
  unsigned n = orderer.iddOrder()->size();
  success = orderer.move(openstudio::iddobjectname::Refrigeration_CompressorList,
                         IddObjectType(openstudio::iddobjectname::Lights));
  EXPECT_TRUE(success);
  EXPECT_EQ(static_cast<unsigned>(0),
            *(orderer.indexInOrder(openstudio::iddobjectname::Refrigeration_CompressorList)));
  success = orderer.move(openstudio::iddobjectname::ElectricLoadCenter_Generators,
                         IddObjectType(openstudio::iddobjectname::Building));
  EXPECT_FALSE(success);
  success = orderer.move(openstudio::iddobjectname::Building,
                         IddObjectType(openstudio::iddobjectname::ElectricLoadCenter_Generators));
  EXPECT_TRUE(success);
  EXPECT_EQ(orderer.iddOrder()->size() - 1,
            *(orderer.indexInOrder(openstudio::iddobjectname::Building)));
  EXPECT_EQ(n,orderer.iddOrder()->size());

  // move to index
  success = orderer.move(openstudio::iddobjectname::Building,0);
  EXPECT_TRUE(success);
  EXPECT_EQ(static_cast<unsigned>(0),
            *(orderer.indexInOrder(openstudio::iddobjectname::Building)));
  success = orderer.move(openstudio::iddobjectname::RunPeriod,18601);
  EXPECT_TRUE(success);
  EXPECT_EQ(orderer.iddOrder()->size() - 1,
            *(orderer.indexInOrder(openstudio::iddobjectname::RunPeriod)));
  success = orderer.move(openstudio::iddobjectname::ZoneControl_Humidistat,0);
  EXPECT_FALSE(success);
  EXPECT_EQ(n,orderer.iddOrder()->size());

  // swap
  unsigned i = *(orderer.indexInOrder(openstudio::iddobjectname::Lights));
  unsigned j = *(orderer.indexInOrder(openstudio::iddobjectname::Refrigeration_CompressorList));
  success = orderer.swap(openstudio::iddobjectname::Lights,
                         openstudio::iddobjectname::Refrigeration_CompressorList);
  EXPECT_TRUE(success);
  EXPECT_EQ(i,*(orderer.indexInOrder(openstudio::iddobjectname::Refrigeration_CompressorList)));
  EXPECT_EQ(j,*(orderer.indexInOrder(openstudio::iddobjectname::Lights)));
  EXPECT_EQ(n,orderer.iddOrder()->size());

  // erase
  success = orderer.erase(openstudio::iddobjectname::Refrigeration_CompressorList);
  EXPECT_TRUE(success);
  EXPECT_EQ(orderer.iddOrder()->size(),
            *(orderer.indexInOrder(openstudio::iddobjectname::Refrigeration_CompressorList)));
  success = orderer.erase(openstudio::iddobjectname::Refrigeration_CompressorList);
  EXPECT_FALSE(success);
  EXPECT_TRUE(orderer.iddOrder()->size() < n);
}
