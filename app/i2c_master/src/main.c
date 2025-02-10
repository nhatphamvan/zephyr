/*
 * Copyright (c) 2024 Open Pixel Systems
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>

#define TAR_I2C_ADDR    0x50  // Target I2C address

// I2C bus and device references
static const struct device *bus = DEVICE_DT_GET(DT_NODELABEL(i2c2));  // The I2C bus used for communication
static const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));  // The I2C device to send/receive data
static char last_byte;  // Variable to store the last byte received

/*
 * @brief Callback when a write request is received from the master.
 * This callback is triggered when the master requests to write data.
 */
static int target_write_requested_cb(struct i2c_target_config *config)
{
	printk("Target write requested\n");
	return 0;
}

/*
 * @brief Callback when a write is received from the master.
 * This callback is triggered when the master sends data to the target.
 */
static int target_write_received_cb(struct i2c_target_config *config, uint8_t val)
{
	printk("Target write received: 0x%02x\n", val);  // Print the received byte value
	last_byte = val;  // Store the received byte in the last_byte variable
	return 0;
}

/*
 * @brief Callback when a read request is received from the master.
 * This callback is triggered when the master requests to read data from the target.
 */
static int target_read_requested_cb(struct i2c_target_config *config, uint8_t *val)
{
	printk("sample target read request: 0x%02x\n", *val);
	*val = 0x42;  // Provide the value to be read (fixed for this example)
	return 0;
}

/*
 * @brief Callback when a read is processed from the master.
 * This callback is triggered when the master reads data from the target.
 */
static int target_read_processed_cb(struct i2c_target_config *config, uint8_t *val)
{
	printk("Target read processed: 0x%02x\n", *val);  // Print the byte value to be returned
	*val = 0x43;  // Provide a new value for reading (fixed for this example)
	return 0;
}

/*
 * @brief Callback when the master sends a stop condition.
 * This callback is triggered when the master sends a stop signal to end communication.
 */
static int target_stop_cb(struct i2c_target_config *config)
{
	printk("Target stop callback\n");  // Indicate stop condition
	return 0;
}

// Struct to define all the callbacks for the I2C target device
static struct i2c_target_callbacks target_callbacks = {
	.write_requested = target_write_requested_cb,
	.write_received = target_write_received_cb,
	.read_requested = target_read_requested_cb,
	.read_processed = target_read_processed_cb,
	.stop = target_stop_cb,
};

int main(void)
{
	// Target configuration for I2C communication
	struct i2c_target_config target_cfg = {
		.address = TAR_I2C_ADDR,  // Set the I2C address of the target
		.callbacks = &target_callbacks,  // Register the callbacks for the target
	};

	// I2C configuration for the master device (controller mode, standard speed)
	uint32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_CONTROLLER;
	uint8_t write_data[16], read_data[16];  // Buffers for data to write and read

	printk("I2C custom target example\n");

	// Register the target device with the bus
	if (i2c_target_register(bus, &target_cfg) < 0) {
		printk("Failed to register target\n");
		return -1;
	}

	// Check if the I2C device (master) is ready
	if (!device_is_ready(i2c_dev)) {
		printk("I2C: Device is not ready\n");
		return -1;
	}

	// Configure the I2C device
	if (i2c_configure(i2c_dev, i2c_cfg) == 0) {
		printk("I2C configured successfully\n");
	} else {
		printk("Failed to configure I2C\n");
		return -1;
	}

	// Initialize the write_data buffer with values
	for (uint8_t i = 0; i < 16; i++) {
		write_data[i] = i;  // Fill the write data with increasing values
	}

	// Main loop to continually send and receive I2C data
	while (1) {
		/* Send data to the target (master to target communication) */
		if (i2c_write(i2c_dev, write_data, sizeof(write_data), TAR_I2C_ADDR) == 0) {
			printk("I2C write successful\n");
		} else {
			printk("I2C write failed\n");
		}

		k_msleep(1000);  // Wait for 1 second

		/* Read data from the target (master reads from target) */
		if (i2c_read(i2c_dev, read_data, 16, TAR_I2C_ADDR) == 0) {
			printk("I2C read successful\n");
			
			// Print the values read from the target
			printk("Read data: ");
			for (int i = 0; i < 16; i++) {
				printk("0x%02x ", read_data[i]);
			}
			printk("\n");
		} else {
			printk("I2C read failed\n");
		}
		
		k_msleep(5000);  // Wait for 5 seconds before the next communication cycle

		


	}
}
