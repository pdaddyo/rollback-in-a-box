#!/bin/sh
set -e
GODOT="${GODOT:-/Applications/Godot.app/Contents/MacOS/Godot}"
cd "$(dirname "$0")/.."

echo "=== import ==="
import_output=$("$GODOT" --headless --import 2>&1 || true)
if printf '%s\n' "$import_output" | grep -E 'SCRIPT ERROR:|Parse Error:|FAIL:' >/dev/null; then
  printf '%s\n' "$import_output"
  exit 1
fi

for t in test_smoke test_raw_api test_determinism test_compat test_session_loopback test_session_multiplayer test_partial_resim test_replay; do
  echo "=== $t ==="
  status=0
  output=$("$GODOT" --headless -s "tests/$t.gd" 2>&1) || status=$?
  printf '%s\n' "$output"
  if [ "$status" -ne 0 ] || printf '%s\n' "$output" | grep -E 'SCRIPT ERROR:|Parse Error:|FAIL:' >/dev/null; then
    exit 1
  fi
done

echo "=== all tests passed ==="
