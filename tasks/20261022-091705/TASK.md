# detect cyclic subcommand configuration

- STATUS: CLOSED
- PRIORITY: 100
- TAGS:bug,validation,release

Right now a config is only marked as validated when `clags_validate` finshes.
Therefore, if a recursive instance of `clags_validate` of a child subcommand that
falsely links back to the initial config an endless recursion occurs.
