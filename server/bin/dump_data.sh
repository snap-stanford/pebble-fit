
# password.txt should contain the password the specific database.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# The list of collections that exist in the DB and of whom the data would be dumped.
collections=( events activities users groups )

# Database information 
HOST=ds111748.mlab.com:11748
DB=heroku_0cbvznft
u=pebble-fit
PASSWD=${DIR}/password.txt

# Input arguments check.
if [[ $# -gt 1 ]]; then
    echo "$0: Usage: dump_data.sh [out_dir]"
    exit 1
fi
out_dir=${1:-dump}

# Dump data as JSON files.
export_json() {
  ts_start=$1
  ts_end=$2
  output_name=$3

  for collection in "${collections[@]}"; do
    echo; echo "Try dumping database \"${DB}\", collection \"${collection}\" to ${out_dir}......"
    hash mongoexport >/dev/null 2>&1
    if [[ $? -eq 0 ]]; then
      # Use the system installation of mongoexport
      cmd="mongoexport"
    elif [[ -x ${DIR}/mongoexport ]]; then 
      # Use the local installation of mongoexport
      cmd="${DIR}/mongoexport"
    else
      echo "Error: could not find mongoexport!"
      exit 1
    fi
    out_file=${out_dir}/${output_name}/${collection}.json
    if [[ ${collection} == 'events' || ${collection} == 'activities' ]]; then
      query="--query \"{\\\"time\\\":{\\\$gt:new Date(${ts_start}),\\\$lte:new Date(${ts_end})}}\""
    else 
      query=""
    fi
  
    cmd+=" -h ${HOST} -d ${DB} -u ${u} -c ${collection} -o ${out_file} ${query} < ${PASSWD}"

    echo $cmd
    eval ${cmd}

    # Change permission to read only.
    chmod 444 ${out_file}
  done

  # Change permission to read only.
  chmod 544 ${out_dir}/${output_name}
}

# Dump the data 1 week ago as to capture the missing data.
ts_start=`date +%s -d "-7 days"`000
ts_end=`date +%s -d "-6 days"`000
output_name=`date +%Y%m%d-%H:%M:%S -d "-6 days"`
export_json ${ts_start} ${ts_end} ${output_name}

# Dump the daliy data.
ts_start=`date +%s -d "-1 days"`000
ts_end=`date +%s`000
output_name=`date +%Y%m%d-%H:%M:%S`-daily
export_json ${ts_start} ${ts_end} ${output_name}
