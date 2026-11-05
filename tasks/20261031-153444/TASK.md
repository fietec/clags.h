# Use string builder for all usage generation

- STATUS: OPEN
- PRIORITY: 30
- TAGS:improvement

This would unify the usage printing and help to remove artifacts, such as
`inherit_options_count`. On the other hand, this means more dynamic allocation
that is technically not really needed.
