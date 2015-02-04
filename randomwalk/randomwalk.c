// Copyright 2015 SimpleThings, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <canopy.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

float rand_unit() {
    return (float)rand()/(float)RAND_MAX;
}

float lerp(float a, float b, float t) {
    return t*a + (1-t)*b;
}

#define MIN_LAT_START 40.694045f
#define MAX_LAT_START 40.801466f
#define MIN_LNG_START -111.936636f
#define MAX_LNG_START -111.80892f
int main(int argc, const char *argv[])
{
    CanopyContext canopy;
    CanopyResultEnum result;
    int i;
    float lat, lng;
    float offsX, offsY;

    int seed = time(NULL);
    srand(seed);
    offsX = lerp(-0.0015f, 0.0015f, rand_unit());
    offsY = lerp(-0.0015f, 0.0015f, rand_unit());

    if (!getenv("CANOPY_CLOUD_SERVER"))
    {
        fprintf(stderr, "You must set CANOPY_CLOUD_SERVER env var\n");
        return -1;
    }

    if (!getenv("CANOPY_DEVICE_UUID"))
    {
        fprintf(stderr, "You must set CANOPY_DEVICE_UUID env var\n");
        return -1;
    }

    canopy = canopy_init_context();
    assert(canopy);

    result = canopy_set_opt(canopy,
        CANOPY_CLOUD_SERVER, getenv("CANOPY_CLOUD_SERVER"),
        CANOPY_DEVICE_UUID, getenv("CANOPY_DEVICE_UUID"),
        CANOPY_SYNC_BLOCKING, true,
        CANOPY_SYNC_TIMEOUT_MS, 10000,
        CANOPY_SKIP_SSL_CERT_CHECK, true,
        CANOPY_VAR_SEND_PROTOCOL, CANOPY_PROTOCOL_WSS,
        CANOPY_VAR_RECV_PROTOCOL, CANOPY_PROTOCOL_WSS
    );
    assert(result == CANOPY_SUCCESS);

    result = canopy_var_init(canopy, "out float32 latitude");
    assert(result == CANOPY_SUCCESS);
    result = canopy_var_init(canopy, "out float32 longitude");
    assert(result == CANOPY_SUCCESS);

    // Pick start position:
    lat = lerp(MIN_LAT_START, MAX_LAT_START, rand_unit());
    lng = lerp(MIN_LNG_START, MAX_LNG_START, rand_unit());

    for (i = 0; i < 400; i++) {
        result = canopy_var_set_float32(canopy, "latitude", lat);
        assert(result == CANOPY_SUCCESS);

        result = canopy_var_set_float32(canopy, "longitude", lng);
        assert(result == CANOPY_SUCCESS);

        result = canopy_sync(canopy, NULL);
        assert(result == CANOPY_SUCCESS);

        lng += lerp(-0.002f, 0.002f, rand_unit()) + offsX;
        lat += lerp(-0.0020f, 0.002f, rand_unit()) + offsY;
    }



    result = canopy_shutdown_context(canopy);
    assert(result == CANOPY_SUCCESS);

    return 0;
}
