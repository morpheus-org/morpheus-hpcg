
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

#include <fstream>
#include <list>
#include <sstream>
#include <string>

#include "OutputFile.hpp"

using std::list;
using std::ofstream;
using std::string;
using std::stringstream;

OutputFile::OutputFile(const string& name_arg, const string& version_arg)
    : name(name_arg), version(version_arg), eol("\n"), keySeparator("::") {}

OutputFile::OutputFile(void) : eol("\n"), keySeparator("::") {}

OutputFile::~OutputFile() {
  for (list<OutputFile*>::iterator it = descendants.begin();
       it != descendants.end(); ++it) {
    delete *it;
  }
}

void OutputFile::add(const string& key_arg, const string& value_arg) {
  descendants.push_back(allocKeyVal(key_arg, value_arg));
}

void OutputFile::add(const string& key_arg, double value_arg) {
  stringstream ss;
  ss << value_arg;
  descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

void OutputFile::add(const string& key_arg, int value_arg) {
  stringstream ss;
  ss << value_arg;
  descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

#ifndef HPCG_NO_LONG_LONG

void OutputFile::add(const string& key_arg, long long value_arg) {
  stringstream ss;
  ss << value_arg;
  descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

#endif

void OutputFile::add(const string& key_arg, size_t value_arg) {
  stringstream ss;
  ss << value_arg;
  descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

void OutputFile::setKeyValue(const string& key_arg, const string& value_arg) {
  key   = key_arg;
  value = value_arg;
}

OutputFile* OutputFile::get(const string& key_arg) {
  for (list<OutputFile*>::iterator it = descendants.begin();
       it != descendants.end(); ++it) {
    if ((*it)->key == key_arg) return *it;
  }

  return 0;
}

string OutputFile::generateRecursive(string prefix) {
  string result = "";

  result += prefix + key + "=" + value + eol;

  for (list<OutputFile*>::iterator it = descendants.begin();
       it != descendants.end(); ++it) {
    result += (*it)->generateRecursive(prefix + key + keySeparator);
  }

  return result;
}

string OutputFile::generate(void) {
  string result = name + "\nversion=" + version + eol;

  for (list<OutputFile*>::iterator it = descendants.begin();
       it != descendants.end(); ++it) {
    result += (*it)->generateRecursive("");
  }

  time_t rawtime;
  time(&rawtime);
  tm* ptm = localtime(&rawtime);
  char sdate[25];
  // use tm_mon+1 because tm_mon is 0 .. 11 instead of 1 .. 12
  sprintf(sdate, "%04d-%02d-%02d_%02d-%02d-%02d", ptm->tm_year + 1900,
          ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min,
          ptm->tm_sec);

  string filename = name + "_" + version + "_";
  filename += string(sdate) + ".txt";

  ofstream myfile(filename.c_str());
  myfile << result;
  myfile.close();

  return result;
}

OutputFile* OutputFile::allocKeyVal(const std::string& key_arg,
                                    const std::string& value_arg) {
  OutputFile* of = new OutputFile();
  of->setKeyValue(key_arg, value_arg);
  return of;
}
