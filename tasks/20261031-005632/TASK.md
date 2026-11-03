# Shared configs not detected in all cases

- STATUS: CLOSED
- PRIORITY: 100
- TAGS:validation

A setup like this:
```
root
--- command-a
|   |-- shared-config
|-- command-b
    |-- shared-config
```
is currently not recognized.
