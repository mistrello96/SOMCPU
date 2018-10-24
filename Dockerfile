FROM alpine
RUN apk add build-base
RUN apk add make
RUN apk add cmake
RUN apk add bash
RUN apk add gengetopt
RUN apk add less
ADD /src /home/SOMCPU/src
ADD ./CMakeLists /home/SOMCPU
ADD ./LICENSE /home/SOMCPU
ADD ./README.md /home/SOMCPU
WORKDIR /home/SOMCPU
RUN mkdir build && cd build && cmake .. && make
ENTRYPOINT ["./SOMCPU"]