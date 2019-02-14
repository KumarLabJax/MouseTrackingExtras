labeling-docker
============

## Building the image 

Download the Dockerfile and startup.sh from this folder.

Download the [Watershed](../Watershed_Labeler) source file (Watershed_Interactive_Release.cpp).


```
cd <folder_with_DockerFile>
docker build -t labeling-docker .
```

## Running the program

Startup the docker:
```
docker run -td -p 6080:6080 -v /host/directory:/tmp/LinkedFolder labeling-docker
```

Once it is running, connect to it through a web browser by seeking to the following address:
localhost:6080

Right click on the black box (the window into the machine) and select "terminal emulator" to start a terminal.

From there change into the directory where your data is located (cd /tmp/LinkedFolder).
The labeling software has been compiled inside the docker and named "Labeler".

To run it, execute the following: 
```
Labeler -fn=<Movie.avi> -nf=<number_of_frames_to_label> -rm=True
```

## Cleaning up of docker images

If you use docker for other items, these commands will remove all other containers/images. Proceed with caution.

Remove currently not-in-use containers : 
```
sudo docker rm `sudo docker ps --no-trunc -aq`
```

Show any existing images: 
```
sudo docker images
```

Remove images : 
```
sudo docker rmi <list of image IDs>
```

## Credits

* [NoVNC](http://kanaka.github.io/noVNC/)
* [Original docker-novnc project](https://github.com/paimpozhil/docker-novnc)
* [Minimal docker-novnc project](https://github.com/zerodivide1/docker-novnc)
* [Opencv](https://github.com/opencv/opencv)
* [Opencv Docker project](https://hub.docker.com/r/wlads/opencv3-contrib-python)
