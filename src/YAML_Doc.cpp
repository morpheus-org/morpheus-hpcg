
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

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include "YAML_Doc.hpp"
using namespace std;

/*!
  Sets the application name and version which will become part of the YAML doc.

  @param[in] miniApp_Name application name
  @param[in] miniApp_Version application name
  @param[in] destination_Directory destination directory for the YAML document
  @param[in] destination_FileName file name for the YAML document
*/
YAML_Doc::YAML_Doc(const std::string& miniApp_Name,
                   const std::string& miniApp_Version,
                   const std::string& destination_Directory,
                   const std::string& destination_FileName) {
  miniAppName          = miniApp_Name;
  miniAppVersion       = miniApp_Version;
  destinationDirectory = destination_Directory;
  destinationFileName  = destination_FileName;
}

// inherits the destructor from YAML_Element
YAML_Doc::~YAML_Doc(void) {}

/*!
  Generates YAML from the elements of the document and saves it to a file.

  @return returns the complete YAML document as a string
*/
string YAML_Doc::generateYAML() {
  string yaml;

  yaml = yaml + miniAppName + " version: " + miniAppVersion + "\n";

  for (size_t i = 0; i < children.size(); i++) {
    yaml = yaml + children[i]->printYAML("");
  }

  time_t rawtime;
  tm* ptm;
  time(&rawtime);
  ptm = localtime(&rawtime);
  char sdate[25];
  // use tm_mon+1 because tm_mon is 0 .. 11 instead of 1 .. 12
  sprintf(sdate, "%04d.%02d.%02d.%02d.%02d.%02d", ptm->tm_year + 1900,
          ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min,
          ptm->tm_sec);

  string filename;
  if (destinationFileName == "")
    filename = miniAppName + "-" + miniAppVersion + "_";
  else
    filename = destinationFileName;
  filename = filename + string(sdate) + ".yaml";
  if (destinationDirectory != "" && destinationDirectory != ".") {
    string mkdir_cmd = "mkdir " + destinationDirectory;
    system(mkdir_cmd.c_str());
    filename = destinationDirectory + "/" + destinationFileName;
  } else
    filename = "./" + filename;

  ofstream myfile;
  myfile.open(filename.c_str());
  myfile << yaml;
  myfile.close();
  return yaml;
}
