# Shared configs not detected in all cases

- STATUS: OPEN
- PRIORITY: 100
- TAGS:validation,release

A setup like this:
```
root
--- command-a
|   |-- shared-config
|-- command-b
    |-- shared-config
```
is currently not recognized. 
Maybe it is even worth rethinking the whole concept of config re-use completely.
