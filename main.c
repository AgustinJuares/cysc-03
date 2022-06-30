#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

/* Struct para los datos de temperatura */
typedef struct {
  float lm35;
  float pote;
} temperature_data_t;

/* Queue para comunicar los dos nucleos */
queue_t queue;

/* Main para el core 1 */
void core1_main() {
  
    while(1) {
        /* Variable para recuperar el dato de la queue */
        temperature_data_t data;
        /* Espera a que esten los datos para recibir */
        queue_remove_blocking(&queue, &data);
        printf("La temperatura es  %.2f\n ", data.lm35 );
        printf("La temperatura deseada es  %.2f\n ", data.pote );
        sleep_ms (500);
    }

}

/* Main para el core 0 */
int main() {
    
    stdio_init_all();
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    
    /* Inicializa la cola para enviar un unico dato */
    queue_init(&queue, sizeof(temperature_data_t), 1);
    /* Inicializa el core 1 */
    multicore_launch_core1(core1_main);
    const float conversion_factor = 3.3f / (1 << 12);
    while(1) {
        /* Variable para enviar los datos */
        temperature_data_t data;
        adc_select_input (0); 
        uint16_t result0 = adc_read(); 
        adc_select_input (1);
        uint16_t result1 = adc_read();
        float volt0= result0 * conversion_factor;
        float volt1= result1 * conversion_factor;
        float temp0= volt0 * 35 / 3.3; 
        float temp1= volt0 / 0.01; 

        data.lm35 = temp0;
        data.pote = temp1;

        /* Cuando los datos estan listos, enviar por la queue */
        queue_add_blocking(&queue, &data);
        sleep_ms(500);
    }
    
}
