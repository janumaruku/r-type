#!/bin/bash
set -euo pipefail

# Turns the errors of a build log into GitHub annotations: compiler errors
# (GCC/Clang "file:line:col: error:" and clang-cl "file(line,col): error:"),
# CMake errors ("CMake Error at file:line") and linker errors.

LOG="${1:-}"

if [[ -z "$LOG" ]]; then
  echo "::error::No build log provided"
  exit 1
fi

if [[ ! -f "$LOG" ]]; then
  echo "No build log at $LOG: nothing to annotate"
  exit 0
fi

WORKSPACE="${GITHUB_WORKSPACE:-$PWD}"
WORKSPACE="${WORKSPACE//\\//}/"

echo "::group::Build errors"

awk -v workspace="$WORKSPACE" '
  function escape(text) {
    gsub(/\r/, "", text)
    gsub(/%/, "%25", text)
    return text
  }

  function relative(path) {
    gsub(/\\/, "/", path)
    if (tolower(substr(path, 1, length(workspace))) == tolower(workspace))
      path = substr(path, length(workspace) + 1)
    return path
  }

  function annotate(file, line, title, message,    key) {
    key = file ":" line ":" message
    if (key in seen)
      return
    seen[key] = 1
    if (file == "")
      printf "::error title=%s::%s\n", title, escape(message)
    else
      printf "::error file=%s,line=%s,title=%s::%s\n", \
        relative(file), line, title, escape(message)
  }

  cmakeFile != "" {
    if ($0 ~ /^[ \t]+[^ \t]/) {
      message = $0
      sub(/^[ \t]+/, "", message)
      annotate(cmakeFile, cmakeLine, "CMake error", message)
      cmakeFile = ""
      next
    }
    if ($0 !~ /^[ \t\r]*$/)
      cmakeFile = ""
  }

  /^CMake Error at / {
    rest = substr($0, length("CMake Error at ") + 1)
    if (match(rest, /:[0-9]+ \(/)) {
      cmakeFile = substr(rest, 1, RSTART - 1)
      cmakeLine = substr(rest, RSTART + 1, RLENGTH - 3)
    }
    next
  }

  match($0, /:[0-9]+:[0-9]+: (fatal )?error: /) {
    file = substr($0, 1, RSTART - 1)
    split(substr($0, RSTART + 1), position, ":")
    annotate(file, position[1], "Compilation error", substr($0, RSTART + RLENGTH))
    next
  }

  match($0, /\([0-9]+,[0-9]+\) ?: (fatal )?error[^:]*: /) {
    file = substr($0, 1, RSTART - 1)
    split(substr($0, RSTART + 1), position, ",")
    annotate(file, position[1], "Compilation error", substr($0, RSTART + RLENGTH))
    next
  }

  /undefined reference to|multiple definition of|undefined symbol:|^lld-link: error:/ {
    message = $0
    sub(/^[ \t]+/, "", message)
    annotate("", "", "Linker error", message)
  }
' "$LOG"

echo "::endgroup::"
