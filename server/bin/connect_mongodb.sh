
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

args=" ds111748.mlab.com:11748/heroku_0cbvznft -u pebble-fit -p"

hash mongo >/dev/null 2>&1
if [[ $? -eq 0 ]]; then
  # Use the system installation of mongo
  cmd="mongo"
elif [[ -x ${DIR}/mongo ]]; then 
  # Use the local installation of mongo
  cmd="${DIR}/mongo"
else
  echo "Error: could not find mongo!"
  exit 1
fi

cmd+=${args}

echo "eval ${cmd}"
eval ${cmd}
