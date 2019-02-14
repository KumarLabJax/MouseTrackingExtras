NPYReader
============

## Function

The neural network code exports the numpy data in append mode (not supported by numpy's base code) to allow for very large files to be output without memory issues. This is the format produced by our lab's [neural network tracking code](https://github.com/KumarLabJax/MouseTracking).

ReadNPYAppend.py contains a function definition to read the data into a numpy array.

ExportCSVExample.py contains example code for usage of this function as well as direct exporting this data to the CSV format.

## Compatability

This software was tested using python 2.7.12 and 3.6.2. Numpy version 1.13.1 was used in both python versions.

While these were the versions tested, most other versions should function just fine.

## Example Usage

```
python ExportCSVExample.py my-data.npy
```

The resulting command will read the data in my-data.npy and export it into my-data.csv.
