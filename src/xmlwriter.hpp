/*
===============================================================================

  FILE:  xmlwriter.hpp
  
  CONTENTS:
  
    Writes a LAScheck report to a file in a very simple XML format.

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
  
    1 April 2013 -- on Easter Monday all-nighting in Perth airport for PER->SYD

===============================================================================
*/
#ifndef XML_WRITER_HPP
#define XML_WRITER_HPP

#include "mydefs.hpp"

#include <stdio.h>

class XMLwriter
{
public:

  BOOL is_open() const;
  BOOL open(const std::string& file_name, const std::string& key);
  BOOL begin(const std::string& key);
  BOOL beginsub(const std::string& key);
  BOOL write(I32 value);
  BOOL write(const std::string& value);
  BOOL write(const std::string& key, I32 value);
  BOOL write(const std::string& key, const std::string& value);
  BOOL write(const std::string& variable, const std::string& key, const std::string& note);
  BOOL endsub(const std::string& key);
  BOOL end(const std::string& key);
  BOOL close(const std::string& key);

  XMLwriter();
  ~XMLwriter();

private:
  BOOL sub;
  FILE* file;
};

#endif
