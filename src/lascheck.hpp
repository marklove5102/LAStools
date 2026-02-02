/*
===============================================================================

  FILE:  lascheck.hpp
  
  CONTENTS:
  
    A set of functions to check LAS headers and points for being specification
    conform. Supported are LAS formats from 1.0 to 1.4 ...
  
  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2020, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     21 January 2026 -- The tool has been renewed and version 1.5 check added, as well as json output
     4 January 2020 -- optional check for heaber bounding box matching tile size
     1 April 2013 -- on Easter Monday all-nighting in Perth airport for PER->SYD
  
===============================================================================
*/
#ifndef LAS_CHECK_HPP
#define LAS_CHECK_HPP

#include "lasdefinitions.hpp"
#include "lasutility.hpp"
#include "lasvalidationresult.hpp"
#include <array>

constexpr std::array<double, 25> allowed_scale_factors = {1.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001, 0.00000001, 
                                                          0.25, 0.025, 0.0025, 0.00025, 0.000025, 0.0000025, 0.00000025, 0.000000025,
                                                          0.5, 0.05, 0.005, 0.0005, 0.00005, 0.000005, 0.0000005, 0.00000005};
#define LASCHECK_VERSION_MAJOR 1
#define LASCHECK_VERSION_MINOR 1
#define LASCHECK_BUILD_DATE 260128

class LAScheck
{
public:

  void check_parse(const LASpoint* laspoint, ValidationResult& results);
  void check(ValidationResult& results, std::string& crsdescription, BOOL no_CRS_fail = FALSE, F64 tile_size = 0.0);

  LAScheck(const LASheader* lasheader);
  ~LAScheck();

private:
  const LASheader *lasheader;
  F64 min_x, min_y, min_z;
  F64 max_x, max_y, max_z;
  I64 points_outside_bounding_box;
  LASinventory lasinventory;
  LASsummary lassummary;
  std::ostringstream problem_oss;
  std::ostringstream note_oss;
};

#endif
