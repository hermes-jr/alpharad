#!/bin/bash

threads=10
# Modify to balance speed vs detail
batch_size=256

total_points=$(wc -l ../points.log | cut -d' ' -f1)
let leap=${batch_size}*${threads}

# Temporary working directory
TMPDIR="$(mktemp -t -d render-saved-XXXXXXXXXX)"
echo "Working in: ${TMPDIR}"
function cleanup() {
    echo "Cleaning up: ${TMPDIR}"
    rm -r "${TMPDIR}"
}
trap cleanup EXIT

for cur_thread in $(seq 0 $((${threads} - 1)))
do
	{
		let offset=${cur_thread}*${batch_size}
		points_file="${TMPDIR}/points_slice_${cur_thread}.log"
		data_file="${TMPDIR}/out_slice_${cur_thread}.dat"

		seq ${offset} ${leap} ${total_points} | while read n
		do
			printf "Thread %2d: Line %d/%d: %5.2f%%\n" ${cur_thread} ${n} ${total_points} $(echo "scale=2; 100*${n}/${total_points}" | bc)
		    head -n $n ../points.log > "${points_file}"
			head -c $((${n}/8)) ../out.dat > "${data_file}"
#			python3 coords-to-scatter.py
			python3 plot-data.py --data ${data_file} --points ${points_file} -v
		done
	}&
done

# Wait for backgrounds jobs to finish before cleaning up
wait
