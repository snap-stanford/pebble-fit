
# This script is used to update either the configuration file for a specific 
# group or the messages file. 


HEROKU_MONGODB="ds111748.mlab.com:11748/heroku_0cbvznft"
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [[ $# -ne 2 && $# -ne 1 ]]; then
    echo $0: "Usage: update_config.sh file_path"
    exit 1
fi
file_path=$1

# Move the file to the proper location.
# TODO: need to move to Heroku server 
cp ${file_path} ${DIR}/../config 


# Update MongoDB.
if [[ "${file_path}" == *messages.json ]]; then
  group_filter=''
else
  group_name=`expr "$(basename ${file_path})" : '\(.*\)\.json'`
  group_filter='name: "'${group_name}'"'
fi
mongo_cmd='
  db.groups.updateMany(
    { '${group_filter}' },
    { 
      $currentDate: { configUpdatedAt: true } 
    }
  )'

echo ${mongo_cmd}
#mongo pebble-fit --eval "${mongo_cmd}"



# Leave -p as the last argument will force mongo to prompt for a password
#mongo ${HEROKU_MONGODB} --eval '
#  db.groups.update(
#    { "name": "'${group_name}'" },
#    { 
#      $set: { "file": "./config/'${group_name}'.json" },
#      $currentDate: { configUpdatedAt: true } 
#    },
#    { "upsert": true}
#  )' -u pebble-fit -p
# local mode
