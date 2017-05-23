
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
MONGODB_DIR="${DIR}/../../../mongodb/bin"

MONGODB_URI="$(cat ${DIR}/db_uri.txt)"
PASSWD_FILE=${DIR}/password.txt

if [[ $# -ne 2 ]]; then
  echo "Error"
  echo "Usage: ./change_group.sh watch_id group_name"
  echo
  exit 1
fi

if [[ "$2" != "passive_tracking" && "$2" != "dail_message" && "$2" != "real_time_random" && "$2" != "real_time_action" && "$2" != "real_time_health" && "$2" != "real_time_outcome" ]]; then
  echo "Error"
  echo "Usage: ./change_group.sh watch_id group_name"
  echo "group_name must be  'passive_tracking', 'daily_message', 'real_time_random', 'real_time_action', 'real_time_health', or 'real_time_outcome'"
  echo
  exit 1
fi

# Choose the correct command.
hash mongo >/dev/null 2>&1
if [[ $? -eq 0 ]]; then
  # Use the system installation of mongo.
  cmd="mongo"
elif [[ -x ${MONGODB_DIR}/mongo ]]; then 
  # Use the local installation of mongo.
  cmd="${MONGODB_DIR}/mongo"
else
  echo "Error"
  echo "Could not find the command mongo!"
  exit 1
fi

watch_id=$1
group_name=$2

# Update the group name for the given user.
mongo_cmd='
  db.users.updateOne(
    { "watch" : "'${watch_id}'" },
    { 
      $currentDate: { lastModified: true },
      $set: { "group" : "'${group_name}'" } 
    }
  )'

echo ${mongo_cmd}
#${cmd} pebble-fit --eval "${mongo_cmd}"
${cmd} ${MONGODB_URI} --eval "${mongo_cmd}" -u admin -p < ${PASSWD_FILE}

# Update the timestamp for the new group so that the server will send this info to watch.
mongo_cmd='
  db.groups.updateMany(
    { "name" : "'${group_name}'" },
    { 
      $currentDate: { configUpdatedAt: true } 
    }
  )'

echo ${mongo_cmd}
#${cmd} pebble-fit --eval "${mongo_cmd}"
${cmd} ${MONGODB_URI} --eval "${mongo_cmd}" -u admin -p < ${PASSWD_FILE}

