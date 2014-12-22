#include <canopy.h>
#include <stdio.h>

static void SetFanSpeed(int8_t speed)
{
    printf("Fan speed set to %d", speed);
}

static bool ReadSensor(float *temperature, float *humidity)
{
    *temperature = 40.1f;
    *humidity = 23.3f;
}


static int OnFanSpeedChange(CanopyContext ctx, const char *varName, void *userData)
{
    int8_t fanSpeed;
    CanopyResultEnum result;

    result = canopy_var_get_int8(ctx, "fan_speed", &fanSpeed);
    if (!ctx) {
        fprintf(stderr, "Error reading fan_speed");
        return -1;
    }

    SetFanSpeed(fanSpeed);
    return 0;
}


int main(void)
{
    CanopyContext ctx;
    CanopyResultEnum result;
    long reportTimer = 0;

    ctx = canopy_init_context();
    if (!ctx) {
        fprintf(stderr, "Error initializing context");
        return -1;
    }

    result = canopy_set_opt(ctx,
        CANOPY_DEVICE_UUID, "e6968460-f010-48ef-8e69-835543843b32",
        CANOPY_DEVICE_SECRET_KEY, "/pMdwTzEA3+d66qo3MZQ2bWjsYGXAHAb",
        CANOPY_CLOUD_HOST, "sandbox.canopy.link"
    );

    if (result != CANOPY_SUCCESS){
        fprintf(stderr, "Failed to configure context");
        return -1;
    }

    result = canopy_var_init("out float32 temperature");
    if (result != CANOPY_SUCCESS){
        fprintf(stderr, "Failed to init cloudvar 'temperature'");
        return -1;
    }
    result = canopy_var_init("out float32 humidity");
    if (result != CANOPY_SUCCESS){
        fprintf(stderr, "Failed to init cloudvar 'humidity'");
        return -1;
    }
    result = canopy_var_init("in int8 fan_speed");
    if (result != CANOPY_SUCCESS){
        fprintf(stderr, "Failed to init cloudvar 'fan_speed'");
        return -1;
    }

    result = canopy_var_on_change(ctx, "fan_speed", OnFanSpeedChange, NULL);
    if (result != CANOPY_SUCCESS) {
        fprintf(stderr, "Error setting up fan_speed callback");
        return -1;
    }

    while (1) {
        canopy_sync_blocking(ctx, 10*CANOPY_SECONDS);

        if (canopy_once_every(&reportTimer, 60*CANOPY_SECONDS)) {
            float temperature, humidity;
            bool sensorOk;
            sensorOk = ReadSensor(&temperature, &humidity);
            if (sensorOk) {
                result =canopy_var_set_float32(ctx, "temperature", temperature);
                if (result != CANOPY_SUCCESS) {
                    fprintf(stderr, "Failed to set cloudvar 'temperature'");
                    return -1;
                }

                result = canopy_var_set_float32(ctx, "humidity", humidity);
                if (result != CANOPY_SUCCESS) {
                    fprintf(stderr, "Failed to set cloudvar 'humidity'");
                    return -1;
                }
            }
        }
    }
}
