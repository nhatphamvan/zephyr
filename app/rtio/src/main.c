#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/drivers/adc.h>


RTIO_DEFINE(rtio, 4, 4);


// Define a callback function for completion
void tx_callback(struct rtio *r, struct rtio_cqe *cqe) {
    printk("TX operation completed with result: %d\n", cqe->result);
}

void my_iodev_submit(struct rtio_iodev_sqe *iodev_sqe) {
    // Example implementation of the submit function
    printk("Submitting SQE to my I/O device\n");
    // Here you would enqueue or start processing the SQE with the hardware
}

int main()
{
    struct rtio_sqe sqe ;

    const struct rtio_iodev_api my_iodev_api = {
        .submit =  my_iodev_submit,
    };

    const struct rtio_iodev my_iodev = {
        .api = &my_iodev_api,
        .data = (void *) NULL,
    };

    rtio_sqe_prep_nop(&sqe, &my_iodev, (void *)NULL );
    printf("Operation: %d\n", sqe.op);

}