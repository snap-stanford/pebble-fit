# Prepare the ZIP file for the CloudPebble installation

# Generate enamel.c and enamel.h from our config.json
# enamel/enamel.py is obtained from Enamel's GitHub repo
cd enamel-cloudpebble
python ../enamel/enamel.py --config src/js/config.json
cd ..

# Generate the ZIP file (must contain at least package.json, src, and wscript)
rm pebble-fit.zip
cp enamel-cloudpebble/enamel.c src/
cp enamel-cloudpebble/enamel.h src/
zip -r pebble-fit.zip README.md package.json src/ server/ wscript

# Remove them so that it will not affect the Pebble SDK compilation
rm src/enamel.c
rm src/enamel.h
