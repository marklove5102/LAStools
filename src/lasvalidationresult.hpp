#pragma once

#include "mydefs.hpp"
#include <string>
#include <vector>

enum class ValidationStatus { 
  passed, 
  warning, 
  failed 
};

struct ValidationMessage {
  std::string key;
  std::string note;
};

struct ValidationResult {
  ValidationStatus status = ValidationStatus::passed;
  std::vector<ValidationMessage> fail_messages;
  std::vector<ValidationMessage> warning_messages;
  U32 fail_count = 0;
  U32 warning_count = 0;

  void add_fail(const std::string& key, const std::string& note) {
    fail_messages.push_back({key, note});
    status = ValidationStatus::failed;
    fail_count++;
  }

  void add_warning(const std::string& key, const std::string& note) {
    warning_messages.push_back({key, note});
    status = ValidationStatus::warning;
    warning_count++;
  }

  ValidationStatus passed() const {
    return status;
  }
};
