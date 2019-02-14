import numpy as np
from ReadNPYAppend import read_data
import argparse
import os
import sys

def main(argv):
	parser = argparse.ArgumentParser(description='Exports numpy data (.npy) as a csv (.csv)')
	parser.add_argument('numpy_file', help='Name of numpy file to export')

	args = parser.parse_args()

	# Check if the numpy file exists
	if not os.path.exists(args.numpy_file):
		sys.exit('File: ' + args.numpy_file + ' not found.')

	csv_filename = os.path.splitext(args.numpy_file)[0] + '.csv'

	# Read in the data to export
	data = read_data(args.numpy_file)

	# Export the file
	np.savetxt(csv_filename, data, delimiter=',')

if __name__ == '__main__':
	main(sys.argv[1:])

