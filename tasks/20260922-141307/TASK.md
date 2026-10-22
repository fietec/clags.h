# Autocompletion

- STATUS: OPEN
- PRIORITY: 100
- TAGS: v4.1.0,feature

Add the `clags_autocomplete` function to print the autocompletion for the current argv and config
to a file (`stdout` by default).
The challenge is to find a common input and output format that is easy to adapt by the various shells:
- some shells split `--output=file` into multipile arguments
- the same goes for outputting
- some shells allow meta data to display
