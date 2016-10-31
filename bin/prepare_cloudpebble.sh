# Prepare the ZIP file for the CloudPebble installation

# Generate enamel.c and enamel.h from our config.json
# enamel/enamel.py is obtained from Enamel's GitHub repo
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd ${DIR}/../src
python ../../enamel/enamel.py --config js/config.json
cd ..

# Generate the ZIP file (must contain at least package.json, src, and wscript)
rm pebble-fit.zip
zip -r pebble-fit.zip README.md package.json src/ server/ wscript

# Remove them so that it will not affect the Pebble SDK compilation
rm src/enamel.c
rm src/enamel.h
