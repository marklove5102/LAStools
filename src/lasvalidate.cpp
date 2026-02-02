/*
===============================================================================

  FILE:  lasvalidate.cpp
  
  CONTENTS:
  
    A tool to validate whether a LAS file conforms to the LAS specification

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2016, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     21 January 2026 -- The tool has been renewed and version 1.5 check added, as well as json output
     2 August 2015 -- not failing but warning if OCG WRT has intentional empty payload 
    12 April 2015 -- not failing but warning for certain empty VLR payloads 
    20 March 2015 -- fail on files containing zero point records
    26 January 2015 -- more useful reports if CRS always missing with '-no_CRS_fail'
     3 September 2013 -- made open source after the ASPRS LVS contract fiasko
     1 April 2013 -- on Easter Monday all-nighting in Perth airport for PER->SYD
  
===============================================================================
*/

#include <cstdint>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>

#include "mydefs.hpp"
#include "lastool.hpp"
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "geoprojectionconverter.hpp"
#include "lasvalidationresult.hpp"
#include "xmlwriter.hpp"
#include "lascheck.hpp"
#include "lasdefinitions.hpp"

#define VALIDATE_VERSION  260128

class LasTool_lasvalidate : public LasTool {
 private:
 public:
  void usage() override {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "lasvalidate -i lidar.las\n");
    fprintf(stderr, "lasvalidate -i lidar.laz -no_CRS_fail\n");
    fprintf(stderr, "lasvalidate -v -i lidar.las -o report.xml\n");
    fprintf(stderr, "lasvalidate -v -i lidar.laz -oxml\n");
    fprintf(stderr, "lasvalidate -vv -i tile1.las tile2.las tile3.las -oxml\n");
    fprintf(stderr, "lasvalidate -i tile1.laz tile2.laz tile3.laz -o summary.xml\n");
    fprintf(stderr, "lasvalidate -i *.las -no_CRS_fail -o report.xml\n");
    fprintf(stderr, "lasvalidate -i *.laz -o summary.xml\n");
    fprintf(stderr, "lasvalidate -i *.laz -tile_size 1000 -o summary.xml\n");
    fprintf(stderr, "lasvalidate -i *.las -oxml\n");
    fprintf(stderr, "lasvalidate -i c:\\data\\lidar.las -oxml\n");
    fprintf(stderr, "lasvalidate -i ..\\subfolder\\*.las -o summary.xml\n");
    fprintf(stderr, "lasvalidate -v -i ..\\..\\flight\\*.laz -o oxml\n");
    fprintf(stderr, "lasvalidate -h\n");
  }
};

static void write_version(XMLwriter& xmlwriter)
{
  std::ostringstream version_oss;
  version_oss << "lasvalidate (" << VALIDATE_VERSION << ") built with LAStools version " << LAS_TOOLS_VERSION;

  std::string version = version_oss.str();

  xmlwriter.begin("version");
  xmlwriter.write(version);
  xmlwriter.end("version");
}

static void write_command_line(XMLwriter& xmlwriter, int argc, char *argv[])
{
  std::ostringstream command_line_oss;
  for (int i = 0; i < argc; i++) {
    if (!argv[i]) continue;

    if (i > 0) command_line_oss << ' ';

    if (std::strchr(argv[i], ' '))
      command_line_oss << '"' << argv[i] << '"';
    else
      command_line_oss << argv[i];
  }

  std::string command_line = command_line_oss.str();

  xmlwriter.begin("command_line");
  xmlwriter.write(command_line);
  xmlwriter.end("command_line");
}

static double taketime()
{
  return (double)(clock())/CLOCKS_PER_SEC;
}

#ifdef COMPILE_WITH_MULTI_CORE
extern void lasvalidate_multi_core(
    int argc, char* argv[], GeoProjectionConverter* geoprojectionconverter, LASreadOpener* lasreadopener, LASwriteOpener* laswriteopener, int cores,
    BOOL cpu64);
#endif

