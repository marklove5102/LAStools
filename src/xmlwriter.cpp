/*
===============================================================================

  FILE:  xmlwriter.cpp
  
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
#include "xmlwriter.hpp"

XMLwriter::XMLwriter() {
  sub = FALSE;
  file = nullptr;
}

XMLwriter::~XMLwriter() {
  if (file && (file != stdout)) fclose(file);
}

BOOL XMLwriter::is_open() const {
  return (BOOL)(file != nullptr);
}

BOOL XMLwriter::open(const std::string& file_name, const std::string& key) {
  if (!file_name.empty())
  {
    file = LASfopen(file_name.c_str(), "w");
    if (file == nullptr)
    {
      laserror("cannot open XML file '%s'\n", file_name);
      return FALSE;
    }
  }
  else
  {
    file = stdout;
  }
  fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(file, "<%s>\n", key.c_str());
  return TRUE;
}

BOOL XMLwriter::begin(const std::string& key) {
  fprintf(file, "  <%s>\n", key.c_str());
  return TRUE;
}

BOOL XMLwriter::beginsub(const std::string& key) {
  if (sub)
  {
    return FALSE;
  }
  sub = TRUE;
  fprintf(file, "    <%s>\n", key.c_str());
  return TRUE;
}

BOOL XMLwriter::write(const std::string& value) {
  if (sub)
  {
    fprintf(file, "      %s\n", value.c_str());
  }
  else
  {
    fprintf(file, "    %s\n", value.c_str());
  }
  return TRUE;
}

BOOL XMLwriter::write(I32 value) {
  if (sub)
  {
    fprintf(file, "      %d\n", value);
  }
  else
  {
    fprintf(file, "    %d\n", value);
  }
  return TRUE;
}

BOOL XMLwriter::write(const std::string& key, const std::string& value) {
  if (sub)
  {
    fprintf(file, "      <%s>%s</%s>\n", key.c_str(), value.c_str(), key.c_str());
  }
  else
  {
    fprintf(file, "    <%s>%s</%s>\n", key.c_str(), value.c_str(), key.c_str());
  }
  return TRUE;
}

BOOL XMLwriter::write(const std::string& key, I32 value) {
  if (sub)
  {
    fprintf(file, "      <%s>%d</%s>\n", key.c_str(), value, key.c_str());
  }
  else
  {
    fprintf(file, "    <%s>%d</%s>\n", key.c_str(), value, key.c_str());
  }
  return TRUE;
}

BOOL XMLwriter::write(const std::string& variable, const std::string& key, const std::string& note) {
  if (sub)
  {
    fprintf(file, "      <%s>\n", key.c_str());
    fprintf(file, "        <variable>%s</variable>\n", variable.c_str());
    if (!note.empty())
    {
      fprintf(file, "        <note>%s</note>\n", note.c_str());
    }
    fprintf(file, "      </%s>\n", key.c_str());
  }
  else
  {
    fprintf(file, "    <%s>\n", key.c_str());
    fprintf(file, "      <variable>%s</variable>\n", variable.c_str());
    if (!note.empty())
    {
      fprintf(file, "      <note>%s</note>\n", note.c_str());
    }
    fprintf(file, "    </%s>\n", key.c_str());
  }
  return TRUE;
}

BOOL XMLwriter::endsub(const std::string& key) {
  if (!sub)
  {
    return FALSE;
  }
  sub = FALSE;
  fprintf(file, "    </%s>\n", key.c_str());
  return TRUE;
}

BOOL XMLwriter::end(const std::string& key) {
  fprintf(file, "  </%s>\n", key.c_str());
  return TRUE;
}

BOOL XMLwriter::close(const std::string& key) {
  fprintf(file, "</%s>\n", key.c_str());
  if (file != stdout) fclose(file);
  file = nullptr;
  return true;
}
