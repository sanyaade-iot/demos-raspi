# Copyright 2015 SimpleThings, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
ifneq ($(CANOPY_EDK_ENVSETUP),1)
    $(error You must first run "source envsetup.sh" from the /build directory)
endif

SOURCE_FILES := \
        randomwalk.c

TARGET := $(CANOPY_EDK_BUILD_OUTDIR)/randomwalk

LIB_FLAGS := \
        -L$(CANOPY_EDK_BUILD_DESTDIR)/lib \
        -lcanopy \
        -lred-canopy \
        -lsddl \
        -lwebsockets-canopy \
        -lm \
        -lrt

INCLUDE_FLAGS := \
        -I$(CANOPY_EDK_BUILD_DESTDIR)/include

ifneq ($(CANOPY_CROSS_COMPILE),1)
    LIB_FLAGS += -lcurl
endif

default: all

run: $(TARGET)
	$(TARGET)

dbg: $(TARGET)
	gdb $(TARGET)

clean:
	rm -rf $(CANOPY_EDK_BUILD_OUTDIR)

$(TARGET) : $(SOURCE_FILES)
	mkdir -p $(CANOPY_EDK_BUILD_OUTDIR)
	$(CC) $(INCLUDE_FLAGS) $(SOURCE_FILES) $(LIB_FLAGS) $(CANOPY_CFLAGS) -o $(TARGET)

all: $(TARGET)
