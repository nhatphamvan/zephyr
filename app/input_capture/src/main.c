#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define BUTTON_NODE DT_ALIAS(sw0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(LED0_NODE, gpios);


// Define PWM period and pulse width for output (channel 1)
#define PWM_PERIOD_USEC 1000000000   // Period in microseconds (1ms)
#define PWM_PULSE_USEC 500000000     // Pulse width in microseconds (50% duty cycle)


// Callback for PWM capture on channel 2
void pwm_capture_callback(const struct device *dev, 
                          uint32_t channel, 
                          uint32_t period_cycles, 
                          uint32_t pulse_cycles, 
                          int status, 
                          void *user_data) {
    if (status == 0) {
        printf("PWM capture successful on channel %d\n", channel);
        printf("Captured Period (cycles): %u\n", period_cycles);
        printf("Captured Pulse Width (cycles): %u\n", pulse_cycles);
        gpio_pin_toggle_dt(&led);
        //Get and print the current LED state
        int led_state = gpio_pin_get_dt(&led);
        printf("LED state: %s\n", led_state ? "ON" : "OFF");

    } else {
        printf("PWM capture error: %d", status);
    }
}

void main(void) {

	int ret;

	if (!gpio_is_ready_dt(&led)) {
        printf("LED GPIO not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

    if (!gpio_is_ready_dt(&button)) {
		return 0;
	}
	// ret = gpio_pin_configure_dt(&button, GPIO_INPUT );
	// if (ret < 0) {
	// 	return 0;
	// }

    //Loi giua chan pa5 va pc13

    // Get the PWM device
    const struct device *pwm5_dev = DEVICE_DT_GET(DT_NODELABEL(pwm5));
    const struct device *pwm2_dev = DEVICE_DT_GET(DT_NODELABEL(pwm2));


    if (!device_is_ready(pwm5_dev)) {
        printf("PWM device not ready\n");
        return;
    }

    if (!device_is_ready(pwm2_dev)) {
        printf("PWM device not ready\n");
        return;
    }

    printf("Configuring PWM Output on Channel 1...\n");
    // Configure Channel 1 as PWM output
    ret = pwm_set(
        pwm2_dev,                  // PWM device
        1,                        // Channel 1
        PWM_PERIOD_USEC,          // Period in microseconds
        PWM_PULSE_USEC,           // Pulse width in microseconds
        PWM_POLARITY_NORMAL                         // Flags (e.g., polarity)
    );

    if (ret < 0) {
        printf("Failed to configure PWM output on channel 1: %d", ret);
        return;
    }
    printf("PWM Output started on Channel 1 with %u us period and %u us pulse width\n", 
            PWM_PERIOD_USEC, PWM_PULSE_USEC);

    printf("Configuring PWM Input (Capture) on Channel 2...\n");
    // Configure Channel 2 for PWM capture
    ret = pwm_configure_capture(
        pwm5_dev,                              // PWM device
        2,                                    // Channel 2
        PWM_CAPTURE_TYPE_PERIOD | PWM_CAPTURE_TYPE_PULSE | PWM_CAPTURE_MODE_CONTINUOUS, // Capture both period and pulse width
        pwm_capture_callback,              // Callback function
        NULL                                  // No user data
    );

    if (ret < 0) {
        printf("Failed to configure PWM capture on channel 2: %d\n", ret);
        return;
    }

    // Enable PWM capture on Channel 2
    ret = pwm_enable_capture(pwm5_dev, 2);
    if (ret < 0) {
        printf("Failed to enable PWM capture on channel 2: %d\n", ret);
        return;
    }

    printf("PWM Capture started on Channel 2\n");

    // Keep the application running to allow continuous capture
    while (1) {


        k_sleep(K_MSEC(1000));
    }
}
