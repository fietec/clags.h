# Changelog

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
