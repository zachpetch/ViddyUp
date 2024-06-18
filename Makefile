CC=gcc

SRC_DIR = src
OBJ_DIR = obj

FFMPEG_LIBS=	libavdevice		\
				libavformat		\
				libavfilter		\
				libavcodec		\
				libswresample	\
				libswscale		\
				libavutil		\

CFLAGS += -Wall -g
# `-I/opt/homebrew/Cellar/ffmpeg/7.0.1/include`
CFLAGS := $(shell pkg-config --cflags $(FFMPEG_LIBS)) $(CFLAGS)
LDLIBS := $(shell pkg-config --libs $(FFMPEG_LIBS)) $(LDLIBS)
BIN=ViddyUp
SOURCES=$(wildcard $(SRC_DIR)/*.c)
OBJECTS=$(OBJ_DIR)/$(addsuffix .o,$(BIN))
avcodec:		LDLIBS += -lm
encode_audio:	LDLIBS += -lm
mux:			LDLIBS += -lm
resample_audio:	LDLIBS += -lm

.phony: all clean

#test:
#	echo $(CFLAGS)
#	echo $(LDLIBS)

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) $^ $(LDLIBS) -o $@

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) $(BIN) $(OBJECTS)
