/*
 * Minimal vendor shim for pboettch/json-schema-validator
 * This file provides a tiny, no-op implementation of json_schema::json_validator
 * so builds succeed when the full upstream validator isn't available in CI.
 * It is intentionally lightweight and only implements the interfaces used by
 * ProfileManager.cpp: `set_root_schema` and `validate`.
 *
 * NOTE: This shim does NOT perform real JSON Schema validation. If you need
 * runtime schema validation, replace this vendored shim with the upstream
 * library (or enable FetchContent to fetch it during configure).
 */

#pragma once

#include <nlohmann/json.hpp>

namespace nlohmann {
namespace json_schema {

class json_validator {
public:
  json_validator() noexcept {}
  ~json_validator() noexcept {}

  // Accept a schema (no-op)
  void set_root_schema(const nlohmann::json& /*schema*/) {}

  // Validate an instance against the previously set schema (no-op)
  void validate(const nlohmann::json& /*instance*/) const {}
};

} // namespace json_schema
} // namespace nlohmann
