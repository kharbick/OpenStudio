/**********************************************************************
*  Copyright (c) 2008-2013, Alliance for Sustainable Energy.
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

#include <utilities/units/OSQuantityVector.hpp>
#include <utilities/units/Quantity.hpp>
#include <utilities/units/ScaleFactory.hpp>
#include <utilities/units/TemperatureUnit.hpp>
#include <utilities/units/TemperatureUnit_Impl.hpp>
#include <utilities/units/IPUnit.hpp>
#include <utilities/units/IPUnit_Impl.hpp>

#include <utilities/math/FloatCompare.hpp>
#include <utilities/data/Vector.hpp>

#include <utilities/core/Containers.hpp>
#include <utilities/core/Assert.hpp>

#include <boost/foreach.hpp>

namespace openstudio {

OSQuantityVector::OSQuantityVector(const Unit& units)
  : m_units(units.clone())
{}

OSQuantityVector::OSQuantityVector(const Unit& units, unsigned n, double value)
  : m_units(units.clone()), m_values(n,value)
{}

OSQuantityVector::OSQuantityVector(const Unit& units, const std::vector<double>& values)
  : m_units(units.clone()), m_values(values)
{}

OSQuantityVector::OSQuantityVector(const std::vector<Quantity>& values)
  : m_units(), m_values(values.size(),0.0)
{
  if (!values.empty()) {
    m_units = values[0].units();
  }
  const Scale& myScale = scale();
  for (unsigned i = 0, n = size(); i < n; ++i) {
    if (!(values[i].units() == m_units)) {
      LOG_AND_THROW("Quantity " << values[i] << " has units inconsistent with the first "
                    << "Quantity in vector values, " << values[0] << ".");
    }
    if (values[i].scale() != myScale) {
      Quantity working = values[i];
      working.setScale(myScale.exponent);
      m_values[i] = working.value();
    }
    else {
      m_values[i] = values[i].value();
    }
  }
}

OSQuantityVector::OSQuantityVector(const OSQuantityVector& other)
  : m_units(other.units()), m_values(other.values())
{}

OSQuantityVector& OSQuantityVector::operator=(const OSQuantityVector& other) {
  if (this == &other) {
    return *this;
  }
  m_units = other.units();
  m_values = other.values();
  return *this;
}

Unit OSQuantityVector::units() const {
  return m_units.clone();
}

UnitSystem OSQuantityVector::system() const {
  return m_units.system();
}

const Scale& OSQuantityVector::scale() const {
  return m_units.scale();
}

std::vector<Quantity> OSQuantityVector::quantities() const {
  QuantityVector result;
  BOOST_FOREACH(double value,values()) {
    result.push_back(Quantity(value,units()));
  }
  return result;
}

std::vector<double> OSQuantityVector::values() const {
  return m_values;
}

Quantity OSQuantityVector::getQuantity(unsigned i) const {
  return Quantity(m_values[i],units());
}

bool OSQuantityVector::empty() const {
  return m_values.empty();
}

unsigned OSQuantityVector::size() const {
  return m_values.size();
}

bool OSQuantityVector::setScale(int scaleExponent) {
  ScaleConstant candidate = ScaleFactory::instance().createScale(scaleExponent);
  double candidateValue = candidate().value;
  if (candidateValue == 0.0) {
    return false;
  }
  (*this) *= (scale().value / candidateValue);
  bool ok = m_units.setScale(scaleExponent);
  OS_ASSERT(ok);
  return true;
}

bool OSQuantityVector::setScale(const std::string& scaleAbbreviation) {
  ScaleConstant candidate = ScaleFactory::instance().createScale(scaleAbbreviation);
  double candidateValue = candidate().value;
  if (candidateValue == 0.0) {
    return false;
  }
  (*this) *= (scale().value / candidateValue);
  bool ok = m_units.setScale(scaleAbbreviation);
  OS_ASSERT(ok);
  return true;
}

void OSQuantityVector::push_back(Quantity q) {
  if (!(q.units() == units())) {
    LOG_AND_THROW("Quantity " << q << " is incompatible with this OSQuantityVector, which has "
                  "units " << units() << ".");
  }
  else if (q.scale() != scale()) {
    q.setScale(scale().exponent);
  }
  m_values.push_back(q.value());
}

void OSQuantityVector::pop_back() {
  m_values.pop_back();
}

void OSQuantityVector::resize(unsigned n, double value) {
  m_values.resize(n,value);
}

void OSQuantityVector::clear() {
  m_values.clear();
}

bool OSQuantityVector::isTemperature() const {
  return m_units.optionalCast<TemperatureUnit>();
}

bool OSQuantityVector::isAbsolute() const {
  if (OptionalTemperatureUnit tu = m_units.optionalCast<TemperatureUnit>()) {
    return tu->isAbsolute();
  }
  LOG_AND_THROW("Could not evaluate OSQuantityVector::isAbsolute because the units are in system "
                << system().valueName() << ", not Celsius or Fahrenheit.");
  return false;
}

bool OSQuantityVector::isRelative() const {
  return !isAbsolute();
}

void OSQuantityVector::setAsAbsolute() {
  if (OptionalTemperatureUnit tu = m_units.optionalCast<TemperatureUnit>()) {
    tu->setAsAbsolute();
  }
  else {
    LOG_AND_THROW("Could not OSQuantityVector::setAsAbsolute because the units are in system "
                  << system().valueName() << ", not Celsius or Fahrenheit.");
  }
}

void OSQuantityVector::setAsRelative() {
  if (OptionalTemperatureUnit tu = m_units.optionalCast<TemperatureUnit>()) {
    tu->setAsRelative();
  }
  else {
    LOG_AND_THROW("Could not OSQuantityVector::setAsRelative because the units are in system "
                  << system().valueName() << ", not Celsius or Fahrenheit.");
  }
}

void OSQuantityVector::lbmToLbf() {
  if (OptionalIPUnit iu = m_units.optionalCast<IPUnit>()) {
    int x = m_units.baseUnitExponent("lb_m");
    if (x != 0) {
      (*this) /= std::pow(IPUnit::gc(),x);
      iu->lbmToLbf();
    }
    OS_ASSERT(m_units.baseUnitExponent("lb_m") == 0);
  }
  LOG_AND_THROW("Cannot convert non-IP units " << m_units << " in system " << system().valueName()
                << " from lb_m to lb_f.");
}

void OSQuantityVector::lbfToLbm() {
  if (OptionalIPUnit iu = m_units.optionalCast<IPUnit>()) {
    int x = m_units.baseUnitExponent("lb_f");
    if (x != 0) {
      (*this) *= std::pow(IPUnit::gc(),x);
      iu->lbmToLbf();
    }
    OS_ASSERT(m_units.baseUnitExponent("lb_f") == 0);
  }
  LOG_AND_THROW("Cannot convert non-IP units " << m_units << " in system " << system().valueName()
                << " from lb_f to lb_m.");
}

OSQuantityVector& OSQuantityVector::operator+=(OSQuantityVector rVector) {
  if (this == &rVector) {
    (*this) *= 2.0;
    return *this;
  }

  if (units() != rVector.units()) {
    LOG_AND_THROW("Cannot add OSQuantityVectors with different units (" << units()
                  << " and " << rVector.units() << ").");
  }

  unsigned n = size();
  if (rVector.size() != n) {
    LOG_AND_THROW("Cannot add vectors of different sizes.");
  }

  if (scale() != rVector.scale()) {
    rVector.setScale(scale().exponent);
  }

  DoubleVector rValues = rVector.values();
  for (unsigned i = 0; i < n; ++i) {
    m_values[i] += rValues[i];
  }

  if (isTemperature() && rVector.isTemperature()) {
    if (!isAbsolute() && rVector.isAbsolute()) {
      setAsAbsolute();
    }
  }

  return *this;
}

OSQuantityVector& OSQuantityVector::operator+=(Quantity rQuantity) {
  if (units() != rQuantity.units()) {
    LOG_AND_THROW("Cannot add OSQuantityVector and Quantity with different units (" << units()
                  << " and " << rQuantity.units() << ").");
  }

  if (scale() != rQuantity.scale()) {
    rQuantity.setScale(scale().exponent);
  }

  double value = rQuantity.value();
  for (unsigned i = 0, n = size(); i < n; ++i) {
    m_values[i] += value;
  }

  if (isTemperature() && rQuantity.isTemperature()) {
    if (!isAbsolute() && rQuantity.isAbsolute()) {
      setAsAbsolute();
    }
  }

  return *this;
}

OSQuantityVector& OSQuantityVector::operator-=(OSQuantityVector rVector) {
  unsigned n = size();
  if (this == &rVector) {
    clear();
    resize(n,0.0);
    return *this;
  }

  if (units() != rVector.units()) {
    LOG_AND_THROW("Cannot subtract OSQuantityVectors with different units (" << units()
                  << " and " << rVector.units() << ").");
  }

  if (rVector.size() != n) {
    LOG_AND_THROW("Cannot subtract vectors of different sizes.");
  }

  if (scale() != rVector.scale()) {
    rVector.setScale(scale().exponent);
  }

  DoubleVector rValues = rVector.values();
  for (unsigned i = 0; i < n; ++i) {
    m_values[i] -= rValues[i];
  }

  if (isTemperature() && rVector.isTemperature()) {
    if (isAbsolute() && rVector.isAbsolute()) {
      // units must be the same, check that exponent on this is 1
      std::vector<std::string> bus = m_units.baseUnits();
      assert(bus.size() == 1);
      if (m_units.baseUnitExponent(bus[0]) == 1) {
        setAsRelative();
      }
    }
    else if (!isAbsolute() && rVector.isAbsolute()) {
      setAsAbsolute();
    }
  }

  return *this;
}

OSQuantityVector& OSQuantityVector::operator-=(Quantity rQuantity) {
  if (units() != rQuantity.units()) {
    LOG_AND_THROW("Cannot subtract OSQuantityVector and Quantity with different units (" << units()
                  << " and " << rQuantity.units() << ").");
  }

  if (scale() != rQuantity.scale()) {
    rQuantity.setScale(scale().exponent);
  }

  double value = rQuantity.value();
  for (unsigned i = 0, n = size(); i < n; ++i) {
    m_values[i] -= value;
  }

  if (isTemperature() && rQuantity.isTemperature()) {
    if (isAbsolute() && rQuantity.isAbsolute()) {
      // units must be the same, check that exponent on this is 1
      std::vector<std::string> bus = m_units.baseUnits();
      assert(bus.size() == 1);
      if (m_units.baseUnitExponent(bus[0]) == 1) {
        setAsRelative();
      }
    }
    else if (!isAbsolute() && rQuantity.isAbsolute()) {
      setAsAbsolute();
    }
  }

  return *this;
}

OSQuantityVector& OSQuantityVector::operator*=(const Quantity& rQuantity) {
  // unit *= will throw if m_units is not base-class and rQuantity.m_units is different system
  if ((system() != rQuantity.system()) && (system() != UnitSystem::Mixed)) {
    m_units = m_units.cloneToMixed();
  }
  // ETH@20100517 ugly workaround for temperature units
  Unit rUnits = rQuantity.units();
  OptionalTemperatureUnit tu1 = m_units.optionalCast<TemperatureUnit>();
  OptionalTemperatureUnit tu2 = rUnits.optionalCast<TemperatureUnit>();
  if (tu1 && tu2) {
    *tu1 *= *tu2;
  }
  else {
    m_units *= rUnits;
  }
  ScaleOpReturnType resultScale = scale()*rQuantity.scale();
  (*this) *= ( rQuantity.value() * resultScale.second );

  return *this;
}

OSQuantityVector& OSQuantityVector::operator/=(const Quantity& rQuantity) {
  // unit *= will throw if m_units is not base-class and rQuantity.m_units is different system
  if ((system() != rQuantity.system()) && (system() != UnitSystem::Mixed)) {
    m_units = m_units.cloneToMixed();
  }
  // ETH@20100517 ugly workaround for temperature units
  Unit rUnits = rQuantity.units();
  OptionalTemperatureUnit tu1 = m_units.optionalCast<TemperatureUnit>();
  OptionalTemperatureUnit tu2 = rUnits.optionalCast<TemperatureUnit>();
  if (tu1 && tu2) {
    *tu1 /= *tu2;
  }
  else {
    m_units /= rUnits;
  }
  ScaleOpReturnType resultScale = scale()/rQuantity.scale();
  (*this) *= ( resultScale.second / rQuantity.value() );

  return *this;
}

OSQuantityVector& OSQuantityVector::operator*=(double d) {
  for (unsigned i = 0, n = size(); i < n; ++i) {
    m_values[i] *= d;
  }
  return *this;
}

OSQuantityVector& OSQuantityVector::operator/=(double d) {
  for (unsigned i = 0, n = size(); i < n; ++i) {
    m_values[i] /= d;
  }
  return *this;
}

OSQuantityVector operator-(const OSQuantityVector& rVector) {
  OSQuantityVector result(rVector);
  result *= -1.0;
  return result;
}

OSQuantityVector operator+(const OSQuantityVector& lVector, const OSQuantityVector& rVector) {
  OSQuantityVector result(lVector);
  result += rVector;
  return result;
}

OSQuantityVector operator+(const OSQuantityVector& lVector,const Quantity& rQuantity) {
  OSQuantityVector result(lVector);
  result += rQuantity;
  return result;
}

OSQuantityVector operator+(const Quantity& lQuantity,const OSQuantityVector& rVector) {
  OSQuantityVector result(rVector);
  result += lQuantity;
  return result;
}

OSQuantityVector operator-(const OSQuantityVector& lVector, const OSQuantityVector& rVector) {
  OSQuantityVector result(lVector);
  result -= rVector;
  return result;
}

OSQuantityVector operator-(const OSQuantityVector& lVector, const Quantity& rQuantity) {
  OSQuantityVector result(lVector);
  result -= rQuantity;
  return result;
}

OSQuantityVector operator-(const Quantity& lQuantity, const OSQuantityVector& rVector) {
  OSQuantityVector result(rVector);
  result = -result;
  result += lQuantity;
  return result;
}

OSQuantityVector operator*(const OSQuantityVector& lVector, const Quantity& rQuantity) {
  OSQuantityVector result(lVector);
  result *= rQuantity;
  return result;
}

OSQuantityVector operator*(const OSQuantityVector& lVector, double d) {
  OSQuantityVector result(lVector);
  result *= d;
  return result;
}

OSQuantityVector operator*(const Quantity& lQuantity, const OSQuantityVector& rVector) {
  OSQuantityVector result(rVector);
  result *= lQuantity;
  return result;
}

OSQuantityVector operator*(double d, const OSQuantityVector& rVector) {
  OSQuantityVector result(rVector);
  result *= d;
  return result;
}

OSQuantityVector operator/(const OSQuantityVector& lVector,const Quantity& rQuantity) {
  OSQuantityVector result(lVector);
  result /= rQuantity;
  return result;
}

OSQuantityVector operator/(const OSQuantityVector& lVector, double d) {
  OSQuantityVector result(lVector);
  result /= d;
  return result;
}

Quantity dot(OSQuantityVector lVector, const OSQuantityVector& rVector) {
  Unit lUnits(lVector.units()), rUnits(rVector.units());
  OptionalTemperatureUnit ltu = lUnits.optionalCast<TemperatureUnit>();
  OptionalTemperatureUnit rtu = rUnits.optionalCast<TemperatureUnit>();
  Unit resultUnits;
  if (ltu && rtu) {
    resultUnits = ltu.get() * rtu.get();
  }
  else {
    resultUnits = lUnits * rUnits;
  }
  ScaleOpReturnType resultScale = lVector.scale() * rVector.scale();
  lVector *= resultScale.second;
  DoubleVector lValues(lVector.values()), rValues(rVector.values());
  double resultValue = dot(createVector(lValues),createVector(rValues));
  return Quantity(resultValue,resultUnits);
}

bool operator==(const OSQuantityVector& lVector,const OSQuantityVector& rVector) {
  return (&lVector == &rVector) || ((lVector.units() == rVector.units()) && (lVector.values() == rVector.values()));
}

bool operator!=(const OSQuantityVector& lVector,const OSQuantityVector& rVector) {
  return !(lVector == rVector);
}

} // openstudio
