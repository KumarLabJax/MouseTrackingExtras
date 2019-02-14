watershed_labeler
============

## Algorithm Description

The software uses [opencv's watershed algorithm](https://docs.opencv.org/2.4/modules/imgproc/doc/miscellaneous_transformations.html?highlight=watershed#cv2.watershed) to segment the foreground and background.

Once the foreground and background are segmented, morphological filtering is applied to smooth out the mask edges prior to fitting the ellipse.

An ellipse is fit using [opencv's fitEllipse function](http://docs.opencv.org/2.4/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html?#fitellipse).

Finally, direction is selected for the ellipse using closest selected quantile.

## Compatability

This code has been compiled in both [OpenCV 2.4.13](https://docs.opencv.org/2.4/index.html) and [OpenCV 3.1.0](https://docs.opencv.org/3.1.0/).

While primarily developed for use on Ubuntu 16.04 LTS, the code has also been successfully compiled without changes on Windows 7 and MacOSX. The following instructions are only specific to Ubuntu 16.04.

## Compiling

First, ensure that OpenCV has been correctly compiled from source on your system. This is not a straight forward task. Please refer to OpenCV's installation guides for assistance. Here are direct links for installation assistance: [OpenCV 2.4](https://docs.opencv.org/2.4/doc/tutorials/introduction/linux_install/linux_install.html#linux-installation) and [OpenCV 3.1](https://docs.opencv.org/3.1.0/d7/d9f/tutorial_linux_install.html).

Compiling on ubuntu using pkg-config:
```
g++ Watershed_Interactive_Release.cpp `pkg-config --cflags --libs opencv` -o Labeler
```

Full compiling command:  
```
g++ Watershed_Interactive_Release.cpp \  
-I/usr/local/include/opencv -I/usr/local/include \  
-L/usr/local/lib -lopencv_highgui -lopencv_videoio \  
-lopencv_imgcodecs -lopencv_imgproc -lopencv_core -o Labeler
```

## Program Controls

### Startup Options

fn = movie filename  
sf = start frame  
nf = number of frames to label before program closes  
rm = random mode (randomizes frame number while seeking)  
ps = point size for painting (default 3 pixels)

Example:
```
./Labeler -fn=Movie.avi -sf=10000 -nf=25 -rm=true -ps=1
```

### Labeling

Left mouse click: label point as foreground  
Right mouse click: label point as background  
hold mouse click: paint (foreground or background)  
w: apply the watershed algorithm  
r: remove labels on current frame current frame  
+: increase gamma (enhances contrast), resets on w and r keys  
q: quit program (doesn't save last frame)  

<strong>(Not supported on all versions)</strong>  
mouse wheel: zoom in and out  
hold left click while zoomed: paint 1 pixel + pan image

### Color Definitions

<span style="color:#00FF00">Green</span>: Newly labeled Foreground  
<span style="color:#FF0000">Red</span>: Newly labeled Background  
<span style="color:#00FFFF">Cyan</span>: Predicted ellipse fit  
<span style="color:#FF00FF">Magenta</span>: Predicted Segmentation Boundary  
<span style="color:#AA0044">Dark Red</span>: Old Foreground label  
<span style="color:#FFFF00">Yellow</span>: Old Background label

### Saving Frames

Angle is selected via the ellipse-fit and selecting the direction (within 180deg). The following 4 keys select the correct closest angle based on this ellipse-fit: 

1. Up Arrow Key
2. Down Arrow Key
3. Left Arrow Key
4. Right Arrow Key

When saving frames, the terminal will provide output stating how many frames have currently been saved this session as well as the frame number that was just saved (or skipped). This notifies you that 3 files were saved: The reference image, the segmentation image, and the ellipse fit text file.

The directory structure of the saved files are as follows:  
Reference Frame = `<movie_name>/Ref/<movie_name>_<frame_number>.png`  
Segmentation Frame = `<movie_name>/Seg/<movie_name>_<frame_number>.png`  
Ellipse Fit = `<movie_name>/Ell/<movie_name>_<frame_number>.txt`


### Saved File Formats

The reference image is saved as an 8-bit color png. This image is a copy of the frame read from the video.  
The segmentation image is saved as an 8-bit color png. This image contains only 2 color values: 0 (background) and 127 (foreground). The comparison in the neural network is only 0 and non-zero.  
The ellipse-fit file is saved as plaintext.

#### Ellipse Fit File Details
The ellipse-fit file contains 5 values separated by tabs. The values are in the following order:   

1. center_x (in pixels)  
2. center_y (in pixels)  
3. minor_axis_length (in pixels, half the width of the bounding rectangle)  
4. major_axis_length (in pixels, half the height of the bounding rectangle)  
5. angle (in degrees, zero pointing down with positive angles going counter-clockwise)  

These are copies from [opencv's fitEllipse function](http://docs.opencv.org/2.4/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html?#fitellipse) with corrections applied to the angle to make it 0-360deg (instead of 0-180).

#### Merging Multiple Video Labels into One Dataset

While the Ref/Seg/Ell folders reside within the movie_name folder, they can be merged without additional renaming (as long as the movie names are different).

## Known Issues

If the foreground and background mask labels touch eachother, the watershed algorithm will break down. Use the hotkey "r" to re-label the frame when this occurs.

## Credits

* [Opencv](https://github.com/opencv/opencv)
