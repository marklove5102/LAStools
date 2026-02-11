/*
===============================================================================

  FILE:  validate_xml_writer.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2026, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "validate_xml_writer.hpp"

BOOL ValidteXmlWriter::is_open() const {
  return (BOOL)(file != nullptr);
}

BOOL ValidteXmlWriter::open(const std::string& key) {
  if (file == nullptr) return FALSE;

  indent = 0;
  fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  printIndent();
  fprintf(file, "<%s>\n", key.c_str());
  indent++;
  return TRUE;
}

//BOOL ValidteXmlWriter::open(const std::string& key) {
//  if (file == nullptr) return FALSE;
//
//  fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
//  fprintf(file, "<%s>\n", key.c_str());
//  return TRUE;
//}

BOOL ValidteXmlWriter::begin(const std::string& key) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "<%s>\n", key.c_str());
  indent++;
  return TRUE;
}

//BOOL ValidteXmlWriter::begin(const std::string& key) {
//  if (file == nullptr) return FALSE;
//  fprintf(file, "  <%s>\n", key.c_str());
//  return TRUE;
//}

BOOL ValidteXmlWriter::beginsub(const std::string& key) {
  return begin(key);
  /*if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "<%s>\n", key.c_str());
  indent++;
  return TRUE;*/
}

//BOOL ValidteXmlWriter::beginsub(const std::string& key) {
//  if (file == nullptr) return FALSE;
//  if (sub)
//  {
//    return FALSE;
//  }
//  sub = TRUE;
//  fprintf(file, "    <%s>\n", key.c_str());
//  return TRUE;
//}

BOOL ValidteXmlWriter::write(const std::string& value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%s\n", value.c_str());
  return TRUE;
}

//BOOL ValidteXmlWriter::write(const std::string& value) {
//  if (file == nullptr) return FALSE;
//  if (sub)
//  {
//    fprintf(file, "      %s\n", value.c_str());
//  }
//  else
//  {
//    fprintf(file, "    %s\n", value.c_str());
//  }
//  return TRUE;
//}

BOOL ValidteXmlWriter::write(I32 value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%d\n", value);
  return TRUE;
}

//BOOL ValidteXmlWriter::write(I32 value) {
//  if (file == nullptr) return FALSE;
//  if (sub)
//  {
//    fprintf(file, "      %d\n", value);
//  }
//  else
//  {
//    fprintf(file, "    %d\n", value);
//  }
//  return TRUE;
//}

BOOL ValidteXmlWriter::write(const std::string& key, const std::string& value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "<%s>%s</%s>\n", key.c_str(), value.c_str(), key.c_str());
  return TRUE;
}

//BOOL ValidteXmlWriter::write(const std::string& key, const std::string& value) {
//  if (file == nullptr) return FALSE;
//  if (sub)
//  {
//    fprintf(file, "      <%s>%s</%s>\n", key.c_str(), value.c_str(), key.c_str());
//  }
//  else
//  {
//    fprintf(file, "    <%s>%s</%s>\n", key.c_str(), value.c_str(), key.c_str());
//  }
//  return TRUE;
//}

BOOL ValidteXmlWriter::write(const std::string& key, I32 value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "<%s>%d</%s>\n", key.c_str(), value, key.c_str());
  return TRUE;
}

//BOOL ValidteXmlWriter::write(const std::string& key, I32 value) {
//  if (file == nullptr) return FALSE;
//  if (sub)
//  {
//    fprintf(file, "      <%s>%d</%s>\n", key.c_str(), value, key.c_str());
//  }
//  else
//  {
//    fprintf(file, "    <%s>%d</%s>\n", key.c_str(), value, key.c_str());
//  }
//  return TRUE;
//}

BOOL ValidteXmlWriter::write(const std::string& variable, const std::string& key, const std::string& note) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "<%s>\n", key.c_str());
  indent++;

  printIndent();
  fprintf(file, "<variable>%s</variable>\n", variable.c_str());

  if (!note.empty()) {
    printIndent();
    fprintf(file, "<note>%s</note>\n", note.c_str());
  }

  indent--;
  printIndent();
  fprintf(file, "</%s>\n", key.c_str());
  return TRUE;
}

//BOOL ValidteXmlWriter::write(const std::string& variable, const std::string& key, const std::string& note) {
//  if (file == nullptr) return FALSE;
//  if (sub)
//  {
//    fprintf(file, "      <%s>\n", key.c_str());
//    fprintf(file, "        <variable>%s</variable>\n", variable.c_str());
//    if (!note.empty())
//    {
//      fprintf(file, "        <note>%s</note>\n", note.c_str());
//    }
//    fprintf(file, "      </%s>\n", key.c_str());
//  }
//  else
//  {
//    fprintf(file, "    <%s>\n", key.c_str());
//    fprintf(file, "      <variable>%s</variable>\n", variable.c_str());
//    if (!note.empty())
//    {
//      fprintf(file, "      <note>%s</note>\n", note.c_str());
//    }
//    fprintf(file, "    </%s>\n", key.c_str());
//  }
//  return TRUE;
//}

BOOL ValidteXmlWriter::endsub(const std::string& key) {
  /*if (file == nullptr) return FALSE;

  indent--;
  printIndent();
  fprintf(file, "</%s>\n", key.c_str());
  return TRUE;*/

  return end(key);
}

//BOOL ValidteXmlWriter::endsub(const std::string& key) {
//  if (file == nullptr) return FALSE;
//  if (!sub)
//  {
//    return FALSE;
//  }
//  sub = FALSE;
//  fprintf(file, "    </%s>\n", key.c_str());
//  return TRUE;
//}

BOOL ValidteXmlWriter::end(const std::string& key) {
  if (file == nullptr) return FALSE;

  indent--;
  printIndent();
  fprintf(file, "</%s>\n", key.c_str());
  return TRUE;
}

//BOOL ValidteXmlWriter::end(const std::string& key) {
//  if (file == nullptr) return FALSE;
//  fprintf(file, "  </%s>\n", key.c_str());
//  return TRUE;
//}
