FROM alpine
RUN apk add build-base
RUN apk add make
RUN apk add cmake
RUN apk add bash
RUN apk add gengetopt
RUN apk add nano
RUN apk add libgomp
ADD . /home/SOMCPU
WORKDIR /home/SOMCPU
RUN mkdir build && cd build && cmake .. && make
ENTRYPOINT ["./SOMCPU"]
