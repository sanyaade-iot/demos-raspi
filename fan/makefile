TARGETS := fan 
.PHONY: default
default: $(TARGETS)

SOURCE_FILES := fan.c

INCLUDE_DIRS := \
    -I../../../libcanopy/include \
    -I../../../libsddl/include \
    -I../../../3rdparty/libred/include

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
