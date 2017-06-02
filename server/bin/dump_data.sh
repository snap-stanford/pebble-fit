
# This script is used for dumping MongoDB data to the local server.
# password.txt should contain the password the specific database.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
MONGODB_DIR="${DIR}/../../../mongodb/bin"

# The list of collections that exist in the DB and of whom the data would be dumped.
collections=( events activities configs users groups messages references )

# Database information 
HOST=ds111748.mlab.com:11748
DB=heroku_0cbvznft
username=user
PASSWD_FILE=${DIR}/password.txt
PASSWD=$(cat ${PASSWD_FILE})

# Input arguments check.
if [[ $# -gt 1 ]]; then
    echo "$0: Usage: dump_data.sh [out_dir]"
    exit 1
fi
out_dir=${1:-mongodump}

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
  elif [[ -x ${MONGODB_DIR}/mongoexport ]]; then 
    # Use the local installation of mongoexport.
    cmd="${MONGODB_DIR}/mongoexport"
  else
    echo "Error: could not find mongoexport!"
    exit 1
  fi

  for collection in "${collections[@]}"; do
    echo; echo "Try dumping database \"${DB}\", collection \"${collection}\" to ${out_dir}......"
    
    out_file=${out_dir}/${output_name}/${collection}.json
    if [[ ${collection} == 'events' || ${collection} == 'activities' || \
          ${collection} == 'messages' ]]; then
      # Will only dump the data that has been uploaded within the specified range time.
      query="--query \"{\\\"created\\\":{\\\$gt:new Date(${ts_start}),\\\$lte:new Date(${ts_end})}}\""
      cmd_final=${cmd}" -h ${HOST} -d ${DB} -u ${username} -c ${collection} \
        -o ${out_file} ${query} < ${PASSWD_FILE}"

      echo ${cmd_final}
      eval ${cmd_final}
 
    elif [[ ${collection} == 'configs' ]]; then
      mongo_cmd='db.configs.aggregate([
        { $sort: { created: 1 } }, 
        { $group: {
            _id: "$watch", 
            id: {$last: "$_id"},
            watch: {$last: "$watch"},
            timeZone: {$last: "$timeZone"},
            startTime: {$last: "$startTime"},
            endTime: {$last: "$endTime"},
            breakFreq: {$last: "$breakFreq"},
            breakLen: {$last: "$breakLen"},
            threshold: {$last: "$threshold"},
            group: {$last: "$group"},
            created: {$last: "$created"} 
        }}]);'

      ${cmd} ${MONGODB_URI} --eval "${mongo_cmd}" -u ${username} -p ${PASSWD} \
        > ${out_dir}/${output_name}/${collection}.json

    else # groups, users and references collections
      # Will dump every user and group in the DB currently.
      #query="--query \"{\\\"configUpdatedAt\\\":{\\\$gt:new Date(${ts_start}),\\\$lte:new Date(${ts_end})}}\""
      query=""
  
      cmd_final=${cmd}" -h ${HOST} -d ${DB} -u ${username} -c ${collection} \
        -o ${out_file} ${query} < ${PASSWD_FILE}"

      echo ${cmd_final}
      eval ${cmd_final}
      jj
    fi
    # Change permission of the output files to read-only.
    chmod 444 ${out_file}
  done

  # Change permission of the output directory to read-only.
  chmod 554 ${out_dir}/${output_name}
}

# Dump the daliy data. This should be scheduled to run after midnight to dump the data
# collected from the previous day
ts_start=`date +%s -d " -2 days 23:59:59 "`000
ts_end=`date +%s -d " yesterday 23:59:59 "`000
output_name=`date +%Y%m%d-%H:%M:%S`
export_json ${ts_start} ${ts_end} ${output_name}
