#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

// Configurações para LEDs WS2812
#define IS_RGBW false // LEDs RGB
#define NUM_PIXELS 25 // Matriz 5x5 de LEDs
#define WS2812_PIN 7  // Pino para os LEDs WS2812

// Função para enviar dados de cor ao LED
static inline void put_pixel(uint32_t pixel_grb) {
    // Reduz a potência do LED dividindo os valores RGB por 4
    uint8_t r = (pixel_grb >> 16) & 0xFF;
    uint8_t g = (pixel_grb >> 8) & 0xFF;
    uint8_t b = pixel_grb & 0xFF;
    r /= 32;
    g /= 32;
    b /= 32;
    uint32_t dimmed_pixel = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    pio_sm_put_blocking(pio0, 0, dimmed_pixel << 8u);
}

// Função para gerar um valor RGB de 24 bits para cada LED
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Padrões para números de 0 a 9 usando uma matriz 5x5
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
void display_number(int num)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (numbers[num][i])
        {
            put_pixel(urgb_u32(0xff, 0xff, 0xff));
        }
        else
        {
            put_pixel(urgb_u32(0, 0, 0)); // Desliga o LED
        }
    }
    sleep_ms(1000); // Pausa de 1 segundo para exibir o número
}

int main()
{
    stdio_init_all();
    printf("WS2812 Number Display\n");

    // Configuração do PIO para os LEDs WS2812
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    while (1)
    {
        for (int i = 0; i < 11; i++)
        {
            display_number(i); // Exibe os números de 0 a 9
            sleep_ms(500);     // Pausa entre os números
        }
    }
}