# Example: Small Hardening Result

This is an illustrative example.
It is not a required output template.

Scope:
- one debug transport helper used from foreground only

Allowed hardening shape:
- add explicit argument validation at the public boundary
- document that the path is foreground-only
- preserve existing module ownership

Good result:
- the function returns a predictable failure when input is invalid
- the header comment makes the calling context explicit
- the change stays inside the current module or boundary pair

What this example avoids:
- renaming unrelated symbols
- rewriting the whole error model
- moving the module across layers
- folding style cleanup into the safety task unless it supports the hardening

How to report it:
- identify the concrete risk
- name the minimal fix
- state what was intentionally left untouched
