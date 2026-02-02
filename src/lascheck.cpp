/*
===============================================================================

  FILE:  lascheck.cpp
  
  CONTENTS:
  
    see corresponding header file

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
  
    see corresponding header file
  
===============================================================================
*/

#include <ctime>
#include <string.h>
#include <iomanip> 
#include <cmath>

#include "lascheck.hpp"
#include "mydefs.hpp"
#include "crscheck.hpp"

/// Formats a double value, removes trailing zeros, and writes to string.
static I32 lidardouble2string(std::string& string, F64 value) {
  char buffer[64];
  int len = std::snprintf(buffer, sizeof(buffer), "%.15f", value);
  if (len <= 0 || len >= static_cast<int>(sizeof(buffer))) return -1;

  int last_char_index = len - 1;
  while (last_char_index > 0 && buffer[last_char_index] == '0') --last_char_index;

  if (buffer[last_char_index] != '.') ++last_char_index;
  buffer[last_char_index] = '\0';

  string = buffer;
  return last_char_index;
};

/// Formats a double value according to the given scaling and writes it to string.
static I32 lidardouble2string(std::string& string, F64 value, F64 scale) {
  I32 decimal_digits = 0;
  while (scale < 1.0) {
    scale *= 10.0;
    ++decimal_digits;
  }

  char buffer[64];
  int len = 0;

  if (decimal_digits == 0) {
    len = std::snprintf(buffer, sizeof(buffer), "%d", static_cast<I32>(value));
  } else if (decimal_digits <= 8) {
    len = std::snprintf(buffer, sizeof(buffer), "%.*f", decimal_digits, value);
  } else {
    return lidardouble2string(string, value);
  }

  if (len <= 0 || len >= static_cast<int>(sizeof(buffer))) return -1;

  string = buffer;
  return len - 1;
};

/// Checks and collects point information
void LAScheck::check_parse(const LASpoint* laspoint, ValidationResult& results) {
  // add point to inventory
  lasinventory.add(laspoint);
  lassummary.add(laspoint);

  // check point against bounding box
  if (!laspoint->inside_bounding_box(min_x, min_y, min_z, max_x, max_y, max_z))
  {
    points_outside_bounding_box++;
  }

  // Return Number / Number of Returns consistency check
  // point_data_format 0–5 : legacy return fields
  if (lasheader->point_data_format <= 5) {
    const U8 return_number = laspoint->get_return_number();
    const U8 number_of_returns = laspoint->get_number_of_returns();

     if (number_of_returns == 0 || return_number == 0 || return_number > number_of_returns) {
      set_oss_content(note_oss, "invalid return number (", return_number, ") / number of returns (", number_of_returns, ") combination");
      results.add_fail("return number", note_oss.str());
    }
  }
  // point_data_format 6–10 : extended return fields (LAS 1.4)
  else if (lasheader->point_data_format >= 6 && lasheader->point_data_format <= 10) {
    const U8 extended_return_number = laspoint->get_extended_return_number();
    const U8 extended_number_of_returns = laspoint->get_extended_number_of_returns();

    if (extended_number_of_returns == 0 || extended_return_number == 0 || extended_return_number > extended_number_of_returns) {
      set_oss_content(note_oss, "invalid extended return number (", extended_return_number, ") / extended number of returns (", 
          extended_number_of_returns, ") combination");
      results.add_fail("return number", note_oss.str());
    }
  }
}

