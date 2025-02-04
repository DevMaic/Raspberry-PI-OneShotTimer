#include <stdio.h> // Biblioteca padrão do C.
#include "pico/stdlib.h" // Biblioteca padrão do Raspberry Pi Pico para controle de GPIO, temporização e comunicação serial.
#include "pico/time.h"   // Biblioteca para gerenciamento de temporizadores e alarmes.

//Pinos dos leds
const uint BLUE_PIN = 11;
const uint RED_PIN = 12;
const uint GREEN_PIN = 13;

//Pino do push button
const uint BUTTON_PIN = 5;

bool leds_on = false; // Variável global para armazenar o estado dos leds
int led_Atual = 11; // Indica qual led deve ser desligado
int lastTime = 0; // Variável para debounce

// Função de callback para desligar o LED após o tempo programado.
int64_t turn_off_callback(alarm_id_t id, void *user_data) {
    // Desliga um dos leds
    gpio_put(led_Atual, false);
    
    // Atualiza o próximo LED a ser desligado
    if (led_Atual == GREEN_PIN) {
        leds_on = false; // Todos os LEDs foram desligados
        led_Atual = BLUE_PIN; // Volta para o primeiro LED
    } else {
        led_Atual++; // Passa para o próximo LED
        add_alarm_in_ms(3000, turn_off_callback, NULL, false); // Agenda o próximo desligamento
    }

    // Retorna 0 para indicar que o alarme não deve se repetir.
    return 0;
}

//rotina da interrupção
static void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t currentTime = to_us_since_boot(get_absolute_time());
   
    if(currentTime - lastTime > 200000 && !leds_on) { // Debounce
        gpio_put(BLUE_PIN, true);
        gpio_put(RED_PIN, true);
        gpio_put(GREEN_PIN, true);

        //   Define 'leds_on' como true para indicar que há leds acesos e n permitir que
        // apertar o botão novamente acione os leds antes do tempo programado
        leds_on = true;

        add_alarm_in_ms(3000, turn_off_callback, NULL, false); // Agenda o primeiro desligamento
    }
}

int main() {
    // Configura o pino LED_PIN (11) como saída digital.
    gpio_init(BLUE_PIN);
    gpio_set_dir(BLUE_PIN, GPIO_OUT);

    gpio_init(RED_PIN);
    gpio_set_dir(RED_PIN, GPIO_OUT);

    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);

    // Configura o pino BUTTON_PIN (5) como entrada digital.
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, 1, &gpio_irq_handler);
    
    // Loop principal do programa que verifica continuamente o estado do botão.
    while (true) {
        sleep_ms(100); // Aguarda 100ms para evitar consumo excessivo de CPU.
    }

    return 0;
}
