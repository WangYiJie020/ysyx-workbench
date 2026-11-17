#!/usr/bin/env bash

set -euo pipefail

if [[ $# -eq 0 ]]; then
  echo "usage: $0 ECC_PROJECT_DIR..." >&2
  exit 2
fi

target_mhz=${ECC_FREQ_MHZ:-400}
minirv_area_limit=${ECC_MINIRV_AREA_LIMIT:-10000}
riscv32e_area_limit=${ECC_RISCV32E_AREA_LIMIT:-11000}
result=0

printf '%-10s %10s %10s %10s %8s %8s %12s %12s %12s %12s %12s\n' \
  ISA TARGET_MHZ FREQ_MHZ WNS_NS STATUS NVP CELLA_UM2 AREA_LIMIT AREA_STATUS DIE_MM2 COST_CNY

for project_dir in "$@"; do
  isa=$(basename "$project_dir")
  report="$project_dir/runs/default/Synthesis_yosys/report/post_synthesis/qor_summary.rpt"
  if [[ ! -s "$report" ]]; then
    echo "missing ECC QoR report: $report" >&2
    exit 1
  fi

  read -r wns nvp freq < <(
    awk '$1 == "Summary" { gsub(/MHz/, "", $5); print $2, $4, $5; exit }' "$report"
  )
  area=$(
    awk '
      /CELLA/ { found = 1; next }
      found && $1 !~ /^-+$/ { print $5; exit }
    ' "$report"
  )

  if [[ -z "$wns" || -z "$nvp" || -z "$freq" || -z "$area" ]]; then
    echo "failed to parse ECC QoR report: $report" >&2
    exit 1
  fi

  die_area=$(awk -v area="$area" 'BEGIN { printf "%.5f", area / 1000000.0 / 0.60 }')
  cost=$(awk -v area="$area" 'BEGIN { printf "%.2f", area / 1000000.0 / 0.60 * 30000.0 }')
  status=$(awk -v wns="$wns" 'BEGIN { print (wns >= 0) ? "MET" : "VIOLATED" }')
  case "$isa" in
    minirv) area_limit=$minirv_area_limit ;;
    riscv32e) area_limit=$riscv32e_area_limit ;;
    *) echo "unknown ISA for area limit: $isa" >&2; exit 1 ;;
  esac
  area_status=$(awk -v area="$area" -v limit="$area_limit" \
    'BEGIN { print (area < limit) ? "PASS" : "FAIL" }')
  if [[ "$status" != MET || "$area_status" != PASS ]]; then
    result=1
  fi
  printf '%-10s %10s %10s %10s %8s %8s %12s %12s %12s %12s %12s\n' \
    "$isa" "$target_mhz" "$freq" "$wns" "$status" "$nvp" "$area" "$area_limit" "$area_status" "$die_area" "$cost"
done

echo
echo "Cost estimate: CELLA / 1e6 / 60% * CNY 30000/mm^2; CPU core only."
exit "$result"
