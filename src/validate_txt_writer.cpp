/*
===============================================================================

  FILE:  validate_txt_writer.cpp

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
#include "validate_txt_writer.hpp"

//void ValidateTxtWriter::printIndent() {
//  for (int i = 0; i < indent; ++i) {
//    fprintf(file, "  ");  // 2 spaces per level
//  }
//}

BOOL ValidateTxtWriter::is_open() const {
  return (BOOL)(file != nullptr);
}

BOOL ValidateTxtWriter::open(const std::string& key) {
  if (file == nullptr) return FALSE;

  fprintf(file, "%s\n", key.c_str());
  fprintf(file, "============\n");
  indent = 0;

  return TRUE;
}

BOOL ValidateTxtWriter::begin(const std::string& key) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%s:\n", key.c_str());
  indent++;

  return TRUE;
}

BOOL ValidateTxtWriter::beginsub(const std::string& key) {
  //if (file == nullptr || inSub) return FALSE;

  //printIndent();
  //fprintf(file, "%s:\n", key.c_str());
  //indent++;
  //inSub = true;
  //return TRUE;

  return begin(key);
}

BOOL ValidateTxtWriter::write(const std::string& value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%s\n", value.c_str());
  return TRUE;
}

BOOL ValidateTxtWriter::write(I32 value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%d\n", value);
  return TRUE;
}

BOOL ValidateTxtWriter::write(const std::string& key, const std::string& value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%s: %s\n", key.c_str(), value.c_str());
  return TRUE;
}

BOOL ValidateTxtWriter::write(const std::string& key, I32 value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%s: %d\n", key.c_str(), value);
  return TRUE;
}

BOOL ValidateTxtWriter::write(const std::string& variable, const std::string& key, const std::string& note) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "- %s\n", key.c_str());

  indent++;
  printIndent();
  fprintf(file, "variable: %s\n", variable.c_str());
  if (!note.empty()) {
    printIndent();
    fprintf(file, "note: %s\n", note.c_str());
  }
  indent--;

  return TRUE;
}

BOOL ValidateTxtWriter::endsub(const std::string& key) {
  //if (file == nullptr || !inSub) return FALSE;

  //indent--;
  //inSub = false;
  //fprintf(file, "\n");
  //return TRUE;

  return end(key);
}

BOOL ValidateTxtWriter::end(const std::string& key) {
  if (file == nullptr || indent == 0) return FALSE;

  indent--;
  fprintf(file, "\n");
  return TRUE;
}
