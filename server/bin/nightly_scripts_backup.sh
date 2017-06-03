HOME="/datamb/pebble"
SERVER_DIR="/datamb/pebble/dean-pebble-fit/server"
JESS_DIR="${HOME}/jess"

echo "Running $0..."
date

# Download data from Heroku server to this server.
${SERVER_DIR}/bin/dump_data.sh /datamb/pebble/data >>${HOME}/logs/crontab_dump_data.log 2>&1
exit 0

# Run analysis scripts.
jupyter nbconvert --execute ${JESS_DIR}/PebbleParseUsers_V3.ipynb         >>${HOME}/logs/crontab_ipynb.log 2>&1
jupyter nbconvert --execute ${JESS_DIR}/PebbleParseDays_V4.ipynb          >>${HOME}/logs/crontab_ipynb.log 2>&1
jupyter nbconvert --execute ${JESS_DIR}/PebbleAnalyzeUserDays_V15.ipynb   >>${HOME}/logs/crontab_ipynb.log 2>&1

# Update reference scores.
${SERVER_DIR}/bin/update_ref.sh >>${HOME}/logs/crontab_update_ref.log 2>&1

echo "Finish running $0."
date
