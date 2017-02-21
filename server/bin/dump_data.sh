
# password.txt should contain the password the specific database.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DATE=$(date +%Y%m%d-%H:%M:%S)
collections=( events activities users groups )

# Database information 
HOST=ds111748.mlab.com:11748
DB=heroku_0cbvznft
u=pebble-fit
PASSWD=${DIR}/password.txt

mongoexport_params=""

if [[ $# -gt 1 ]]; then
    echo "$0: Usage: dump_data.sh [out_dir]"
    exit 1
fi
out_dir=${1:-dump}

export_json() {
  for collection in "${collections[@]}"; do
    echo; echo "Try dumping database ${DB}'s collection ${collection} to ${out_dir}......"
    hash foo >/dev/null 2>&1
    if [[ $? -eq 0 ]]; then
      # Use the system installation of mongoexport
      BIN=mongoexport
    elif [[ -x ${DIR}/mongoexport ]]; then 
      # Use the local installation of mongoexport
      BIN=${DIR}/mongoexport
    else
      echo "Error: could not find mongoexport!"
      exit 1
    fi
    out_file=${out_dir}/${DATE}/${collection}.json
    $BIN -h ${HOST} -d ${DB} -u ${u} -c ${collection} -o ${out_file} < ${PASSWD}

    # Change permission to read only.
    chmod 444 ${out_file}
  done

  # Change permission to read only.
  chmod 544 ${out_dir}/${DATE}
}

export_json
