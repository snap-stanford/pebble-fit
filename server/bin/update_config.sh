
# This script is used to update the configuration file. 
# The current configuration files will be moved to a storage directory first, and the
# the new configurations files will be pushed onto the server. Finally, the group
# settings on the server will be updated to reflect that new configuration available.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SERVER_CONFIG_DIR=${DIR}/../config
HEROKU_MONGODB="ds111748.mlab.com:11748/heroku_0cbvznft"
PASSWD=${DIR}/password.txt

if [[ $# -ne 1 ]]; then
    echo $0: "Usage: update_config.sh configuration_directory"
    exit 1
fi

if [[ ! -d $1 ]]; then
    echo $0: "Error: $1 is not a valid directory."
    exit 1
fi

# Assume we use the same mb server to store the local config files, so these are hard-coded.
new_config_dir=$1/new_configuration
storage_dir=$1/old_configuration

if [[ ! -d ${new_config_dir} ]]; then
    echo $0: "Error: ${new_config_dir} is not a valid directory."
    exit 1
fi
if [[ ! -d ${storage_dir} ]]; then
    echo $0: "Error: ${storage_dir} is not a valid directory."
    exit 1
fi

# Store and keep a record of the new configuration files for future reference.
new_storage_dir="${storage_dir}/`date +%Y%m%d-%H:%M:%S`"
mkdir -p ${new_storage_dir}
#cp ${DIR}/../config/* ${new_storage_dir} # Copy the old config to the storage dir.
cp ${new_config_dir}/* ${new_storage_dir} # Copy the new config to the storage dir.
chmod 444 ${new_storage_dir}/*
chmod 554 ${new_storage_dir}

# Move the new configuration files to the proper location.
cp ${new_config_dir}/* ${SERVER_CONFIG_DIR}

# Convert the CSV files to JSON files.
cd ${SERVER_CONFIG_DIR}
for f in *.csv; do 
  echo "Processing $f file..."
  python ${DIR}/csv2json.py $f .
done
cd -

# Push new configuration files to the server. First we have to commit the changes.
# Note that some files are not to be commited, since they are not used by the backend logic.
# TODO: is there an easier way to transfer files to Heroku server?
cd ${DIR}/../.. # cd to pebble-fit
rm ${SERVER_CONFIG_DIR}/*.csv
rm ${SERVER_CONFIG_DIR}/constants.h
git add ${SERVER_CONFIG_DIR} && \
  git commit -m "Update configuration files." && \
  git push origin master
./bin/push_server.sh
cd -

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
# Here we supply password stored in a local file defined by the variable $PASSWD
echo ${mongo_cmd}
${cmd} ${HEROKU_MONGODB} --eval "${mongo_cmd}" -u admin -p < ${PASSWD}

echo
echo "Successfully upload configuration files to the server."
