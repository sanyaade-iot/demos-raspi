TARGETS := fan 
.PHONY: default
default: $(TARGETS)

SOURCE_FILES := fan.c \
    ../3rdparty/pi_dht/common_dht_read.c \
    ../3rdparty/pi_dht/Raspberry_Pi/pi_dht_read.c \
    ../3rdparty/pi_dht/Raspberry_Pi/pi_mmio.c

INCLUDE_DIRS := \
    -I../../../libcanopy/include \
    -I../../../libsddl/include \
    -I../../../3rdparty/libred/include \
    -I../3rdparty/pi_dht/Raspberry_Pi

LIB_FLAGS := \
    -L../../../build/_out/lib \
    -lcanopy \
    -lred-canopy \
    -lsddl \
    -lcurl \
    -lwebsockets-canopy \
    -lrt \
    -lm

fan: $(SOURCE_FILES)
	gcc $(SOURCE_FILES) $(INCLUDE_DIRS) -std=gnu99 -g $(LIB_FLAGS) -o $@
