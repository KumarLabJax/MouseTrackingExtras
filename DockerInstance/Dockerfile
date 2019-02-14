FROM wlads/opencv3-contrib-python

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update -y && \
    apt-get install -y git x11vnc wget python python-numpy unzip xvfb firefox openbox geany menu && \
    cd /root && git clone https://github.com/kanaka/noVNC.git && \
    cd noVNC/utils && git clone https://github.com/kanaka/websockify websockify && \
    cd /root && \
    apt-get autoclean && \
    apt-get autoremove && \
    rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y net-tools

ADD startup.sh /startup.sh
RUN chmod 0755 /startup.sh
ADD Watershed_Interactive_Release.cpp /tmp/Watershed_Interactive_Release.cpp
RUN g++ /tmp/Watershed_Interactive_Release.cpp -I/usr/local/include/opencv -I/usr/local/include -L/usr/local/lib -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -o /bin/Labeler
CMD /startup.sh
EXPOSE 6080
