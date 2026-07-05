# Changelog

## [v3.5.2] - 05.07.2026
### Changes
- use O(1) address comparison for `clags_choice_index`

### Fixes
- check for pointer misallignment in `clags_subcmd_index` and `clags_choice_index`

---

## [v3.5.1] - 04.07.2026
### Changes
- improved duplicates validation: shadowing of reachable ancestor arguments is no longer allowed
- improved comments in implementation section
- use O(1) address comparison for `clags_subcmd_index`

---

## [v3.5.0] - 01.07.2026
### Added
- new custom strtoint api
  - `strtol`-like functions for all fixed-size integer types with two variants each
	- `strto(u)int<size>`: standard `strtol` behavior, uses `errno` for error propagation
	- `strto(u)int<size>_s`: safe/structured variant populating a result struct instead of just and `endptr`
  - `clags_strtoint_res_t`: the new result struct populated by the `_s` functions
	- conversion information: `endptr`, `negative`, `leading_spaces`
	- error flags: `invalid_base`, `no_digits`, `out_of_range`
### Changes
- all internal verifiers now use the new `clags_strtoint..` functions instead of `strtoll` and `strtoull`
- improved integer verfication error messages
- deprecated `CLAGS_STRTOLL` and `CLAGS_STRTOULL` redefinable macros since they are obsolete now

---

## [v3.4.1] - 23.06.2026
### Changes
- added compiler format checks to `clags_sb_appendf`

---

## [v3.4.0] - 23.06.2026
### Added
- new redefinable macros `CLAGS_STRTOLL`, `CLAGS_STRTOULL` and `CLAGS_STRTOD`
### Changes
- with the above macros, clags can now be configured to not depend on `stdlib.h`

---

## [v3.3.2] - 12.06.2026
### Added
- new example `06_time` to demonstrate the the `Clags_TimeS` and `Clags_TimeNS` types

### Changes
- renamed all examples starting with `06_1_custom` to `07_1_custom` and so on to make room for the time example
- changed the display type name of `Clags_TimeS` from `time_s` to `time` to hide this implementation detail from the user of the application
- updated documentation about the time value types to include the new compound time string support
- renamed some internal parameters to make the code more readable

---

## [v3.3.1] - 11.06.2026
### Changes
- `Clags_TimeS` and `Clags_TimeNS` values now support compound time strings like `1h2m17s`

---

## [v3.3.0] - 07.06.2026
### Added
- new `clags_config_duplicate` function
  - a more general version of `clags_config_duplicate_string` which works with a size slice or raw data

### Fixes
- `clags_free` had a critical bug where a wrong index was used for array indexing

## [v3.2.0] - 04.06.2026

### Added
- new `clags_usage_to_file` function
  - identical behavior to `clags_usage` but with a specifiable output file stream

---

## [v3.1.0] - 03.06.2026

### Added
- new recursive version of `clags_config_free`: `clags_free`

### Fixes
- parent flag suggestion now also works when a child config has inheritance disabled
- `clags_parse` no longer unnecessarily collects option and flags

---

## [v3.0.0] - 28.05.2026

### Added
- Argument inheritance:
  - added the `.inherit` flag to option and flag definitions
  - all such declared arguments are shared with all child configs automatically
  - added `.no_inheritance` flag to config options to disable inheritance for a specific config
- `clags_validate` is now part of the api to cover cases where `clags_usage` is called on a child config without the root config having been validated
- duplicate flag and option definitions are now detected for the entire scope of each config
- new `CLAGS_MAX_INLINE_CHOICES` macro: set the maximum amount of choices to print inline when `.print_no_details` is set
- new `Clags_Float` and `Clags_Double` value types
- new `Clags_Int`, `Clags_UInt` and `Clags_Real` value types with new custom ranges, respectively
  - custom ranges:
	- added `clags_<int/uint/real>_range(min, max)` macros and `.range` field for arguments of these types
- new `clags_custom_t` type for custom type definition:
  - you can now create a custom type with a name verification function and also additional custom arguments
  - this allows you to have multiple custom types share the same verification function
- exposed internal verifiers:
  - all verifiers such as `clags_verify_int32` are now part of the api
  - this allows custom types to inherit/extend functionality of already existing ones
- redefinable `CLAGS_PANIC` macro used by `clags_assert` and `clags_unreachable`
- new examples `06_2_custom_inheritance` and `10_allocator`
- added more tests

### Breaking Changes
- `.custom` field now expects a pointer to the new `clags_custom_t` struct
- removed `clags_custom_verify_func_t`, custom verifiers now use the same signature as all internal verifiers
- allocation macros are now parameterized, this is to add support for non-size-tracking custom allocators like arenas
  - `CLAGS_CALLOC(count, size)`
  - `CLAGS_REALLOC(ptr, oldsz, newsz)`
  - `CLAGS_FREE(ptr, sz)`
- removed `clags_args_t` from api since it was only used internally
- renamed `clags_timeS_list` and `clags_timeNS_list` to `clags_time_s_list` and `clags_time_ns_list`, respectively

### Changes
- completely re-wrote the parser and validator to cover more cases and make the code more readable
- removed dependency on `strings.h` and therefore no longer POSIX-dependent

### Fixes
- freeing duplicated strings now uses the correct size for custom allocators
  - a config's `allocs` list now stores pairs of allocated data and its size in bytes

---

## [v2.3.0] - 12.03.2026

### Added
- allow default input values for optional positional arguments

---

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
