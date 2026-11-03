# Shared configs not detected in all cases

- STATUS: OPEN
- PRIORITY: 30
- TAGS:validation

A setup like this:
```
root
--- command-a
|   |-- shared-config
|-- command-b
    |-- shared-config
```
is currently not recognized. However, in comparison to cyclic config setups this does not seem a big issue
and if it were adressed it should probably just raise a `Clags_ConfigWarning`
