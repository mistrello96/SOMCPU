FROM alpine
RUN apk add build-base
RUN apk add make
RUN apk add cmake
RUN apk add bash
RUN apk add gengetopt
RUN apk add less
ADD . /home/SOMCPU
WORKDIR /home/SOMCPU
RUN mkdir build && cd build && cmake .. && make
CMD ["./SOMCPU"]