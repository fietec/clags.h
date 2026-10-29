# Improve time verifier error messages

- STATUS: CLOSED
- PRIORITY: 70
- TAGS:release,improvement

`clags_verify_time_s` treats units accepted by `clags_verify_time_ns` e.g. `ns` the same as some arbitrary unit.
It would be nice to have better error messages to indicate that for the current argument sub-second units are not allowed.
