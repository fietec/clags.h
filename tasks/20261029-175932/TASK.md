# Incorrect path verifier error messages

- STATUS: CLOSED
- PRIORITY: 100
- TAGS:bug,release

Currently, all the path verifiers use `strerror(errno)` for error logging.
However, on windows, we use `GetFileAttributesW` to get the path type, which does
not set errno. Therefore, we either have set errno manually in `clags_path_type`
or change the value printing to use `GetLastError` instead.
