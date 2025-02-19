#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_ADDR 0x3C

#define VRX_PIN 26  // GPIO para eixo X do joystick
#define VRY_PIN 27  // GPIO para eixo Y do joystick
#define SW_PIN 22   // Botão do joystick
#define BUTTON_A 5  // Botão A

#define LED_R 13  // GPIO LED Vermelho
#define LED_G 11  // GPIO LED Verde
#define LED_B 12  // GPIO LED Azul

volatile bool pwm_active = true;
volatile uint8_t border_style = 0;
ssd1306_t ssd;

void debounce_irq(uint gpio, uint32_t events) {
    static uint32_t last_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_time < 200) return; // Debounce de 200ms
    last_time = current_time;
    
    if (gpio == SW_PIN) {
        gpio_put(LED_G, !gpio_get(LED_G)); // Alterna o LED Verde
        border_style = (border_style + 1) % 3; // Alterna estilos de borda
    } else if (gpio == BUTTON_A) {
        pwm_active = !pwm_active;
    }
}

void setup_pwm(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, 4095);
    pwm_set_enabled(slice, true);
}

int main() {
    stdio_init_all();
    
    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);
    gpio_set_irq_enabled_with_callback(SW_PIN, GPIO_IRQ_EDGE_FALL, true, &debounce_irq);
    
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &debounce_irq);
    
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, false);
    
    setup_pwm(LED_R);
    setup_pwm(LED_B);
    
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    ssd1306_init(&ssd, 128, 64, false, I2C_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    printf("Display inicializado com sucesso\n");
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    sleep_ms(50);
    
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);
    
    uint16_t x_value, y_value;
    int square_x = 64 - 4, square_y = 32 - 4;
    while (true) {
        adc_select_input(0);
        x_value = adc_read();
        sleep_ms(2);

        adc_select_input(1);
        y_value = adc_read();
        sleep_ms(2);

        if (pwm_active) {
            pwm_set_gpio_level(LED_B, abs(y_value - 2048) * 4095 / 2048);
            pwm_set_gpio_level(LED_R, abs(x_value - 2048) * 4095 / 2048);
        } else {
            pwm_set_gpio_level(LED_B, 0);
            pwm_set_gpio_level(LED_R, 0);
        }

        square_x = ((y_value - 2048) * 56 / 2048) + 64;
        if (square_x < 0) square_x = 0;
        if (square_x > 120) square_x = 120;

        square_y = ((2048 - x_value) * 32 / 2048) + 32;
        if (square_y < 0) square_y = 0;
        if (square_y > 56) square_y = 56;

        ssd1306_fill(&ssd, false);
        ssd1306_rect(&ssd, square_x, square_y, 8, 8, true, true);

        if (border_style == 1) {
            ssd1306_rect(&ssd, 0, 2, 127, 61, true, false);  
        } else if (border_style == 2) {
            ssd1306_line(&ssd, 0, 2, 127, 2, true);
            ssd1306_line(&ssd, 0, 63, 127, 63, true);
            ssd1306_line(&ssd, 0, 2, 0, 63, true);
            ssd1306_line(&ssd, 127, 2, 127, 63, true);
        }
        
        ssd1306_send_data(&ssd);
        sleep_ms(100);
    }
}
