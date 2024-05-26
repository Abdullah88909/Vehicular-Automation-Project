#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "eecs388_lib.h"

#define SERVO_PULSE_MAX 2400    /* 2400 us */
#define SERVO_PULSE_MIN 544     /* 544 us */
#define SERVO_PERIOD    20000   /* 20000 us (20ms) */

void auto_brake(int devid) {
 
    if ('Y' == ser_read(devid) && 'Y' == ser_read(devid)) {         
        uint8_t dist_low = ser_read(devid);
        uint8_t dist_high = ser_read(devid);
        u_int16_t dist = (dist_high << 8) | (dist_low);

        if (dist > 200){
            gpio_write(RED_LED,OFF);
            gpio_write(BLUE_LED,OFF);
            gpio_write(GREEN_LED,ON);
        }
        else if (dist > 100 && dist <= 200){
            gpio_write(BLUE_LED,OFF);
            gpio_write(RED_LED,ON);
            gpio_write(GREEN_LED,ON);
        }
        else if (dist > 60 && dist <= 100){
            gpio_write(BLUE_LED,OFF);
            gpio_write(RED_LED,ON);
            gpio_write(GREEN_LED,OFF);
        }
        else{
            gpio_write(RED_LED,ON);
            gpio_write(GREEN_LED,OFF);
            gpio_write(BLUE_LED,OFF);
            delay(100);
            gpio_write(RED_LED,OFF);
            gpio_write(GREEN_LED,OFF);
            gpio_write(BLUE_LED,OFF);
            delay(100);
        }
        printf("distance %d",dist);
    }
      

    
}

int read_from_pi(int devid){
    int angle = 0; 
    if (ser_isready(1)){
        char buffer[100];
        ser_readline(1, 100, buffer);
        sscanf(buffer, "%d", &angle);

    }

    return angle;
}

void steering(int gpio, int pos) {
    int pulse_width=SERVO_PULSE_MIN + (pos*(SERVO_PULSE_MAX-SERVO_PULSE_MIN)/180);
    gpio_write(gpio,ON);
    delay_usec(pulse_width);
    gpio_write(gpio,OFF);
    delay_usec(SERVO_PERIOD-pulse_width);
 
}

int main() {
    // Initialize UART channels
    ser_setup(0); // UART 0 for LIDAR
    ser_setup(1); // UART 1 for Raspberry Pi
    int pi_to_hifive = 1; // The connection with Pi uses UART 1
    int lidar_to_hifive = 0; // The LIDAR uses UART 0
    
    // Initialize GPIO pins
    gpio_mode(PIN_19, OUTPUT);
    gpio_mode(RED_LED, OUTPUT);
    gpio_mode(BLUE_LED, OUTPUT);
    gpio_mode(GREEN_LED, OUTPUT);

    printf("Setup completed.\n");
    printf("Begin the main loop.\n");

    while (1) {
        // Call auto_brake to handle LIDAR distance data and LED control
        auto_brake(lidar_to_hifive);
        
        // Read angle data from Raspberry Pi
        int angle = read_from_pi(pi_to_hifive);
        printf("\nangle=%d", angle);
        
        // Perform steering based on the angle value
        for (int i = 0; i < 10; i++) {
            if (angle > 0) {
                steering(PIN_19, 180);
            } else {
                steering(PIN_19, 0);
            }
            
            // Optionally, uncomment the line below to use the exact angle values for steering
            //steering(PIN_19, angle);
        }

        // Add a delay if needed to control the loop frequency
        delay(100);
    }
    
    return 0;
}