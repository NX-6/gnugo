## https://hub.docker.com/r/emscripten/emsdk
## https://github.com/emscripten-core/emsdk/blob/main/docker/Dockerfile
FROM emscripten/emsdk:2.0.31

## patterns/*.{c, db}

COPY /gnugo-3.8 /gg-gcc
WORKDIR         /gg-gcc
RUN ./configure --without-readline --without-curses
COPY /config.h  /gg-gcc/config.h
RUN make


## gnugo.{js, wasm}

COPY /gnugo-3.8 /gg-emcc
WORKDIR         /gg-emcc
COPY /api.c     /gg-emcc/api.c
COPY /config.h  /gg-emcc/config.h

RUN mkdir /out

RUN emcc -o /out/gnugo.js \
         -O3 \
         -s MODULARIZE=1 \
         -s EXPORT_NAME=GnuGo \
         -s ASSERTIONS=1 \
         -s ENVIRONMENT=web,worker \
         -s DEMANGLE_SUPPORT=1 \
         -s INITIAL_MEMORY=33554432 \
         -s ALLOW_MEMORY_GROWTH=1 \
         --use-preload-plugins \
         -DHAVE_CONFIG_H \
         -I /gg-emcc \
         -I /gg-emcc/utils \
         -I /gg-emcc/patterns \
         -I /gg-emcc/sgf \
         -I /gg-emcc/engine \
         -I /gg-emcc/interface \
         /gg-emcc/api.c \
         /gg-emcc/engine/*.c \
         /gg-emcc/utils/getopt.c \
         /gg-emcc/utils/getopt1.c \
         /gg-emcc/utils/random.c \
         /gg-emcc/utils/gg_utils.c \
         /gg-emcc/sgf/sgf_utils.c \
         /gg-emcc/sgf/sgfnode.c \
         /gg-emcc/sgf/sgftree.c \
         /gg-gcc/patterns/connections.c \
         /gg-gcc/patterns/helpers.c \
         /gg-gcc/patterns/transform.c \
         /gg-gcc/patterns/conn.c \
         /gg-gcc/patterns/patterns.c \
         /gg-gcc/patterns/apatterns.c \
         /gg-gcc/patterns/dpatterns.c \
         /gg-gcc/patterns/eyes.c \
         /gg-gcc/patterns/influence.c \
         /gg-gcc/patterns/barriers.c \
         /gg-gcc/patterns/endgame.c \
         /gg-gcc/patterns/aa_attackpat.c \
         /gg-gcc/patterns/owl_attackpat.c \
         /gg-gcc/patterns/owl_vital_apat.c \
         /gg-gcc/patterns/owl_defendpat.c \
         /gg-gcc/patterns/fusekipat.c \
         /gg-gcc/patterns/fuseki9.c \
         /gg-gcc/patterns/fuseki13.c \
         /gg-gcc/patterns/fuseki19.c \
         /gg-gcc/patterns/josekidb.c \
         /gg-gcc/patterns/handipat.c \
         /gg-gcc/patterns/oraclepat.c \
         /gg-gcc/patterns/mcpat.c
