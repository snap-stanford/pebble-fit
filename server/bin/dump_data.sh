
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

  # Choose the correct command.
  hash mongoexport >/dev/null 2>&1
  if [[ $? -eq 0 ]]; then
    # Use the system installation of mongoexport.
    cmd="mongoexport"
  elif [[ -x ${DIR}/mongoexport ]]; then 
    # Use the local installation of mongoexport.
    cmd="${DIR}/mongoexport"
  else
    echo "Error: could not find mongoexport!"
    exit 1
  fi

  for collection in "${collections[@]}"; do
    echo; echo "Try dumping database \"${DB}\", collection \"${collection}\" to ${out_dir}......"
    
    out_file=${out_dir}/${output_name}/${collection}.json
    if [[ ${collection} == 'events' || ${collection} == 'activities' ]]; then
      query="--query \"{\\\"created\\\":{\\\$gt:new Date(${ts_start}),\\\$lte:new Date(${ts_end})}}\""
    else # groups and users collection
      query="--query \"{\\\"configUpdatedAt\\\":{\\\$gt:new Date(${ts_start}),\\\$lte:new Date(${ts_end})}}\""
    fi
  
    cmd_final=${cmd}" -h ${HOST} -d ${DB} -u ${u} -c ${collection} -o ${out_file} ${query} < ${PASSWD}"

    echo ${cmd_final}
    eval ${cmd_final}

    # Change permission of the output files to read-only.
    chmod 444 ${out_file}
  done

  # Change permission of the output directory to read-only.
  chmod 554 ${out_dir}/${output_name}
}

# Dump the daliy data.
ts_start=`date +%s -d "-1 day"`000
ts_end=`date +%s`000
output_name=`date +%Y%m%d-%H:%M:%S`
export_json ${ts_start} ${ts_end} ${output_name}