int main(int argc, char *argv[])
{
  LasTool_lasvalidate lastool;
  lastool.init(argc, argv, "lasvalidate");
  size_t i = 0;
  double start_time = 0.0;
  double full_start_time = 0.0;
  std::string xml_output_file;
  BOOL no_CRS_fail = FALSE;
  F64 tile_size = 0.0;
  BOOL one_report_per_file = FALSE;
  U32 num_pass = 0;
  U32 num_fail = 0;
  U32 num_warning = 0;

  //fprintf(stderr, "This is version %d of the LAS validator. Please contact\n", VALIDATE_VERSION);
  //fprintf(stderr, "me at 'martin.isenburg@rapidlasso.com' if you disagree with\n");
  //fprintf(stderr, "validation reports, want additional checks, or find bugs as\n");
  //fprintf(stderr, "the software is still under development. Your feedback will\n");
  //fprintf(stderr, "help to finish it sooner.\n");

  //LAShistogram lashistogram;
  LASreadOpener lasreadopener;
  GeoProjectionConverter geoprojectionconverter;
  LASwriteOpener laswriteopener;

  if (argc == 1)
  {
#ifdef COMPILE_WITH_GUI
    lasvalidate_gui(argc, argv, 0);
#else
    wait_on_exit();
    fprintf(stderr, "%s is better run in the command line\n", argv[0]);
    std::string file_name;
    fprintf(stderr, "enter input file: ");
    std::getline(std::cin, file_name);
    if (!file_name.empty() && (file_name.back() == '\n' || file_name.back() == '\r')) {
      file_name.pop_back();
    }
    lasreadopener.set_file_name(file_name.c_str());
    //laswriteopener.set_file_name(file_name);
    xml_output_file = std::string(file_name);
#endif
  } else {
    for (i = 1; i < argc; i++) {
      if ((unsigned char)argv[i][0] == 0x96) argv[i][0] = '-';
    }
    //if (!lashistogram.parse(argc, argv)) byebye();
    lasreadopener.parse(argc, argv);
    geoprojectionconverter.parse(argc, argv);
    laswriteopener.parse(argc, argv);
  }

  auto arg_local = [&](int& i) -> bool {
    /*if (strcmp(argv[i],"-version") == 0) {
      LASMessage(LAS_INFO, "lasvalidate %d with LAStools (v %d) and LAScheck (v %d.%d %d) by rapidlasso GmbH", VALIDATE_VERSION, LAS_TOOLS_VERSION,
          LASCHECK_VERSION_MAJOR, LASCHECK_VERSION_MINOR, LASCHECK_BUILD_DATE);
      byebye();
    } else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0) {
      lasreadopener.usage();
    } else*/ if (strcmp(argv[i],"-lof") == 0) {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs 1 argument: list_of_files", argv[i]);
      }
      std::ifstream file(argv[i + 1]);
      if (!file) {
        laserror("cannot open '%s'", argv[i + 1]);
      }
      std::string line;
      while (std::getline(file, line)) {
        // remove trailing whitespace and line endings
        while (!line.empty() && (line.back() == ' ' || line.back() == '\t' || line.back() == '\n' || line.back() == '\r')) {
          line.pop_back();
        }

        if (!line.empty()) {
          lasreadopener.add_file_name(line.c_str());
        }
      }
      i += 1;
    } else if (strcmp(argv[i],"-o") == 0) {
      i++;
      xml_output_file = std::string(argv[i]);
    } else if (strcmp(argv[i],"-oxml") == 0) {
      one_report_per_file = TRUE;
    } else if (strcmp(argv[i],"-no_CRS_fail") == 0) {
      no_CRS_fail = TRUE;
    } else if (strcmp(argv[i],"-tile_size") == 0) {
      if ((i+1) >= argc)
      {
        laserror("'%s' needs at least 1 argument: tile size", argv[i]);
      }
      i++;
      tile_size = atof(argv[i]);
    } else {
      return false;
    }
    return true;
  };

  lastool.parse(arg_local);

