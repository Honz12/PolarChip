#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "instruction_defines.h"

#define MINIMUM_RAM_FOR_SIM 16384 // 16 KiB
#define MINIMUM_RAM_FOR_SIM_SHORT_STR "16k"

#define KILO_BYTE 1024
#define MEGA_BYTE (1024 * 1024)
#define GIGA_BYTE (1024 * 1024 * 1024)

uint32_t get_data_size_from_str(const char *str) {
    uint32_t d = 0;
    while (true) {
        char c = *str;

        if (c >= '0' && c <= '9') {
            d = d * 10 + (c - '0');
        } else {
            switch (c) {
                case 'k': case 'K': return d * KILO_BYTE;
                case 'm': case 'M': return d * MEGA_BYTE;
                case 'g': case 'G': return d * GIGA_BYTE;
                default:            return d;
            }
        }
        str++;
    }
}

void format_data_size_to_str(char *buffer, int buffer_size, uint32_t data_size) {
    char endfix = '\0';
    double value = (double)data_size;

    if (data_size >= GIGA_BYTE) {
        endfix = 'g';
        value = (double)data_size / GIGA_BYTE;
    } 
    else if (data_size >= MEGA_BYTE) {
        endfix = 'm';
        value = (double)data_size / MEGA_BYTE;
    } 
    else if (data_size >= KILO_BYTE) {
        endfix = 'k';
        value = (double)data_size / KILO_BYTE;
    }

    if (endfix != '\0') {
        if (value == (uint32_t)value) {
            snprintf(buffer, buffer_size, "%.0f%c", value, endfix);
        } else {
            snprintf(buffer, buffer_size, "%.2f%c", value, endfix);
        }
    } else {
        snprintf(buffer, buffer_size, "%u", data_size);
    }
}

typedef struct {
    uint32_t qmem[256];

    uint32_t ram_size;
    uint8_t *ram;
    
    bool running;
} CpuData;

int init_cpu_sub(CpuData *cpu_data) {
    char ram_size_str[12];
    if (fgets(ram_size_str, sizeof(ram_size_str), stdin) == NULL) {
        return 1;
    }

    uint32_t ram_size = get_data_size_from_str(ram_size_str);
    char ram_size_formated[12];

    format_data_size_to_str(ram_size_formated, sizeof(ram_size_formated), ram_size);
    printf("Selected %s RAM\n", ram_size_formated);

    if (ram_size < MINIMUM_RAM_FOR_SIM) {
        printf("The minimum allocated ram is %d bytes. (You can also type '%s')\n", MINIMUM_RAM_FOR_SIM, MINIMUM_RAM_FOR_SIM_SHORT_STR);
        return 1;
    }


}

void free_cpu_sub(CpuData *cpu_data) {
    free(cpu_data->ram);
}

void cpu_tick(CpuData *cpu_data) {

}

int main() {

    // Init CPU data

    CpuData *cpu_data = malloc(sizeof(CpuData));

    int init_cpu_sub_error_code = init_cpu_sub(cpu_data);

    if (init_cpu_sub_error_code != 0) {
        return init_cpu_sub_error_code;
    }

    // Free CPU data

    free_cpu_sub(cpu_data);
    free(cpu_data);

    return 0;
}
