/*
===============================================================================

  FILE:  format_writer.cpp

  CONTENTS:

    see corresponding header file

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2013, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    see corresponding header file

===============================================================================
*/

#include "format_writer.hpp"
#include "validate_xml_writer.hpp"
#include "validate_json_writer.hpp"
#include "validate_txt_writer.hpp"
#include "lasdefinitions.hpp"

ValidateWriter* FormatWriterFactory::createWriter(I32 format, FILE* file) {
  switch (format) {
    case LAS_TOOLS_FORMAT_XML:
      return new ValidteXmlWriter(file);
    case LAS_TOOLS_FORMAT_TXT:
      return new ValidateTxtWriter(file);
    default :
      return new ValidateJsonWriter(file);  
  }
  return nullptr; 
}