#ifdef COMPILE_WITH_MULTI_CORE
  if (lastool.cores > 1) {
    if (lasreadopener.get_use_stdin()) {
      LASMessage(LAS_WARNING, "using stdin. ignoring '-cores %d' ...", lastool.cores);
    } else if (lasreadopener.get_file_name_number() < 2) {
      LASMessage(LAS_WARNING, "only %u input files. ignoring '-cores %d' ...", lasreadopener.get_file_name_number(), lastool.cores);
    } else if (lasreadopener.is_merged()) {
      LASMessage(LAS_WARNING, "input files merged on-the-fly. ignoring '-cores %d' ...", lastool.cores);
    } else {
      lasvalidate_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, lastool.cores, lastool.cpu64);
    }
  }
  if (lastool.cpu64) {
    lasvalidate_multi_core(argc, argv, &geoprojectionconverter, &lasreadopener, &laswriteopener, 1, TRUE);
  }
#endif

  // check input

  if (!lasreadopener.active()) {
    laserror("no input specified");
  }

  // output logging

  XMLwriter xmlwriter;

  if (!xml_output_file.empty())
  {
    one_report_per_file = FALSE;
  }
  else if (!one_report_per_file)
  {
    xml_output_file = "validate.xml";
  }

  // maybe we are doing one summary report

  if (!xml_output_file.empty())
  {
    if (!xmlwriter.open(xml_output_file, "LASvalidator"))
    {
      laserror("The LAS validator does not have write permission to the specified output directory");
    }
  }

  // accumulated pass

  ValidationStatus total_pass = ValidationStatus::passed;

  full_start_time = taketime();

  // possibly loop over multiple input files

  while (lasreadopener.active())
  {
    try {
      start_time = taketime();
      
      // open lasreader
      
      LASreader* lasreader = lasreadopener.open();
      if (lasreader == nullptr) {
        laserror("cannot open lasreader");
        continue;
      }
      
      // get a pointer to the header
      LASheader* lasheader = &lasreader->header;
      ValidationResult results;
      
      // maybe we are doing one report per file
      
      if (one_report_per_file)
      {
        std::string current_xml_output_file = lasreadopener.get_file_name();
        // if decimal_digits > 8, fall back to old function without scale
        if (current_xml_output_file.size() >= 4) {
          current_xml_output_file.replace(current_xml_output_file.size() - 4, 4, "_LVS.xml");
        } else {
          // fallback
          current_xml_output_file += "_LVS.xml";
        }
        if (!xmlwriter.open(current_xml_output_file, "LASvalidator"))
        {
          laserror("The LAS validator does not have write permission to the specified output directory");
        }
      }
      
      // start a new report
      
      xmlwriter.begin("report");
      
      // report description of file
      
      xmlwriter.beginsub("file");
      xmlwriter.write("name", lasreadopener.get_file_name_current());
      xmlwriter.write("path", lasreadopener.get_file_name_base());
      std::string version = std::to_string(lasheader->version_major) + "." + std::to_string(lasheader->version_minor);
      xmlwriter.write("version", version);
      std::string system_identifier = lasheader->system_identifier;
      xmlwriter.write("system_identifier", system_identifier.c_str());
      std::string generating_software = lasheader->generating_software;
      xmlwriter.write("generating_software", generating_software.c_str());
      xmlwriter.write("point_data_format", lasheader->point_data_format);
      
      std::string crsdescription = "not valid or not specified";
      
      if (lasheader != nullptr)
      {
        // header was loaded. now parse and check.
      
        LAScheck lascheck(lasheader);
      
        while (lasreader->read_point())
        {
          lascheck.check_parse(&lasreader->point, results);
        }
      
        // check header and points and get CRS description
      
        lascheck.check(results, crsdescription, no_CRS_fail);
      }
      
      xmlwriter.write("CRS", crsdescription);
      xmlwriter.endsub("file");    
      
      // report the verdict
      
      xmlwriter.beginsub("summary");
      xmlwriter.write((results.status == ValidationStatus::passed ? "pass" : ((results.status == ValidationStatus::failed) ? "fail" : "warning")));
      xmlwriter.endsub("summary");
      
      // report details (if necessary)
      
      if (results.status != ValidationStatus::passed)
      {
        xmlwriter.beginsub("details");
        for (i = 0; i < results.fail_messages.size(); ++i)
        {
          const ValidationMessage& msg = results.fail_messages[i];
          xmlwriter.write(msg.key.c_str(), "fail", msg.note.c_str());
        }
        for (i = 0; i < results.warning_messages.size(); ++i) {
          const ValidationMessage& msg = results.warning_messages[i];
          xmlwriter.write(msg.key.c_str(), "warning", msg.note.c_str());
        }
        xmlwriter.endsub("details");
        if (results.status == ValidationStatus::failed) {
          total_pass = ValidationStatus::failed;
        } else if (results.status == ValidationStatus::warning && total_pass == ValidationStatus::passed) {
          total_pass = ValidationStatus::warning;
        }
        if (results.status == ValidationStatus::failed)
        {
          num_fail++;
        }
        else
        {
          num_warning++;
        }
      }
      else
      {
        num_pass++;
      }
      
      // end the report
      
      xmlwriter.end("report");
      
      // maybe we are doing one report per file
      
      if (one_report_per_file)
      {
        // report the total verdict
      
        xmlwriter.begin("total");
        xmlwriter.write((total_pass == ValidationStatus::passed ? "pass" : (total_pass == ValidationStatus::failed ? "fail" : "warning")));
        xmlwriter.beginsub("details");
        xmlwriter.write("pass", num_pass);
        xmlwriter.write("warning", num_warning);
        xmlwriter.write("fail", num_fail);
        xmlwriter.endsub("details");
        xmlwriter.end("total");
      
        num_pass = 0;
        num_warning = 0;
        num_fail = 0;
      
        // write which validator was used
      
        write_version(xmlwriter);
      
        // write which command line was used
      
        write_command_line(xmlwriter, argc, argv);
      
        // close the LASvalidator XML output file
      
        xmlwriter.close("LASvalidator");
      }
      
      lasreader->close();
      delete lasreader;
      LASMessage(LAS_VERBOSE, "needed %.2f sec for '%s' %s", taketime() - start_time, lasreadopener.get_file_name(),
          (results.status == ValidationStatus::passed ? "pass" : (results.status == ValidationStatus::failed ? "fail" : "warning")));
      start_time = taketime();
    }
    catch (...) {
      laserror("processing file '%s'. maybe file is corrupt?", lasreadopener.get_file_name());
    }
  }

  // maybe we are doing one summary report

  if (!one_report_per_file)
  {
    // report the total verdict

    xmlwriter.begin("total");
    xmlwriter.write((total_pass == ValidationStatus::passed ? "pass" : (total_pass == ValidationStatus::failed ? "fail" : "warning")));
    xmlwriter.beginsub("details");
    xmlwriter.write("pass", num_pass);
    xmlwriter.write("warning", num_warning);
    xmlwriter.write("fail", num_fail);
    xmlwriter.endsub("details");
    xmlwriter.end("total");

    // write which validator was used

    write_version(xmlwriter);

    // write which command line was used

    write_command_line(xmlwriter, argc, argv);

    // close the LASvalidator XML output file

    xmlwriter.close("LASvalidator");
  }
  if (lasreadopener.get_file_name_number() > 1) {
    LASMessage(LAS_VERBOSE, "done. total time %.2f sec. total %s (pass=%d,warning=%d,fail=%d)", taketime() - full_start_time,
        (total_pass == ValidationStatus::passed ? "pass" : (total_pass == ValidationStatus::failed ? "fail" : "warning")), 
        num_pass, num_warning, num_fail);
  }

  byebye();
}
