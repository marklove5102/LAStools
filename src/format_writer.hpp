/*
===============================================================================

  FILE:  format_writer.hpp

  CONTENTS:

    Creates a suitable ValidateWriter depending on the selected format (XML, TXT, JSON) and initialises it with the transferred FILE* resource.

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

    5 February 2026 -- created

===============================================================================
*/

#ifndef FORMAT_VALIDATION_WRITER_FACTORY_HPP
#define FORMAT_VALIDATION_WRITER_FACTORY_HPP

#include "mydefs.hpp"
#include "validate_writer.hpp"

#include <stdio.h>

class FormatWriterFactory {
 public:
  static ValidateWriter* createWriter(I32 format, FILE* file);
};

#endif