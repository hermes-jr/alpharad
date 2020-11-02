#!/bin/bash
#  Copyright (C) 2020 Mikhail Antonov <hermes@cyllene.net>
#
#  This file is part of alpharad project.
#
#  alpharad is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  alpharad is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with alpharad.  If not, see <https://www.gnu.org/licenses/>.

threads=12
# Modify to balance speed vs detail
batch_size=512
#export MPLBACKEND="ps"

# 32 for sha256, 1 for default and 1/8 for comparator
data_to_points_ratio='32'

calculated_ratio=$(bc <<<"scale=4; ${data_to_points_ratio}")
total_points="$(wc -l ../points.log | cut -d' ' -f1)"
((leap = batch_size * threads))

# Temporary working directory
TMP_DIR="$(mktemp -t -d render-saved-XXXXXXXXXX)"
echo "Working in: ${TMP_DIR}"
function cleanup() {
  echo "Removing temporary directory: ${TMP_DIR}"
  rm -r "${TMP_DIR}"
}
trap cleanup EXIT

# Distribute tasks across multiple processes
for cur_thread in $(seq 0 $((threads - 1))); do
  {
    ((offset = cur_thread * batch_size))
    points_file="${TMP_DIR}/points_slice_${cur_thread}.log"
    data_file="${TMP_DIR}/out_slice_${cur_thread}.dat"

    seq ${offset} ${leap} "${total_points}" | while read -r n; do
      [ ! -d "${TMP_DIR}" ] && echo "Exiting thread ${cur_thread}" && break

      printf "Thread %2d: Line %d/%d: %5.2f%%\n" "${cur_thread}" "${n}" "${total_points}" "$(bc <<<"scale=2; 100*${n}/${total_points}")"

      head -n "${n}" ../points.log >"${points_file}"
      head -c "$(bc <<<"scale=0; ${n}*(${calculated_ratio})/1")" ../out.dat >"${data_file}"

      python3 -O coords_to_scatter.py --points "${points_file}"
      python3 -O plot_data.py --data "${data_file}" --points "${points_file}"
    done
  } &
done

# Wait for backgrounds jobs to finish before cleaning up
wait
