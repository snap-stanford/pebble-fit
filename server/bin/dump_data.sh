
# password.txt should 

# Database information 
HOST=ds111748.mlab.com:11748
DB=heroku_0cbvznft
u=pebble-fit

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
collections=( events activities users groups )

if [[ $# -gt 1 ]]; then
    echo "$0: Usage: dump_data.sh [out_dir]"
    exit 1
fi
out_dir=${1:-dump}

export_json() {
  for collection in "${collections[@]}"; do
    echo "Dumping ${db} ${collection}......"
    mongoexport -h ${HOST} -d ${DB} -u ${u} -c ${collection} \
        -o ${out_dir}/${collection}.json < ${DIR}/password.txt
  done

  # Change the output data directory name to the current date.
  mv ${out_dir} $(dirname ${out_dir})/$(date +%Y%m%d-%H:%M:%S)
}

export_json
