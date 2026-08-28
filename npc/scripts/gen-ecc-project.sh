#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 6 ]]; then
  echo "usage: $0 PROJECT_DIR DESIGN_NAME RTL_DIR PDK_DIR TOP FREQ_MHZ" >&2
  exit 2
fi

project_dir=$(realpath -m "$1")
design_name=$2
rtl_dir=$(realpath "$3")
pdk_dir=$(realpath "$4")
top=$5
freq_mhz=$6
source_filelist="$rtl_dir/filelist.f"

mkdir -p "$project_dir/rtl"
: >"$project_dir/rtl/filelist.f"
while IFS= read -r source; do
  [[ -z "$source" ]] && continue
  printf '%s\n' "$rtl_dir/$source" >>"$project_dir/rtl/filelist.f"
done <"$source_filelist"

cat >"$project_dir/ecc.toml" <<EOF
[design]
name = "$design_name"
top = "$top"
rtl = ["rtl/filelist.f"]
clock_port = "clock"
frequency_mhz = $freq_mhz

[pdk]
name = "ics55"
root = "$pdk_dir"

[flow]
# Logic synthesis and static timing analysis only; no SoC or physical design.
preset = "syn_sta"
run = "default"

[pdk.overrides]
libs = [
  "$pdk_dir/IP/STD_cell/ics55_LLSC_H7C_V1p10C100/ics55_LLSC_H7CR/liberty/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib",
  "$pdk_dir/IP/STD_cell/ics55_LLSC_H7C_V1p10C100/ics55_LLSC_H7CL/liberty/ics55_LLSC_H7CL_typ_tt_1p2_25_nldm.lib",
]
lefs = [
  "$pdk_dir/IP/STD_cell/ics55_LLSC_H7C_V1p10C100/ics55_LLSC_H7CR/lef/ics55_LLSC_H7CR_ecos.lef",
  "$pdk_dir/IP/STD_cell/ics55_LLSC_H7C_V1p10C100/ics55_LLSC_H7CL/lef/ics55_LLSC_H7CL_ecos.lef",
]
dont_use = [
  "*AO222*",
  "*2BB2*",
  "*AOI222*",
  "*AOI33*",
  "*OA222*",
  "*OAI222*",
  "*OAI33*",
  "*NOR4*",
]
EOF
