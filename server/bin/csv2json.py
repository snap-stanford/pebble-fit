
# Convert messages file in CSV format to JSON. Perserving the name.
# Example:
# Input file, action_messages.csv:
# am1,Action Message 1
# am2,Take a short walk!
#
# Output format:
# Output file, action_messages.json:
# {
#  "am1": "Action Message 1",
#  "am2": "Take a short walk!"
#}

import sys
import csv
import json

def main(argv):
    if len(argv) != 3:
        print "Usage: " + argv[0] + " file_name.csv destination_directory"
        exit(1)

    filename = argv[1]
    dest = argv[2]
    if filename[-4:] != ".csv":
        print "Error: incorrect input file name."
        exit(1)

    with open(filename, 'r') as csvfile:
        reader = csv.DictReader(csvfile, delimiter=' ', skipinitialspace=True)
        rows = list(reader)
    
    with open(dest+"/"+filename[:-4]+".json", 'w') as jsonfile:
        json.dump(rows, jsonfile)

if __name__ == "__main__":
    main(sys.argv)
