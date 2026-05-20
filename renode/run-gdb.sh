#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
renode_bin="${RENODE:-/opt/renode/renode}"
gdb_bin="${GDB:-gdb-multiarch}"
gdb_port="${GDB_PORT:-3333}"
gdb_script="$repo_dir/start.gdb"
export DOTNET_BUNDLE_EXTRACT_BASE_DIR="${DOTNET_BUNDLE_EXTRACT_BASE_DIR:-${TMPDIR:-/tmp}/renode-dotnet}"

usage() {
  printf 'Usage:\n' >&2
  printf '  %s server path/to/program.elf\n' "$(basename "$0")" >&2
  printf '  %s gdb path/to/program.elf\n' "$(basename "$0")" >&2
  printf '\n' >&2
  printf 'Modes:\n' >&2
  printf '  server  Start Renode with its GDB server enabled.\n' >&2
  printf '  gdb     Start GDB and load startup commands from %s.\n' "$gdb_script" >&2
  printf '\n' >&2
  printf 'The ELF path is required. The flash image for server mode is derived by\n' >&2
  printf 'replacing the ELF suffix with .bin, for example foo.elf -> foo.bin.\n' >&2
  printf '\n' >&2
  printf 'Set GDB_PORT to override the default Renode GDB server port 3333.\n' >&2
  printf 'Set GDB or RENODE to override the executable used by each mode.\n' >&2
}

abs_path() {
  local path="$1"
  printf '%s/%s\n' "$(cd "$(dirname "$path")" && pwd)" "$(basename "$path")"
}

require_elf() {
  local elf="$1"
  if [[ ! -f "$elf" ]]; then
    printf 'error: ELF not found: %s\n' "$elf" >&2
    exit 2
  fi
  if [[ "$elf" != *.elf ]]; then
    printf 'error: ELF path must end with .elf: %s\n' "$elf" >&2
    exit 2
  fi
}

run_server() {
  local elf="$1"
  local elf_abs flash_image tmp_resc

  require_elf "$elf"
  elf_abs="$(abs_path "$elf")"
  flash_image="${elf_abs%.elf}.bin"
  if [[ ! -f "$flash_image" ]]; then
    printf 'error: flash image not found: %s\n' "$flash_image" >&2
    printf '       expected a .bin next to the ELF with the same basename\n' >&2
    exit 2
  fi

  tmp_resc="$(mktemp "${TMPDIR:-/tmp}/ysyxsoc-gdb.XXXXXX.resc")"
  trap 'rm -f "$tmp_resc"' EXIT

  cat >"$tmp_resc" <<EOF
\$elf=@$elf_abs
\$flash_image=@$flash_image
\$gdb_port=$gdb_port
include @$repo_dir/ysyxsoc-gdb.resc
EOF

  printf 'Starting Renode for ELF: %s\n' "$elf_abs"
  printf 'Flash image: %s\n' "$flash_image"
  printf 'GDB command:\n'
  printf '  %q gdb %q\n' "$repo_dir/run-gdb.sh" "$elf_abs"
  if [[ "$gdb_port" != "3333" ]]; then
    printf 'Note: update %q to connect to port %s before running GDB.\n' "$gdb_script" "$gdb_port"
  fi
  printf '\n'

  exec "$renode_bin" --disable-gui --console -P -1 "$tmp_resc"
}

run_gdb() {
  local elf="$1"
  local elf_abs

  require_elf "$elf"
  elf_abs="$(abs_path "$elf")"

  exec "$gdb_bin" --command="$gdb_script" "$elf_abs"
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

if [[ $# -ne 2 ]]; then
  usage
  exit 2
fi

echo "run-gdb.sh: target ARCH=$ARCH"

case "$1" in
  server | gdbserver)
    run_server "$2"
    ;;
  gdb)
    run_gdb "$2"
    ;;
  *)
    printf 'error: unknown mode: %s\n' "$1" >&2
    usage
    exit 2
    ;;
esac