/// Run the lasheader and laspoint validation check
void LAScheck::check(ValidationResult& results, std::string& crsdescription, BOOL no_CRS_fail, F64 tile_size) {
  U32 i = 0;
  U32 j = 0;

  // check file signature

  if ((lasheader->file_signature[0] != 'L') || (lasheader->file_signature[1] != 'A') || (lasheader->file_signature[2] != 'S') || (lasheader->file_signature[3] != 'F'))
  {
    set_oss_content(note_oss, "should be 'LASF' and not '", lasheader->file_signature, "'");
    results.add_fail("file signature", note_oss.str());
  }

  // check global encoding

  if ((lasheader->version_major == 1) && (lasheader->version_minor <= 1))
  {
    if (lasheader->global_encoding > 0)
    {
      set_oss_content(note_oss, "should 0 for LAS version ", lasheader->version_major, ".", lasheader->version_minor,
          " but is ", lasheader->global_encoding);
      results.add_fail("global encoding", note_oss.str());
    }
  }
  else if ((lasheader->version_major == 1) && (lasheader->version_minor <= 2))
  {
    if (lasheader->global_encoding > 1)
    {
      set_oss_content(note_oss, "should not be greater than 1 for LAS version ", lasheader->version_major, ".",
          lasheader->version_minor, " but is ", lasheader->global_encoding);
      results.add_fail("global encoding", note_oss.str());
    }
  }
  else if ((lasheader->version_major == 1) && (lasheader->version_minor <= 3))
  {
    if (lasheader->global_encoding > 15)
    {
      set_oss_content(note_oss, "should not be greater than 1 for LAS version ", lasheader->version_major, ".", 
          lasheader->version_minor, " but is ", lasheader->global_encoding);
      results.add_fail("global encoding", note_oss.str());
    }
  }
  else if ((lasheader->version_major == 1) && (lasheader->version_minor <= 4))
  {
    if (lasheader->global_encoding > 31)
    {
      set_oss_content(note_oss, "should not be greater than 1 for LAS version ", lasheader->version_major, ".", 
          lasheader->version_minor, " but is ", lasheader->global_encoding);
      results.add_fail("global encoding", note_oss.str());
    }
  }

  if (lasheader->global_encoding & 16)
  {
    if ((lasheader->version_major == 1) && (lasheader->version_minor <= 3))
    {
      set_oss_content(note_oss, "set bit 4 not defined for LAS version ", lasheader->version_major, ".", lasheader->version_minor);
      results.add_fail("global encoding", note_oss.str());
    }
  }
  else
  {
    if ((lasheader->version_major == 1) && (lasheader->version_minor >= 4) && (lasheader->point_data_format >= 6))
    {
      set_oss_content(note_oss, "bit 4 must be set (OGC WKT must be used) for point data format ", lasheader->point_data_format);
      results.add_fail("global encoding", note_oss.str());
    }
  }

  if (lasheader->global_encoding & 8)
  {
    if ((lasheader->version_major == 1) && (lasheader->version_minor <= 2))
    {
      set_oss_content(note_oss, "set bit 3 not defined for LAS version ", lasheader->version_major, ".", lasheader->version_minor);
      results.add_fail("global encoding", note_oss.str());
    }
  }

  if (lasheader->global_encoding & 4)
  {
    if ((lasheader->version_major == 1) && (lasheader->version_minor <= 2))
    {
      set_oss_content(note_oss, "set bit 2 not defined for LAS version ", lasheader->version_major, ".", lasheader->version_minor);
      results.add_fail("global encoding", note_oss.str());
    }
    if ((lasheader->point_data_format != 4) && (lasheader->point_data_format != 5) && (lasheader->point_data_format != 9) && (lasheader->point_data_format != 10))
    {
      set_oss_content(note_oss, "set bit 2 not defined for point data format ", lasheader->point_data_format);
      results.add_fail("global encoding", note_oss.str());
    }
    if (lasheader->global_encoding & 2)
    {
      set_oss_content(note_oss, "although bit 1 and bit 2 are mutually exclusive they are both set");
      results.add_fail("global encoding", note_oss.str());
    }
  }
  else if ((lasheader->version_major == 1) && (lasheader->version_minor >= 3))
  {
    if ((lasheader->point_data_format == 4) || (lasheader->point_data_format == 5) || (lasheader->point_data_format == 9) || (lasheader->point_data_format == 10))
    {
      if ((lasheader->global_encoding & 2) == 0)
      {
        set_oss_content(note_oss, "neither bit 1 nor bit 2 are set for point data format ", lasheader->point_data_format);
        results.add_fail("global encoding", note_oss.str());
      }
    }
  }

  if (lasheader->global_encoding & 2)
  {
    if ((lasheader->version_major == 1) && (lasheader->version_minor <= 2))
    {
      set_oss_content(note_oss, "set bit 1 not defined for LAS version ", lasheader->version_major, ".", lasheader->version_minor);
      results.add_fail("global encoding", note_oss.str());
    }
    if ((lasheader->point_data_format != 4) && (lasheader->point_data_format != 5) && (lasheader->point_data_format != 9) && (lasheader->point_data_format != 10))
    {
      set_oss_content(note_oss, "set bit 1 not defined for point data format ", lasheader->point_data_format);
      results.add_fail("global encoding", note_oss.str());
    }
  }

  if (lasheader->global_encoding & 1)
  {
    if ((lasheader->version_major == 1) && (lasheader->version_minor <= 1))
    {
      set_oss_content(note_oss, "set bit 0 not defined for LAS version ", lasheader->version_major, ".", lasheader->version_minor);
      results.add_fail("global encoding", note_oss.str());
    }

    if (lasheader->point_data_format == 0)
    {
      set_oss_content(note_oss, "set bit 0 not defined for point data format 0");
      results.add_fail("global encoding", note_oss.str());
    }
  }
  else
  {
    if (lasheader->point_data_format > 0)
    {
      if (lasinventory.active())
      {
        if ((lasinventory.min_gps_time < 0.0) || (lasinventory.max_gps_time > 604800.0)) 
        {
          std::string string1, string2;
          lidardouble2string(string1, lasinventory.min_gps_time, 0.000001);
          lidardouble2string(string2, lasinventory.max_gps_time, 0.000001);
          set_oss_content(note_oss, "unset bit 0 suggests GPS week time but GPS time ranges from ", string1, " to ", string2);
          results.add_fail("global encoding", note_oss.str());
        }
      }
    }
  }

  // check version major

  if (lasheader->version_major != 1)
  {
    set_oss_content(note_oss, "should be 1 and not ", lasheader->version_major);
    results.add_fail("version major", note_oss.str());
  }

  // check version minor

  if ((lasheader->version_minor != 0) && (lasheader->version_minor != 1) && (lasheader->version_minor != 2) && (lasheader->version_minor != 3) && (lasheader->version_minor != 4))
  {
    set_oss_content(note_oss, "should be between 0 and 4 and not ", lasheader->version_minor);
    results.add_fail("version minor", note_oss.str());
  }

  // check system identifier

  for (i = 0; i < 32; i++)
  {
    if (lasheader->system_identifier[i] == '\0')
    {
      break;
    }
  }
  if (i == 32)
  {
    set_oss_content(note_oss, "string should be terminated by a '\\0' character");
    results.add_fail("system identifier", note_oss.str());
  }
  else if (i == 0)
  {
    set_oss_content(note_oss, "empty string. first character is '\\0'");
    results.add_warning("system identifier", note_oss.str());
  }
  for (j = i; j < 32; j++)
  {
    if (lasheader->system_identifier[j] != '\0')
    {
      break;
    }
  }
  if (j != 32)
  {
    set_oss_content(note_oss, "remaining characters should all be '\\0'");
    results.add_fail("system identifier", note_oss.str());
  }

  // check generating software

  for (i = 0; i < 32; i++)
  {
    if (lasheader->generating_software[i] == '\0')
    {
      break;
    }
  }
  if (i == 32)
  {
    set_oss_content(note_oss, "string should be terminated by a '\\0' character");
    results.add_fail("generating software", note_oss.str());
  }
  else if (i == 0)
  {
    set_oss_content(note_oss, "empty string. first character is '\\0'");
    results.add_warning("generating software", note_oss.str());
  }
  for (j = i; j < 32; j++)
  {
    if (lasheader->generating_software[j] != '\0')
    {
      break;
    }
  }
  if (j != 32)
  {
    set_oss_content(note_oss, "remaining characters should all be '\\0'");
    results.add_fail("generating software", note_oss.str());
  }

  // check file creation date

  if (lasheader->file_creation_year == 0)
  {
    if (lasheader->file_creation_day == 0)
    {
      set_oss_content(note_oss, "not set");
      results.add_fail("file creation day", note_oss.str());
    }
    else if (lasheader->file_creation_day > 365)
    {
      set_oss_content(note_oss, "should be between 1 and 365 and not ", lasheader->file_creation_day);
      results.add_fail("file creation day", note_oss.str());
    }
    set_oss_content(note_oss, "not set");
    results.add_fail("file creation year", note_oss.str());
  }
  else
  {
    // get today's date

    std::time_t now = std::time(nullptr);
    std::tm tm_utc{};
#if defined(_WIN32)
    gmtime_s(&tm_utc, &now);
#else
    gmtime_r(&now, &tm_utc);
#endif

    int today_year = tm_utc.tm_year + 1900;

    // does the year fall into the expected range

    if ((lasheader->file_creation_year < 1990) || (lasheader->file_creation_year > today_year))
    {
      set_oss_content(note_oss, "should be between 1990 and ", today_year, " and not ", lasheader->file_creation_year);
      results.add_fail("file creation year", note_oss.str());
    }

    // does the day fall into the expected range

    int max_day_of_year = 365;

    if (lasheader->file_creation_year == today_year)
    {
      // for the current year we need to limit the range (plus 1 because in GPS time January 1 is day 1)

      max_day_of_year = tm_utc.tm_yday + 1;
    }
    else if ((((lasheader->file_creation_year)%4) == 0))
    {
      // in a leap year we need to add one day

      max_day_of_year++;
    }

    if (lasheader->file_creation_day > max_day_of_year)
    {
      set_oss_content(note_oss, "should be between 1 and ", max_day_of_year, " and not ", lasheader->file_creation_day);
      results.add_fail("file creation day", note_oss.str());
    }
  }

  // check header size

  int min_header_size = 227;

  if (lasheader->version_major == 1)
  {
    if (lasheader->version_minor >= 3)
    {
      min_header_size += 8;
    }
    if (lasheader->version_minor >= 4)
    {
      min_header_size += 40;
    }
  }

  if (lasheader->header_size < min_header_size)
  {
    set_oss_content(note_oss, "should be at least ", min_header_size, " and not ", lasheader->header_size);
    results.add_fail("header size", note_oss.str());
  }

  // check offset to point data

  U32 min_offset_to_point_data = lasheader->header_size;

  // add size of all VLRs

  for (i = 0; i < lasheader->number_of_variable_length_records; i++)
  {
    min_offset_to_point_data += 54; // VLR header size
    min_offset_to_point_data += lasheader->vlrs[i].record_length_after_header; // VLR payload size
  }

  if (lasheader->offset_to_point_data < min_offset_to_point_data)
  {
    set_oss_content(note_oss, "should be at least ", min_offset_to_point_data, " and not ", lasheader->offset_to_point_data);
    results.add_fail("offset to point data", note_oss.str());
  }

  // check point data format

  U8 max_point_data_format = 1;

  if (lasheader->version_major == 1)
  {
    if (lasheader->version_minor == 2)
    {
      max_point_data_format = 3;
    }
    else if (lasheader->version_minor == 3)
    {
      max_point_data_format = 5;
    }
    else if (lasheader->version_minor == 4)
    {
      max_point_data_format = 10;
    }
  }

  if (lasheader->point_data_format > max_point_data_format)
  {
    set_oss_content(note_oss, "should be between 0 and ", max_point_data_format, " and not ", lasheader->point_data_format);
    results.add_fail("point data format", note_oss.str());
  }

  // check point data record length

  int min_point_data_record_length = 20;

  switch (lasheader->point_data_format) {
    case 1:
      min_point_data_record_length = 28;
      break;
    case 2:
      min_point_data_record_length = 26;
      break;
    case 3:
      min_point_data_record_length = 34;
      break;
    case 4:
      min_point_data_record_length = 28;
      break;
    case 5:
      min_point_data_record_length = 36;
      break;
    case 6:
      min_point_data_record_length = 30;
      break;
    case 7:
      min_point_data_record_length = 36;
      break;
    case 8:
      min_point_data_record_length = 38;
      break;
    case 9:
      min_point_data_record_length = 57;
      break;
    case 10:
      min_point_data_record_length = 63;
      break;
  }

  /*switch (lasheader->point_data_format)
  {
  case 1:
    min_point_data_record_length = 28;
    break;
  case 2:
    min_point_data_record_length = 26;
    break;
  case 3:
    min_point_data_record_length = 34;
    break;
  case 4:
    min_point_data_record_length = 57;
    break;
  case 5:
    min_point_data_record_length = 63;
    break;
  case 6:
    min_point_data_record_length = 30;
    break;
  case 7:
    min_point_data_record_length = 36;
    break;
  case 8:
    min_point_data_record_length = 38;
    break;
  case 9:
    min_point_data_record_length = 59;
    break;
  case 10:
    min_point_data_record_length = 67;
    break;
  }*/

  if (lasheader->point_data_record_length < min_point_data_record_length)
  {
    set_oss_content(note_oss, "should be at least ", min_point_data_record_length, " and not ", lasheader->point_data_record_length);
    results.add_fail("point data record length", note_oss.str());
  }

  // check that there is at least one point

  if ((lasheader->version_major == 1) && (lasheader->version_minor < 4))
  {
    if (lasheader->number_of_point_records == 0)
    {
      set_oss_content(note_oss, "files must contain at least one point record to be considered valid");
      results.add_fail("zero points in file", note_oss.str());
    }
  }
  else
  {
    if (lasheader->extended_number_of_point_records == 0)
    {
      set_oss_content(note_oss, "file contains zero point records");
      results.add_warning("zero points in file", note_oss.str());
    }
  }

  // check that number of points by return is less than or equal to number of point records
  
  I64 total = 0;

  if ((lasheader->version_major == 1) && (lasheader->version_minor < 4))
  {
    for (i = 0; i < 5; i++)
    {
      if (lasheader->number_of_points_by_return[i] > lasheader->number_of_point_records)
      {
        set_oss_content(problem_oss, "legacy number of points by return[", i, "]");
        set_oss_content(note_oss, "should not be larger than number of point records");
        results.add_fail(problem_oss.str(), note_oss.str());
      }
      total += lasheader->number_of_points_by_return[i];
    }
    if (total > lasheader->number_of_point_records)
    {
      set_oss_content(note_oss, "sum should not be larger than number of point records");
      results.add_fail("legacy number of points by return [] array", note_oss.str());
    }
  }
  else
  {
    for (i = 0; i < 15; i++)
    {
      if (lasheader->extended_number_of_points_by_return[i] > lasheader->extended_number_of_point_records)
      {
        set_oss_content(problem_oss, "number of points by return[", i, "]");
        set_oss_content(note_oss, "should not be larger than number of point records");
        results.add_fail(problem_oss.str(), note_oss.str());
      }
      total += lasheader->extended_number_of_points_by_return[i];
    }
    if (total > lasheader->number_of_point_records)
    {
      set_oss_content(note_oss, "sum should not be larger than number of point records");
      results.add_fail("number of points by return [] array", note_oss.str());
    }
  }

  // check integraty between legacy number of point records and number of point records for LAS 1.4 and higher

  if ((lasheader->version_major == 1) && (lasheader->version_minor >= 4))
  {
    if (lasheader->point_data_format <= 5)
    {
      if (lasheader->number_of_point_records == 0)
      {
        if ((lasheader->extended_number_of_point_records > 0) && (lasheader->extended_number_of_point_records <= U32_MAX))
        {
          set_oss_content(note_oss, "unnecessary lack of forward compatibility. should be ", lasheader->number_of_point_records, " as this LAS 1.",
              lasheader->version_minor, " file contains less than ", U32_MAX, " points of type ", lasheader->point_data_format);
          results.add_warning("legacy number of point records", note_oss.str());
        }
      }
      else
      {
        if (lasheader->extended_number_of_point_records != lasheader->number_of_point_records)
        {
          if (lasheader->extended_number_of_point_records <= U32_MAX)
          {
            set_oss_content(note_oss, "should be ", lasheader->extended_number_of_point_records, " to be consistent with number of point records instead of ",
                lasheader->number_of_point_records);
          }
          else
          {
            set_oss_content(note_oss, "should be 0 for LAS 1.", lasheader->version_minor, " files that contain over ", U32_MAX, " points of type ",
                lasheader->point_data_format);
          }
          results.add_fail("legacy number of point records", note_oss.str());
        }
      }
    }
    else
    {
      if (lasheader->number_of_point_records != 0)
      {
        set_oss_content(note_oss, "should be 0 for LAS 1.", lasheader->version_minor, " files that contain point type ", lasheader->point_data_format);
        results.add_fail("legacy number of point records", note_oss.str());
      }
    }
  }

  // check integraty between legacy number of points by return and number of points by return for LAS 1.4 and higher

  if ((lasheader->version_major == 1) && (lasheader->version_minor >= 4)) {
    if (lasheader->point_data_format <= 5) {
      // Legacy returns may be used
      for (i = 0; i < 5; i++) {
        U64 ext = lasheader->extended_number_of_points_by_return[i];
        U32 leg = lasheader->number_of_points_by_return[i];

        set_oss_content(problem_oss, "legacy number of points by return[", i, "]");

        if (ext <= UINT32_MAX) {
          // Must match exactly
          if (leg != static_cast<uint32_t>(ext)) {
            set_oss_content(note_oss, "legacy ", leg, " and extended ", ext, " points-by-return mismatch as this LAS 1.", lasheader->version_minor,
                " file contains less than", U32_MAX, " points for point data format ", lasheader->point_data_format);
            results.add_fail(problem_oss.str(), note_oss.str());
          }
        } else if (leg ) {
          set_oss_content(note_oss, "legacy value ", leg, " is invalid because extended value ", ext, " exceeds 32-bit range in LAS 1.", 
              lasheader->version_minor, " for point data formate ", lasheader->point_data_format);
          results.add_warning(problem_oss.str(), note_oss.str());
        }
      }
    } else {
      // Point Data Format >= 6, Legacy Returns must be 0
      for (i = 0; i < 5; i++) {
        if (lasheader->number_of_points_by_return[i] != 0) {
          set_oss_content(problem_oss, "should be 0 for LAS 1.", lasheader->version_minor, " files that contain point type ", lasheader->point_data_format);
          results.add_fail(problem_oss.str(), note_oss.str());
        }
      }
    }
  }

  // if tile size was given check that header bounding box agrees with tile size

  if (tile_size)
  {
    F64 epsilon_x = 0.5 * lasheader->x_scale_factor;
    F64 epsilon_y = 0.5 * lasheader->y_scale_factor;

    if ((lasheader->max_x - lasheader->min_x) > (tile_size + epsilon_x))
    {
      set_oss_content(problem_oss, "header bounding box exceeds tile size in x");
      set_oss_content(note_oss, "max_x - min_x is ", (lasheader->max_x - lasheader->min_x), " and exceeds ", tile_size);
      results.add_fail(problem_oss.str(), note_oss.str());
    }
    if ((lasheader->max_y - lasheader->min_y) > (tile_size + epsilon_y))
    {
      set_oss_content(problem_oss, "header bounding box exceeds tile size in y");
      set_oss_content(note_oss, "max_y - min_y is ", (lasheader->max_y - lasheader->min_y), " and exceeds ", tile_size);
      results.add_fail(problem_oss.str(), note_oss.str());
    }  
  }

  // check number of point records in header against the counted inventory

  if (lassummary.active())
  {
    if ((lasheader->version_major == 1) && (lasheader->version_minor >= 4))
    {
      if (lasheader->extended_number_of_point_records != lassummary.number_of_point_records)
      {
        set_oss_content(note_oss, "there are only ", lassummary.number_of_point_records, " point records and not ", lasheader->extended_number_of_point_records);
        results.add_fail("number of point records", note_oss.str());
      }
    }
    else
    {
      if (lasheader->number_of_point_records != U32_CLAMP(lassummary.number_of_point_records))
      {
        set_oss_content(note_oss, "there are only ", U32_CLAMP(lassummary.number_of_point_records), " point records and not ", lasheader->number_of_point_records);
        results.add_fail("number of point records", note_oss.str());
      }
    }
  }

  // check number of points by return in header against the counted inventory

  if (lassummary.active())
  {
    if ((lasheader->version_major == 1) && (lasheader->version_minor >= 4))
    {
      for (i = 0; i < 15; i++)
      {
        if (lasheader->extended_number_of_points_by_return[i] != lassummary.number_of_points_by_return[i + 1])
        {
          set_oss_content(problem_oss, "number of points by return[", i, "]");
          set_oss_content(note_oss, "the number of ", i + 1, (i == 0 ? "st" : (i == 1 ? "nd" : (i == 2 ? "rd" : "th"))), " returns is ",
              lassummary.number_of_points_by_return[i + 1], " and not ", lasheader->extended_number_of_points_by_return[i]);
          results.add_fail(problem_oss.str(), note_oss.str());
        }
      }
    }
    else
    {
      for (i = 0; i < 5; i++)
      {
        if (lasheader->number_of_points_by_return[i] != U32_CLAMP(lassummary.number_of_points_by_return[i + 1]))
        {
          set_oss_content(problem_oss, "number of points by return[", i, "]");
          set_oss_content(note_oss, "the number of ", i + 1, (i == 0 ? "st" : (i == 1 ? "nd" : (i == 2 ? "rd" : "th"))), " returns is ",
              U32_CLAMP(lassummary.number_of_points_by_return[i + 1]), " and not ", lasheader->number_of_points_by_return[i]);
          results.add_fail(problem_oss.str(), note_oss.str());
        }
      }
    }
  }

  // check scale factor x y z

  if (lasheader->x_scale_factor <= 0.0)
  {
    set_oss_content(note_oss, lasheader->x_scale_factor , " is equal to or smaller than zero");
    results.add_fail("x scale factor", note_oss.str());
  }

  if (lasheader->y_scale_factor <= 0.0)
  {
    set_oss_content(note_oss, lasheader->y_scale_factor , " is equal to or smaller than zero");
    results.add_fail("y scale factor", note_oss.str());
  }

  if (lasheader->z_scale_factor <= 0.0)
  {
    set_oss_content(note_oss, lasheader->z_scale_factor , " is equal to or smaller than zero");
    results.add_fail("z scale factor", note_oss.str());
  }

  bool valid_x_scale = false;
  bool valid_y_scale = false;
  bool valid_z_scale = false;
  for (double value : allowed_scale_factors) {
    if (FLOATEQUAL(lasheader->x_scale_factor, value)) {
      valid_x_scale = true;
    }
    if (FLOATEQUAL(lasheader->y_scale_factor, value)) {
      valid_y_scale = true;
    }
    if (FLOATEQUAL(lasheader->z_scale_factor, value)) {
      valid_z_scale = true;
    }
    if (valid_x_scale == true && valid_y_scale == true && valid_z_scale == true) {
      break;
    }
  }

  if (!valid_x_scale) {
    std::string val_str;
    lidardouble2string(val_str, lasheader->x_scale_factor);
    set_oss_content(note_oss, "should be factor ten of 0.1 or 0.5 or 0.25 and not ", val_str);
    results.add_warning("x scale factor", note_oss.str());
  }

  if (!valid_y_scale) {
    std::string string;
    lidardouble2string(string, lasheader->y_scale_factor);
    set_oss_content(note_oss, "should be factor ten of 0.1 or 0.5 or 0.25 and not ", string);
    results.add_warning("y scale factor", note_oss.str());
  }

  if (!valid_z_scale) {
    std::string string;
    lidardouble2string(string, lasheader->z_scale_factor);
    set_oss_content(note_oss, "should be factor ten of 0.1 or 0.5 or 0.25 and not ", string);
    results.add_warning("z scale factor", note_oss.str());
  }

  // check offset x y z

  I64 x_offset_quantized = I64_QUANTIZE(lasheader->x_offset/lasheader->x_scale_factor);
  F64 x_offset = lasheader->x_scale_factor * x_offset_quantized;
  if (!FLOATEQUAL(lasheader->x_offset - x_offset, 0.0))
  {
    std::string string1, string2;
    lidardouble2string(string1, lasheader->x_offset);
    lidardouble2string(string2, lasheader->x_scale_factor);
    set_oss_content(note_oss, "translation fluff: decimal digits of ", string1, " do not match scale factor ", string2);
    results.add_warning("x offset", note_oss.str());
  }

  I64 y_offset_quantized = I64_QUANTIZE(lasheader->y_offset/lasheader->y_scale_factor);
  F64 y_offset = lasheader->y_scale_factor * y_offset_quantized;
  if (!FLOATEQUAL(lasheader->y_offset - y_offset, 0.0))
  {
    std::string string1, string2;
    lidardouble2string(string1, lasheader->y_offset);
    lidardouble2string(string2, lasheader->y_scale_factor);
    set_oss_content(note_oss, "translation fluff: decimal digits of ", string1, " do not match scale factor ", string2);
    results.add_warning("y offset", note_oss.str());
  }

  I64 z_offset_quantized = I64_QUANTIZE(lasheader->z_offset/lasheader->z_scale_factor);
  F64 z_offset = lasheader->z_scale_factor * z_offset_quantized;
  if (!FLOATEQUAL(lasheader->z_offset - z_offset, 0.0))
  {
    std::string string1, string2;
    lidardouble2string(string1, lasheader->z_offset);
    lidardouble2string(string2, lasheader->z_scale_factor);
    set_oss_content(note_oss, "translation fluff: decimal digits of ", string1, " do not match scale factor ", string2);
    results.add_warning("z offset", note_oss.str());
  }

  // check global encoding and waveform data package

  if ((lasheader->version_major == 1) && (lasheader->version_minor >= 3))
  {
    if (((lasheader->global_encoding & 2) == 0) && (lasheader->start_of_waveform_data_packet_record != 0))
    {
      set_oss_content(note_oss, "should be 0 and not ", lasheader->start_of_waveform_data_packet_record, " because global encoding bit 1 is not set");
      results.add_fail("start of waveform data packet record", note_oss.str());
    }
    else if (((lasheader->global_encoding & 2) == 2) && (lasheader->start_of_waveform_data_packet_record == 0))
    {
      set_oss_content(note_oss, "should not be 0 because global encoding bit 1 is set");
      results.add_fail("start of waveform data packet record", note_oss.str());
    }
  }

  // check for resolution fluff in the coordinates

  if (lassummary.active())
  {
    if (lassummary.has_fluff())
    {
      set_oss_content(note_oss, "resolution fluff (x10) in ", (lassummary.has_fluff(0) ? "X" : ""), (lassummary.has_fluff(1) ? "Y" : ""),
          (lassummary.has_fluff(2) ? "Z" : ""));
      results.add_warning("coordinate values", note_oss.str());
      if (lassummary.has_serious_fluff())
      {
        set_oss_content(note_oss, "serious resolution fluff (x100) in ", (lassummary.has_serious_fluff(0) ? "X" : ""),
            (lassummary.has_serious_fluff(1) ? "Y" : ""), (lassummary.has_serious_fluff(2) ? "Z" : ""));
        results.add_warning("coordinate values", note_oss.str());
        if (lassummary.has_very_serious_fluff())
        {
          set_oss_content(note_oss, "very serious resolution fluff (x1000) in ", (lassummary.has_very_serious_fluff(0) ? "X" : ""),
              (lassummary.has_very_serious_fluff(1) ? "Y" : ""), (lassummary.has_very_serious_fluff(2) ? "Z" : ""));
          results.add_warning("coordinate values", note_oss.str());
        }
      }
    }
  }

  // check bounding box x y z

  if (points_outside_bounding_box > 0)
  {
    set_oss_content(note_oss, "there are ", points_outside_bounding_box, " points outside of the bounding box specified in the LAS file header");
    results.add_fail("bounding box", note_oss.str());
  }

  if (lasinventory.active())
  {
    F64 epsilon_x = 0.5 * lasheader->x_scale_factor;
    F64 epsilon_y = 0.5 * lasheader->y_scale_factor;
    F64 epsilon_z = 0.5 * lasheader->z_scale_factor;

    if ((lasheader->min_x - epsilon_x) > lasheader->get_x(lasinventory.min_X))
    {
      std::string string1, string2;
      lidardouble2string(string1, lasheader->get_x(lasinventory.min_X), lasheader->x_scale_factor);
      lidardouble2string(string2, lasheader->min_x, lasheader->x_scale_factor);
      set_oss_content(note_oss, "should be ", string1, " and not ", string2);
      results.add_fail("min x", note_oss.str());
    }
    if ((lasheader->max_x + epsilon_x) < lasheader->get_x(lasinventory.max_X))
    {
      std::string string1, string2;
      lidardouble2string(string1, lasheader->get_x(lasinventory.max_X), lasheader->x_scale_factor);
      lidardouble2string(string2, lasheader->max_x, lasheader->x_scale_factor);
      set_oss_content(note_oss, "should be ", string1, " and not ", string2);
      results.add_fail("max x", note_oss.str());
    }
    if ((lasheader->min_y - epsilon_y) > lasheader->get_y(lasinventory.min_Y))
    {
      std::string string1, string2;
      lidardouble2string(string1, lasheader->get_y(lasinventory.min_Y), lasheader->y_scale_factor);
      lidardouble2string(string2, lasheader->min_y, lasheader->y_scale_factor);
      set_oss_content(note_oss, "should be ", string1, " and not ", string2);
      results.add_fail("min y", note_oss.str());
    }
    if ((lasheader->max_y + epsilon_y) < lasheader->get_y(lasinventory.max_Y))
    {
      std::string string1, string2;
      lidardouble2string(string1, lasheader->get_y(lasinventory.max_Y), lasheader->y_scale_factor);
      lidardouble2string(string2, lasheader->max_y, lasheader->y_scale_factor);
      set_oss_content(note_oss, "should be ", string1, " and not ", string2);
      results.add_fail("max y", note_oss.str());
    }
    if ((lasheader->min_z - epsilon_z) > lasheader->get_z(lasinventory.min_Z))
    {
      std::string string1, string2;
      lidardouble2string(string1, lasheader->get_z(lasinventory.min_Z), lasheader->z_scale_factor);
      lidardouble2string(string2, lasheader->min_z, lasheader->z_scale_factor);
      set_oss_content(note_oss, "should be ", string1, " and not ", string2);
      results.add_fail("min z", note_oss.str());
    }
    if ((lasheader->max_z + epsilon_z) < lasheader->get_z(lasinventory.max_Z))
    {
      std::string string1, string2;
      lidardouble2string(string1, lasheader->get_z(lasinventory.max_Z), lasheader->z_scale_factor);
      lidardouble2string(string2, lasheader->max_z, lasheader->z_scale_factor);
      set_oss_content(note_oss, "should be ", string1, " and not ", string2);
      results.add_fail("max z", note_oss.str());
    }
  }

  // check the inventory for invalid return numbers

  if (lassummary.active())
  {
    if (lassummary.number_of_points_by_return[0] != 0)
    {
      set_oss_content(note_oss, "there are ", lassummary.number_of_points_by_return[0], " points with a return number of 0");
      results.add_warning("return number", note_oss.str());
    }
    if ((lasheader->version_major == 1) && (lasheader->version_minor < 4))
    {
      for (i = 6; i <= 15; i++) {
        if (lassummary.number_of_points_by_return[i] != 0) {
          set_oss_content(note_oss, "there are ", lassummary.number_of_points_by_return[i], " points with a return number of ", i, " for LAS 1.",
              lasheader->version_minor);
          results.add_warning("return number", note_oss.str());
        }
      }
    }
  }

  // check the inventory for invalid number of returns of given pulse

  if (lassummary.active())
  {
    if (lassummary.number_of_returns[0] != 0)
    {
      set_oss_content(note_oss, "there are ", lassummary.number_of_returns[0], " points with a number of returns of given pulse of 0");
      results.add_warning("number of returns of given pulse", note_oss.str());
    }
    if ((lasheader->version_major == 1) && (lasheader->version_minor < 4))
    {
      for (i = 6; i <= 15; i++) {
        if (lassummary.number_of_returns[i] != 0) {
          set_oss_content(note_oss, "there are ", lassummary.number_of_returns[i], " points with a number of returns of given pulse of ", i, " for LAS 1.",
              lasheader->version_minor);
          results.add_warning("return number", note_oss.str());
        }
      }
    }
  }

  // check for odd intensities

  if (lassummary.active())
  {
    if ((lassummary.number_of_point_records > 1) && (lassummary.min.intensity == lassummary.max.intensity))
    {
      set_oss_content(note_oss, "intensity of all ", lassummary.number_of_point_records, " points is ", lassummary.min.intensity);
      results.add_warning("intensity", note_oss.str());
    }
  }

  // check for odd scan angles

  if (lassummary.active())
  {
    if (lassummary.number_of_point_records > 1)
    {
      if (lasheader->point_data_format < 6)
      {
        if (lassummary.min.get_scan_angle_raw() == lassummary.max.get_scan_angle_raw())
        {
          set_oss_content(note_oss, "scan angle rank of all ", lassummary.number_of_point_records, " points is ", lassummary.min.get_scan_angle_raw());
          results.add_warning("scan angle rank", note_oss.str());
        }
      }
      else
      {
        if (lassummary.min.get_scan_angle_disp() == lassummary.max.get_scan_angle_disp())
        {
          set_oss_content(note_oss, "scan angle of all ", lassummary.number_of_point_records, " points is ", std::fixed, std::setprecision(3),
              lassummary.min.get_scan_angle_disp());
          results.add_warning("scan angle", note_oss.str());
        }
      }
    }
  }

  // check for zero point source IDs

  if (lassummary.active())
  {
    if ((lasheader->file_source_ID == 0) && (lassummary.number_of_point_records > 1) && (lassummary.min.point_source_ID == 0) &&
        (lassummary.max.point_source_ID == 0))
    {
      set_oss_content(note_oss, "file source ID in header and point source ID of all ", lassummary.number_of_point_records, " points is ",
          lassummary.min.point_source_ID);
      results.add_warning("point source ID", note_oss.str());
    }
  }

  // check for file source ID and point source IDs disagreement

  if (lassummary.active())
  {
    if ((lasheader->file_source_ID != 0) && (lassummary.number_of_point_records > 1) &&
        ((lasheader->file_source_ID != lassummary.min.point_source_ID) || (lasheader->file_source_ID != lassummary.max.point_source_ID)) &&
        ((lassummary.min.point_source_ID != 0) || (lassummary.max.point_source_ID != 0)))
    {
      set_oss_content(note_oss, "file source ID in header is ", lasheader->file_source_ID, " but point source IDs of all ", lassummary.number_of_point_records,
          " points range from ", lassummary.min.point_source_ID, " to ", lassummary.max.point_source_ID);
      results.add_warning("point source ID", note_oss.str());
    }
  }

  // check for point data formats 1, 3, 4, and higher in the inventory whether all GPS time stamps are identical

  if ((lasheader->point_data_format != 0) && (lasheader->point_data_format != 2))
  {
    if (lasinventory.active() && lassummary.active())
    {
      if ((lassummary.number_of_point_records > 1) && (lasinventory.min_gps_time == lasinventory.max_gps_time))
      {
        set_oss_content(note_oss, "time stamps of all ", lassummary.number_of_point_records, " points are ", lasinventory.min_gps_time);
        results.add_warning("GPS time", note_oss.str());
      }
    }
  }

  // check for point data formats 2, 3, 7, 8, and 10 in the inventory whether all RGB values are identical

  if ((lasheader->point_data_format == 2) || (lasheader->point_data_format == 3) || (lasheader->point_data_format == 7) || (lasheader->point_data_format == 8) || (lasheader->point_data_format == 10))
  {
    if (lassummary.active())
    {
      if ((lassummary.number_of_point_records > 1) && (lassummary.min.rgb[0] == lassummary.max.rgb[0]) &&
          (lassummary.min.rgb[1] == lassummary.max.rgb[1]) && (lassummary.min.rgb[2] == lassummary.max.rgb[2]))
      {
        set_oss_content(note_oss, "color of all ", static_cast<I64>(lassummary.number_of_point_records), " points is (", static_cast<int>(lassummary.max.rgb[0]),
            "/", static_cast<int>(lassummary.max.rgb[1]), "/", static_cast<int>(lassummary.max.rgb[2]), ")");
        results.add_warning("RGB", note_oss.str());
      }
    }
  }

  if (lasheader->point_data_format <= 5)
  {
    // a GEOTIFF tag CRS when the point type is 5 or lower
    if (lasheader->vlr_geo_keys == nullptr)
    {
      set_oss_content(note_oss, "file does not specify a Coordinate Reference System with GEOTIFF tags");
      if (no_CRS_fail)
      {
        results.add_warning("CRS", note_oss.str());
      }
      else
      {
        results.add_fail("CRS", note_oss.str());
      }
    }
  }
  else
  {
    // an OCG WKT CRS when the point type is 6 or higher
    if (lasheader->vlr_geo_ogc_wkt == nullptr)
    {
      set_oss_content(note_oss, "file with point data format ", lasheader->point_data_format, " does not specify Coordinate Reference System with OGC WKT string");
      if (no_CRS_fail)
      {
        results.add_warning("CRS", note_oss.str());
      }
      else
      {
        results.add_fail("CRS", note_oss.str());
      }
    }
  }

  // check for wrong wave packet indices

  if ((lasheader->point_data_format == 4) || (lasheader->point_data_format == 5) || (lasheader->point_data_format == 9) || (lasheader->point_data_format == 10))
  {
    if (!lasheader->vlr_wave_packet_descr) {
      set_oss_content(note_oss, "waveform point data format used, but no wave packet descriptor VLRs defined in header");
      results.add_fail("wave packet descriptor", note_oss.str());
    };

    bool has_any_descriptor = false;

    for (i = 1; i < 256; i++)
    {
      if (lasheader->vlr_wave_packet_descr[i]) {
        has_any_descriptor = true;
        break;
      }
    }

    if (!has_any_descriptor) {
      set_oss_content(note_oss, "waveform point data format used, but wave packet descriptor table is empty");
      results.add_fail("wave packet", note_oss.str());
    }
  }

  if (lasheader->vlr_geo_keys || lasheader->vlr_geo_ogc_wkt)
  {
    CRScheck crscheck(lasheader);
    crscheck.check(results, crsdescription, no_CRS_fail);
  }
}

LAScheck::LAScheck(const LASheader* lasheader)
{
  min_x = lasheader->min_x - lasheader->x_scale_factor;
  min_y = lasheader->min_y - lasheader->y_scale_factor;
  min_z = lasheader->min_z - lasheader->z_scale_factor;
  max_x = lasheader->max_x + lasheader->x_scale_factor;
  max_y = lasheader->max_y + lasheader->y_scale_factor;
  max_z = lasheader->max_z + lasheader->z_scale_factor;
  points_outside_bounding_box = 0;
  this->lasheader = lasheader;
}

LAScheck::~LAScheck()
{
}
