# Changelog

## [v2.2.0] - 05.03.2026

### Added
- `Clags_Number` value type: allows specifying numbers with arbitrary ranges.
  - Use `clags_range_t` and `.range` to define `min` and `max` values.

---

## [v2.1.0] - 04.03.2026

### Added
- support for specifying default input values for arguments via the `.default_input` field
  - default inputs are parsed according to the argument’s `value_type`
  - it is the caller’s responsibility to ensure the default string represents a valid value
  - default inputs are **not supported** for option lists
- added documentation and example usage for `.default_input`

### Changes
- improved error messages related to default input parsing
- removed generic error message for custom verifiers

---

## [v2.0.0] - 03.03.2026

### Breaking Changes
- `clags_list` now requires an explicit `clags_value_type_t`
- lists are now strictly type-safe

### Added
- `clags_path_type`, `clags_path_type_t` and `clags_path_type_name` for filesystem path classification
- improved list validators and stronger type consistency checks

### Changes
- refined error reporting for type mismatches

### Fixes
- removed extra newline in detailed choice usage output

---

## [v1.1.0] - 15.02.2026

### Added
- new `ignored_args` config option: allows the user to keep track of the arguments ignored via the `ignore_prefix`

---

## [v1.0.0] - 06.02.2026
the initial release
