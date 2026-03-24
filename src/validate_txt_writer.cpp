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

BOOL ValidateTxtWriter::is_open() const {
  return (BOOL)(file != nullptr);
}

/// Initializes the writer and opens the root element/container identified by the given key
BOOL ValidateTxtWriter::open(const std::string& key) {
  if (file == nullptr) return FALSE;

  fprintf(file, "%s\n", key.c_str());
  fprintf(file, "============\n");
  indent = 0;

  return TRUE;
}

/// Begins a new structured section identified by the given key and makes it the current write context
BOOL ValidateTxtWriter::begin(const std::string& key, ContainerType type) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%s:\n", key.c_str());
  indent++;

  return TRUE;
}

/// Begins a sub-section within the current section and makes it the active write context
BOOL ValidateTxtWriter::beginsub(const std::string& key, ContainerType type) {
  return begin(key);
}

/// Writes a simple value into the current context
BOOL ValidateTxtWriter::write(const std::string& value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%s\n", value.c_str());
  return TRUE;
}

/// Writes a numeric value into the current context
BOOL ValidateTxtWriter::write(I32 value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%d\n", value);
  return TRUE;
}

/// Writes a key-value pair into the current context
BOOL ValidateTxtWriter::write(const std::string& key, const std::string& value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%s: %s\n", key.c_str(), value.c_str());
  return TRUE;
}

/// Writes a numeric key-value pair into the current context
BOOL ValidateTxtWriter::write(const std::string& key, I32 value) {
  if (file == nullptr) return FALSE;

  printIndent();
  fprintf(file, "%s: %d\n", key.c_str(), value);
  return TRUE;
}

/// Writes a structured entry consisting of a variable identifier and optional descriptive note under the given key
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

/// Closes the current sub-section and restores the previous write context
BOOL ValidateTxtWriter::endsub(const std::string& key) {
  return end(key);
}

/// Closes the current section and restores the parent context; finalizes output if the root section is closed
BOOL ValidateTxtWriter::end(const std::string& key) {
  if (file == nullptr || indent == 0) return FALSE;

  indent--;
  fprintf(file, "\n");
  return TRUE;
}
