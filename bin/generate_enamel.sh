# Generate enamel.c and enamel.h from our config.json
# enamel/enamel.py is obtained from Enamel's GitHub repo
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd ${DIR}/../src
python ../../enamel/enamel.py --config js/config.json
cd -
