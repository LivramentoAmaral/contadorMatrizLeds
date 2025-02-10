#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

#define LED_PIN 7       // GPIO para os LEDs WS2812
#define BTN_START 5     // Botão de início
#define BTN_STOP 6      // Botão de parada
#define NUM_PIXELS 25   // Matriz 5x5 de LEDs

volatile bool counting = false;  // Variável para controlar a contagem

// Inicializa o PIO para controlar os LEDs WS2812
void init_ws2812(PIO pio, int sm) {
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_PIN, 800000, false);
}

// Função para reduzir a potência do LED
static inline void put_pixel(uint32_t pixel_grb)
{
    uint8_t r = (pixel_grb >> 16) & 0xFF;
    uint8_t g = (pixel_grb >> 8) & 0xFF;
    uint8_t b = pixel_grb & 0xFF;

    // Reduz o brilho dos LEDs
    r /= 32;
    g /= 32;
    b /= 32;

    uint32_t dimmed_pixel = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    pio_sm_put_blocking(pio0, 0, dimmed_pixel << 8u);
}


// Padrões de números 0-10 para a matriz 5x5
const uint8_t numbers[11][25] = {
    {// 0
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 1
     1, 1, 1, 1, 1,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 1,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0},
    {// 2
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 3
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 4
     1, 0, 0, 0, 0,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1},
    {// 5
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1},
    {// 6
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1},
    {// 7
     0, 0, 0, 0, 1,
     0, 1, 0, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 0, 1, 0,
     1, 1, 1, 1, 1},
    {// 8
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 9
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},
    {// 10
     1, 1, 1, 0, 1,
     1, 0, 1, 0, 1,
     1, 0, 1, 0, 1,
     1, 0, 1, 0, 1,
     1, 1, 1, 0, 1},

};


// Função para exibir um número na matriz 5x5
void display_number(int num) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        uint32_t color = numbers[num][i] ? 0x3A0986 : 0x000000; // Roxo escuro ou apagado
        put_pixel(color);
    }
    sleep_ms(1000);
}

// Função para debouncing do botão
bool button_pressed(uint gpio) {
    if (gpio_get(gpio) == 0) {  // Botão pressionado (nível lógico baixo)
        sleep_ms(50);  // Delay para debounce
        if (gpio_get(gpio) == 0) { // Confirma se ainda está pressionado
            while (gpio_get(gpio) == 0);  // Aguarda o botão ser solto
            return true;
        }
    }
    return false;
}

int main() {
    stdio_init_all();

    // Configuração dos botões
    gpio_init(BTN_START);
    gpio_set_dir(BTN_START, GPIO_IN);
    gpio_pull_up(BTN_START);

    gpio_init(BTN_STOP);
    gpio_set_dir(BTN_STOP, GPIO_IN);
    gpio_pull_up(BTN_STOP);

    // Inicializa WS2812
    PIO pio = pio0;
    int sm = 0;
    init_ws2812(pio, sm);

    while (true) {
        // Se o botão de início for pressionado, começa a contagem
        if (button_pressed(BTN_START)) {
            counting = true;
            printf("Contagem iniciada!\n");
        }

        // Se o botão de pausa for pressionado, para a contagem
        if (button_pressed(BTN_STOP)) {
            counting = false;
            printf("Contagem pausada!\n");
        }           

        // Se estiver contando, exibe os números de 0 a 9
        if (counting) {
            for (int i = 0; i < 11;i++) {
                if (!counting) break;  // Verifica a variável antes de exibir o próximo número
                display_number(i);

                // Pequeno delay e verificação do botão de pausa durante a contagem
                for (int j = 0; j < 50; j++) {  // Verifica 50x enquanto espera 500ms
                    if (button_pressed(BTN_STOP)) {
                        counting = false;
                        printf("Contagem pausada!\n");
                        break;
                    }
                    sleep_ms(10);  // Pequenos atrasos para verificar o botão
                }

                if (!counting) break;  // Sai do loop de exibição se counting for falso
            }
        } else {
            sleep_ms(100); // Pequeno delay para evitar alto consumo de CPU
        }
    }
}

