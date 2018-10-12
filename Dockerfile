FROM alpine
RUN apk add build-base
RUN apk add make
RUN apk add cmake
RUN apk add bash
ADD . /home/SOMCPU
RUN cd /home/SOMCPU && mkdir build && cd build && cmake .. && make
ENTRYPOINT /home/SOMCPU/SOMCPU -h