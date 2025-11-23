#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define TRIG_PIN 2
#define ECHO_PIN 3
#define LED_PUMP 27
#define SERVO_PIN 21

//Defino los umbrales para el tanque
#define Umbral_alto 17   //activar servo
#define Umbral_bajo 27    //activar bomba (LED)

//función para medir distancia con el HC-SR04
uint32_t Mido_distancia() {
    gpio_put(TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(TRIG_PIN, 0);

    while (gpio_get(ECHO_PIN) == 0);
    uint32_t start = time_us_32();

    while (gpio_get(ECHO_PIN) == 1);
    uint32_t end = time_us_32();

    uint32_t pulse_us = end - start;

    return pulse_us / 58; //conversion a cm
}

//Configurar servo en PWM (50Hz)
void init_servo() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);

    pwm_set_clkdiv(slice, 64); 
    pwm_set_wrap(slice, 39062); //50Hz
    pwm_set_enabled(slice, true);
}

//mover servo (0 a 180 grados)
void Angulo_servo(int Angulo) {
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);
    float duty = 1000 + (Angulo * 1000 / 180); //1–2 ms
    uint level = (duty / 20000.0f) * 39062;
    pwm_set_gpio_level(SERVO_PIN, level);
}

int main() {
    stdio_init_all();

    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);

    gpio_init(LED_PUMP);
    gpio_set_dir(LED_PUMP, GPIO_OUT);

    init_servo();
    Angulo_servo(90); //posición inicial cerrada

    while (1) {
      uint32_t distancia = Mido_distancia();
      printf("Nivel: %u cm\n", distancia);

      if (distancia <= Umbral_alto) {
        //Tanque lleno -> desagotar
        Angulo_servo(0);
        gpio_put(LED_PUMP, 0);
      }
      else if (distancia >= Umbral_bajo) {
        //Tanque vacío -> llenar
        gpio_put(LED_PUMP, 1);
        Angulo_servo(90);
      }
      else {
        //Zona estable -> NO llenar ni desagotar
        gpio_put(LED_PUMP, 0);
        Angulo_servo(90);
      }

    sleep_ms(500);
}
}
