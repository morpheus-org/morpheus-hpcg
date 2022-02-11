
//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER

/* ************************************************************************
 * Modifications (c) 2022 The University of Edinburgh
 *
 * EPCC, The University of Edinburgh
 *
 * Contributing Authors:
 * Christodoulos Stylianou (c.stylianou@ed.ac.uk)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ************************************************************************ */

/*!
 @file YAML_Element.cpp

 HPCG routine
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include "YAML_Element.hpp"
using namespace std;
YAML_Element::YAML_Element(const std::string& key_arg,
                           const std::string& value_arg) {
  key   = key_arg;
  value = value_arg;
}

YAML_Element::~YAML_Element() {
  for (size_t i = 0; i < children.size(); i++) {
    delete children[i];
  }
  children.clear();
}

/*!
  Add an element to the vector
  QUESTION: if an element is not added because the key already exists,
  will this lead to memory leakage?

  @param[in] key_arg   The key under which the element is stored
  @param[in] value_arg The value of the element

  @return Returns the added element
*/
YAML_Element* YAML_Element::add(const std::string& key_arg, double value_arg) {
  this->value            = "";
  string converted_value = convert_double_to_string(value_arg);
  YAML_Element* element  = new YAML_Element(key_arg, converted_value);
  children.push_back(element);
  return element;
}

/*!
  Add an element to the vector

  @param[in] key_arg   The key under which the element is stored
  @param[in] value_arg The value of the element

  @return Returns the added element
*/
YAML_Element* YAML_Element::add(const std::string& key_arg, int value_arg) {
  this->value            = "";
  string converted_value = convert_int_to_string(value_arg);
  YAML_Element* element  = new YAML_Element(key_arg, converted_value);
  children.push_back(element);
  return element;
}

#ifndef HPCG_NO_LONG_LONG

/*!
  Add an element to the vector

  @param[in] key_arg   The key under which the element is stored
  @param[in] value_arg The value of the element

  @return Returns the added element
*/
YAML_Element* YAML_Element::add(const std::string& key_arg,
                                long long value_arg) {
  this->value            = "";
  string converted_value = convert_long_long_to_string(value_arg);
  YAML_Element* element  = new YAML_Element(key_arg, converted_value);
  children.push_back(element);
  return element;
}

#endif

/*!
  Add an element to the vector

  @param[in] key_arg   The key under which the element is stored
  @param[in] value_arg The value of the element

  @return Returns the added element
*/
YAML_Element* YAML_Element::add(const std::string& key_arg, size_t value_arg) {
  this->value            = "";
  string converted_value = convert_size_t_to_string(value_arg);
  YAML_Element* element  = new YAML_Element(key_arg, converted_value);
  children.push_back(element);
  return element;
}

/*!
  Add an element to the vector

  @param[in] key_arg   The key under which the element is stored
  @param[in] value_arg The value of the element

  @return Returns the added element
*/
YAML_Element* YAML_Element::add(const std::string& key_arg,
                                const std::string& value_arg) {
  this->value           = "";
  YAML_Element* element = new YAML_Element(key_arg, value_arg);
  children.push_back(element);
  return element;
}

/*!
  Returns the pointer to the YAML_Element for the given key.
  @param[in] key_arg   The key under which the element was stored

  @return If found, returns the element, otherwise returns NULL
*/
YAML_Element* YAML_Element::get(const std::string& key_arg) {
  for (size_t i = 0; i < children.size(); i++) {
    if (children[i]->getKey() == key_arg) {
      return children[i];
    }
  }
  return 0;
}

/*!
  Prints a line of a YAML document.  Correct YAML depends on
  correct spacing; the parameter space should be the proper
  amount of space for the parent element

  @param[in] space spacing inserted at the beginning of the line

  @return Returns a single line of the YAML document without the leading white
  space
*/
string YAML_Element::printYAML(std::string space) {
  string yaml_line = space + key + ": " + value + "\n";
  for (int i = 0; i < 2; i++) space = space + " ";
  for (size_t i = 0; i < children.size(); i++) {
    yaml_line = yaml_line + children[i]->printYAML(space);
  }
  return yaml_line;
}

/*!
  Converts a double precision value to a string.

  @param[in] value_arg The value to be converted.
*/
string YAML_Element::convert_double_to_string(double value_arg) {
  stringstream strm;
  strm << value_arg;
  return strm.str();
}

/*!
  Converts a integer value to a string.

  @param[in] value_arg The value to be converted.
*/
string YAML_Element::convert_int_to_string(int value_arg) {
  stringstream strm;
  strm << value_arg;
  return strm.str();
}

#ifndef HPCG_NO_LONG_LONG

/*!
  Converts a "long long" integer value to a string.

  @param[in] value_arg The value to be converted.
*/
string YAML_Element::convert_long_long_to_string(long long value_arg) {
  stringstream strm;
  strm << value_arg;
  return strm.str();
}

#endif

/*!
  Converts a "size_t" integer value to a string.

  @param[in] value_arg The value to be converted.
*/
string YAML_Element::convert_size_t_to_string(size_t value_arg) {
  stringstream strm;
  strm << value_arg;
  return strm.str();
}
