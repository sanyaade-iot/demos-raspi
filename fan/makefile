TARGETS := fan 
.PHONY: default
default: $(TARGETS)

SOURCE_FILES = fan.c

INCLUDE_DIRS = \
    -I../../../libcanopy/include \
    -I../../../libsddl/include \
    -I../../../3rdparty/libred/include

fan: $(SOURCE_FILES)
	gcc $(SOURCE_FILES) $(INCLUDE_DIRS) -std=gnu99 -g -lred-canopy -lcanopy -lsddl -lcurl -lwebsockets-canopy -lrt -o $@
