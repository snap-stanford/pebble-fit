
# This script is used to update the configuration file. 
# The current configuration files will be moved to a storage directory first, and the
# the new configurations files will be pushed onto the server. Finally, the group
# settings on the server will be updated to reflect that new configuration available.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
HEROKU_MONGODB="ds111748.mlab.com:11748/heroku_0cbvznft"
PASSWD=${DIR}/password.txt

if [[ $# -ne 1 ]]; then
    echo $0: "Usage: update_config.sh configuration_directory"
    exit 1
fi
new_config_dir=$1/new_configuration
storage_dir=$1/old_configuration

# Store the current configuration files and the replace with the new configuration files.
new_storage_dir="${storage_dir}/`date +%Y%m%d-%H:%M:%S`"
mkdir -p ${new_storage_dir}
cp ${DIR}/../config/* ${new_storage_dir}
chmod 444 ${new_storage_dir}/*
chmod 554 ${new_storage_dir}

cp ${new_config_dir}/* ${DIR}/../config/

# Push new configuration files to the server. Note that this assumes the new configuration
# files have been git commited. 
# TODO: is there an easier way to transfer files to Heroku server?
#${DIR}/../../bin/push_server.sh

# Update MongoDB. TODO: now we will update timestamp of all groups. Later on, we 
# might need to detect which file has been modified and only update those groups
# selectively.
group_filter=''
#if [[ "${file_path}" == *messages.json ]]; then
#  group_filter=''
#else
#  group_name=`expr "$(basename ${file_path})" : '\(.*\)\.json'`
#  group_filter='name: "'${group_name}'"'
#fi

mongo_cmd='
  db.groups.updateMany(
    { '${group_filter}' },
    { 
      $currentDate: { configUpdatedAt: true } 
    }
  )'

# Choose the correct command.
hash mongo >/dev/null 2>&1
if [[ $? -eq 0 ]]; then
  # Use the system installation of mongo.
  cmd="mongo"
elif [[ -x ${DIR}/mongo ]]; then 
  # Use the local installation of mongo.
  cmd="${DIR}/mongo"
else
  echo "Error: could not find mongo!"
  exit 1
fi

# local mode (assuming the local mongod is running, and there is a pebble-fit DB)
#${cmd} pebble-fit --eval "${mongo_cmd}"

# Leave -p as the last argument will force mongo to prompt for a password
echo ${mongo_cmd}
${cmd} ${HEROKU_MONGODB} --eval "${mongo_cmd}" -u admin -p < ${PASSWD}
#${cmd} ${HEROKU_MONGODB} --eval "db.groups.find()" -u user -p < ${PASSWD}
#${cmd} ${HEROKU_MONGODB} --eval '
#  db.groups.update(
#    { "name": "'${group_name}'" },
#    { 
#      $set: { "file": "./config/'${group_name}'.json" },
#      $currentDate: { configUpdatedAt: true } 
#    },
#    { "upsert": true}
#  )' -u admin -p < ${PASSWD}


