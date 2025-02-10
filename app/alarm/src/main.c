/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * This sample demonstrates how to use the counter driver to set an alarm
 * that triggers an interrupt at a specified time. The alarm interval is
 * doubled on each trigger.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/sys/printk.h>

#define DELAY_US 2000000 // Initial delay in microseconds
#define ALARM_CHANNEL_ID 0

#define COUNTER2 DT_INST(0, st_stm32_counter)

/**
 * @brief Counter alarm interrupt handler.
 *
 * This function is called when the counter alarm triggers.
 * It doubles the alarm interval and sets a new alarm.
 *
 * @param counter_dev Pointer to the counter device.
 * @param chan_id Channel ID of the alarm.
 * @param ticks Counter ticks at the time of the alarm.
 * @param user_data User-defined data passed to the callback.
 */
static void counter_alarm_handler(const struct device *counter_dev,
                                 uint8_t chan_id, uint32_t ticks,
                                 void *user_data)
{
    struct counter_alarm_cfg *config = user_data;
    uint32_t now_ticks;
    uint64_t now_usec;
    int now_sec;
    int err;

    // Get current counter value
    err = counter_get_value(counter_dev, &now_ticks);
    if (err) {
        printk("Failed to read counter value (err %d)\n", err);
        return;
    }

    // Adjust for counting direction
    if (!counter_is_counting_up(counter_dev)) {
        now_ticks = counter_get_top_value(counter_dev) - now_ticks;
    }

    // Convert ticks to microseconds
    now_usec = counter_ticks_to_us(counter_dev, now_ticks);
    now_sec = (int)(now_usec / USEC_PER_SEC);

    printk("!!! Alarm !!!\n");
    printk("Current time: %d seconds\n", now_sec);

    // Double the alarm interval
    config->ticks *= 2;

    // Calculate and print the next alarm time
    printk("Next alarm in %u seconds (%u ticks)\n",
           (uint32_t)(counter_ticks_to_us(counter_dev, config->ticks) / USEC_PER_SEC),
           config->ticks);

    // Set the new alarm
    err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID, user_data);
    if (err != 0) {
        printk("Failed to set alarm: %d\n", err);
    }
}

int main(void)
{
    const struct device *const counter_dev = DEVICE_DT_GET(COUNTER2);
    struct counter_alarm_cfg alarm_cfg = {
        .flags = COUNTER_CONFIG_INFO_COUNT_UP,
        .ticks = counter_us_to_ticks(counter_dev, DELAY_US),
        .callback = counter_alarm_handler,
        .user_data = &alarm_cfg,
    };
    int err;

    printk("Counter alarm sample\n\n");

    if (!device_is_ready(counter_dev)) {
        printk("Counter device not ready.\n");
        return 0;
    }

    counter_start(counter_dev);

    err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID, &alarm_cfg);
    if (err) {
        printk("Failed to set initial alarm: %d\n", err);
        return 0;
    }

    printk("Initial alarm set for %u seconds (%u ticks)\n",
           (uint32_t)(counter_ticks_to_us(counter_dev, alarm_cfg.ticks) / USEC_PER_SEC),
           alarm_cfg.ticks);

    while (1) {
        k_sleep(K_FOREVER); // Suspend execution indefinitely
    }

    return 0;
}