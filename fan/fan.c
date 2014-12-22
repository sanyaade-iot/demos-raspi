#include <canopy.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "pi_dht_read.h"

#define SENSOR_PIN 2

/* returns true on success */
static bool set_gpio(int pin, int value)
{
    FILE *fp;
    char buf[1024];

    snprintf(buf, 1024, "/sys/class/gpio/gpio%d/value", pin);
    fp = fopen(buf, "w");
    if (!fp)
    {
        fprintf(stderr, "Failed to set value of gpio%d\n", pin);
        return false;
    }

    printf("writing %d to %s\n", value, buf);
    fprintf(fp, "%d", value);
    fclose(fp);

    return true;
}

static bool set_gpio_direction(int pin, const char *direction) 
{
    FILE *fp;
    char buf[1024];

    /* set GPIO pin direction */
    snprintf(
        buf, 
        1024, 
        "/sys/class/gpio/gpio%d/direction", 
        pin);
    fp = fopen(buf, "w");
    if (!fp)
    {
        fprintf(stderr, "Failed to set direction of gpio%d\n", pin);
        goto cleanup;
    }
    printf("writing %s to %s\n", direction, buf);
    fprintf(fp, "%s", direction);
    fclose(fp);
    return true;
cleanup:
    return false;
}

static bool init_gpio(int pin)
{
    FILE *fp;
    char buf[1024];

    /* export GPIO pin */
    fp = fopen("/sys/class/gpio/export", "w");
    if (!fp)
    {
        fprintf(stderr, "Failed to export pin %d\n", pin);
        goto cleanup;
    }
    printf("writing %d to /sys/class/gpio/export\n", pin);
    fprintf(fp, "%d", pin);
    fclose(fp);

    /* wait for ready */
    snprintf(
        buf, 
        1024, 
        "/sys/class/gpio/gpio%d/direction", 
        pin);
    do
    {
        fp = fopen(buf, "w");
    } while (!fp && errno == EACCES);
    if (!fp)
    {
        fprintf(stderr, "GPIO %d not ready\n", pin);
        return false;
    }
    fclose(fp);
    fprintf(stderr, "GPIO %d ready\n", pin);

    return true;
cleanup:
    return false;
}

static void set_fan_speed(int8_t speed)
{
    static int8_t last_speed=-1;

    if (speed == last_speed)
        return;

    printf("Setting fan speed to %d\n", speed);
    set_gpio(23, 1);
    set_gpio(15, 1);
    set_gpio(18, 1);
    switch (speed)
    {
        case 1:
            /* slowest */
            set_gpio(18, 0);
            break;
        case 2:
            set_gpio(23, 0);
            break;
        case 3:
            /* fastest */
            set_gpio(15, 0);
            break;
    }
    last_speed = speed;
}

bool init_fan_pins()
{
    printf("starting...\n");
    init_gpio(23);
    set_gpio_direction(23, "in");
    init_gpio(15);
    set_gpio_direction(15, "in");
    init_gpio(18);
    set_gpio_direction(18, "in");

    set_gpio_direction(23, "out");
    set_gpio(23, 1);
    set_gpio_direction(15, "out");
    set_gpio(15, 1);
    set_gpio_direction(18, "out");
    set_gpio(18, 1);

    init_gpio(25);
    set_gpio_direction(25, "out");
    set_gpio(25, 0);
    return true;
}

static bool read_sensors(float *temperature, float *humidity)
{
    int result;
    printf("reading...\n");
    result = pi_dht_read(DHT22, SENSOR_PIN, temperature, humidity);
    if (result != DHT_SUCCESS)
    {
        // TODO: cancel report
        printf("Error reading DHT: %d\n", result);
        return false;
    }
    return true;
}


static int on_fan_speed_change(CanopyContext ctx, const char *varName, void *userData)
{
    int8_t fanSpeed;
    CanopyResultEnum result;

    result = canopy_var_get_int8(ctx, "fan_speed", &fanSpeed);
    if (!ctx) {
        fprintf(stderr, "Error reading fan_speed\n");
        return -1;
    }

    set_fan_speed(fanSpeed);
    return 0;
}


int main(void)
{
    CanopyContext ctx;
    CanopyResultEnum result;
    long reportTimer = 0;

    result = canopy_set_global_opt(
        CANOPY_LOG_LEVEL, 0,
        CANOPY_LOG_PAYLOADS, true);

    ctx = canopy_init_context();
    if (!ctx) {
        fprintf(stderr, "Error initializing context\n");
        return -1;
    }

    result = canopy_set_opt(ctx,
        CANOPY_DEVICE_UUID, "e6968460-f010-48ef-8e69-835543843b32",
        CANOPY_DEVICE_SECRET_KEY, "/pMdwTzEA3+d66qo3MZQ2bWjsYGXAHAb",
        CANOPY_CLOUD_SERVER, "sandbox.canopy.link"
    );

    if (result != CANOPY_SUCCESS){
        fprintf(stderr, "Failed to configure context\n");
        return -1;
    }

    result = canopy_var_init(ctx, "out float32 temperature");
    if (result != CANOPY_SUCCESS){
        fprintf(stderr, "Failed to init cloudvar 'temperature'\n");
        return -1;
    }
    result = canopy_var_init(ctx, "out float32 humidity");
    if (result != CANOPY_SUCCESS){
        fprintf(stderr, "Failed to init cloudvar 'humidity'\n");
        return -1;
    }
    result = canopy_var_init(ctx, "inout int8 fan_speed");
    if (result != CANOPY_SUCCESS){
        fprintf(stderr, "Failed to init cloudvar 'fan_speed'\n");
        return -1;
    }

    /*result = canopy_var_on_change(ctx, "fan_speed", on_fan_speed_change, NULL);
    if (result != CANOPY_SUCCESS) {
        fprintf(stderr, "Error setting up fan_speed callback\n");
        return -1;
    }*/

    // turn fan off
    init_fan_pins();
    result = canopy_var_set_int8(ctx, "fan_speed", 0);
    if (!ctx) {
        fprintf(stderr, "Error setting fan_speed\n");
        return -1;
    }

    while (1) {
        int8_t speed;
        canopy_sync_blocking(ctx, 10*CANOPY_SECONDS);

        result = canopy_var_get_int8(ctx, "fan_speed", &speed);
        if (result != CANOPY_SUCCESS) {
            fprintf(stderr, "Error reading fan_speed\n");
            return -1;
        }
        set_fan_speed(speed);

        if (canopy_once_every(&reportTimer, 5*CANOPY_SECONDS)) {
            printf("reading sensors...\n");
            float temperature, humidity;
            bool sensorOk;
            sensorOk = read_sensors(&temperature, &humidity);
            if (sensorOk) {
                printf("Temperature: %f    Humidity: %f\n", temperature, humidity);
                result = canopy_var_set_float32(ctx, "temperature", temperature);
                if (result != CANOPY_SUCCESS) {
                    fprintf(stderr, "Failed to set cloudvar 'temperature'\n");
                    return -1;
                }

                result = canopy_var_set_float32(ctx, "humidity", humidity);
                if (result != CANOPY_SUCCESS) {
                    fprintf(stderr, "Failed to set cloudvar 'humidity'\n");
                    return -1;
                }
            }
        }
    }
}
