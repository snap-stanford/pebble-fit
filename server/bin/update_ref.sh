# This script is used to update the reference score.
# The raw data is fetched from a local directory (created by another script) and then parsed.
# Use MongoDB command to update the database on Heroku.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
MONGODB_DIR="${DIR}/../../../mongodb/bin"

LOCAL_REF_DIR=/datamb/pebble/configuration/reference

MONGODB_URI="$(cat ${DIR}/db_uri.txt)"
PASSWD_FILE=${DIR}/password.txt

if [[ $# -ne 0 ]]; then
    echo "Usage: ./update_ref.sh"
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
  echo "Error: could not find mongo!"
  exit 1
fi

if [[ ! -d ${LOCAL_REF_DIR} ]]; then
    echo $0: "Error: ${LOCAL_REF_DIR} is not a valid directory."
    exit 1
fi

# Get the actual names of the generated reference files.
# Using ls -t to get the latest file with the same pattern.
for f in $(ls -t ${LOCAL_REF_DIR}/hourly_*.csv); do
  if [[ -e "$f" ]]; then
    HOUR_FILE=${f} 
  else 
    echo "${f} do not exist."
    exit 1
  fi

  break
done

for f in $(ls -t ${LOCAL_REF_DIR}/best_*.csv); do
  if [[ -e "$f" ]]; then
    BEST_FILE=${f} 
  else 
    echo "${f} do not exist."
    exit 1
  fi

  break
done

echo "File to be parsed:"
echo ${HOUR_FILE}
echo ${BEST_FILE}

time_range=$(expr "$BEST_FILE" : '.*best_\([0-9_]*\).csv')

while IFS=, read c1 watch c3 c4 c5 c6 c7 c8 c9 c10 best_score
do
    # Skip the first title line.
    if [[ $c1 ]]; then
        echo; echo "=>$watch:$best_score"

        # Update MongoDB. 
        mongo_cmd='
          db.references.updateOne(
            { "watch" : "'${watch}'" },
            { 
              $set: { "best": '${best_score}', "range": "'${time_range}'" } 
            }, 
            {}
          )'
        echo ${mongo_cmd}
        
        # local mode (assuming the local mongod is running, and there is a pebble-fit DB)
        #${cmd} pebble-fit --eval "${mongo_cmd}"
        
        # Leave -p as the last argument will force mongo to prompt for a password
        # Here we supply password stored in a local file defined by the variable $PASSWD
        ${cmd} ${MONGODB_URI} --eval "${mongo_cmd}" -u admin -p < ${PASSWD_FILE}
    fi
done < "${BEST_FILE}"

# Check for a valid float number.
re='^[0-9]+([.][0-9]+)?$'

while IFS=, read c1 watch average_score
do
    # Skip the first title line.
    if [[ $c1 ]]; then
        echo; echo "=>$watch:$average_score"

        # Prepare the MongoDB query.
        mongo_cmd='
          db.references.updateOne(
            { "watch" : "'${watch}'" },
            { 
              $set: { "average": ['
        
        # Append the list of reference score
        count=0
        IFS=', ' read -r -a array <<< "${average_score}"
        for i in "${array[@]}"; do
          if [[ ${i} && ${i} =~ $re ]]; then
            mongo_cmd="${mongo_cmd}${i},"
            let count+=1
          fi
        done

        # Remove the trailing ','
        mongo_cmd=${mongo_cmd%,} 

        #while IFS=',' read -r _ second _; do
        #    echo
        #    mongo_cmd=${mongo_cmd}${score}
        #done < file.txt

        # Complete the MongoDB query. 
        mongo_cmd=${mongo_cmd}'] ,
              "range": "'${time_range}'" ,
              "count": '${count}' } 
            }, 
            {}
          )'
        echo ${mongo_cmd}
        
        # local mode (assuming the local mongod is running, and there is a pebble-fit DB)
        #${cmd} pebble-fit --eval "${mongo_cmd}"
        
        # Leave -p as the last argument will force mongo to prompt for a password
        # Here we supply password stored in a local file defined by the variable $PASSWD
        ${cmd} ${MONGODB_URI} --eval "${mongo_cmd}" -u admin -p < ${PASSWD_FILE}
    fi
done < "${HOUR_FILE}"

echo "Successfully upload reference scores to the server."
