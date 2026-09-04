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
    uint64_t d = 0;
    while (true) {
        char c = *str;

        if (c >= '0' && c <= '9') {
            d = d * 10 + (c - '0');
            if (d > UINT32_MAX) d = UINT32_MAX;
        } else {
            uint64_t result = d;
            switch (c) {
                case 'k': case 'K': result *= KILO_BYTE; break;
                case 'm': case 'M': result *= MEGA_BYTE; break;
                case 'g': case 'G': result *= GIGA_BYTE; break;
                default: break;
            }
            if (result > UINT32_MAX) return UINT32_MAX;
            return (uint32_t)result;
        }
        str++;
    }
}

void format_data_size_to_str(char *buffer, int buffer_size, uint32_t data_size) {
    const char *endfix = "Bytes";
    double value = (double)data_size;

    if (data_size >= GIGA_BYTE) {
        endfix = "GiB";
        value = (double)data_size / GIGA_BYTE;
    } 
    else if (data_size >= MEGA_BYTE) {
        endfix = "MiB";
        value = (double)data_size / MEGA_BYTE;
    } 
    else if (data_size >= KILO_BYTE) {
        endfix = "KiB";
        value = (double)data_size / KILO_BYTE;
    }

    if (value == (uint32_t)value) {
        snprintf(buffer, buffer_size, "%.0f %s", value, endfix);
    } else {
        snprintf(buffer, buffer_size, "%.2f %s", value, endfix);
    }
}

typedef struct {
    uint32_t qmem[256];

    uint32_t pc;
    uint32_t sp;

    uint32_t ram_size;
    uint8_t *ram;

    bool running;
    bool interrupts_enabled;
} CpuData;

uint8_t cpu_ram_get_value8(CpuData *cpu_data, uint32_t pointer) {
    if (pointer >= cpu_data->ram_size) return 0;
    return cpu_data->ram[pointer];
}

uint16_t cpu_ram_get_value16(CpuData *cpu_data, uint32_t pointer) {
    if (pointer + 1 >= cpu_data->ram_size) return 0;
    return cpu_data->ram[pointer] | cpu_data->ram[pointer + 1] << 8;
}

uint32_t cpu_ram_get_value24(CpuData *cpu_data, uint32_t pointer) {
    if (pointer + 2 >= cpu_data->ram_size) return 0;
    return cpu_data->ram[pointer] | cpu_data->ram[pointer + 1] << 8 | cpu_data->ram[pointer + 2] << 16;
}

uint32_t cpu_ram_get_value32(CpuData *cpu_data, uint32_t pointer) {
    if (pointer + 3 >= cpu_data->ram_size) return 0;
    return cpu_data->ram[pointer] | cpu_data->ram[pointer + 1] << 8 | cpu_data->ram[pointer + 2] << 16 | cpu_data->ram[pointer + 3] << 24;
}

void cpu_ram_write_value8(CpuData *cpu_data, uint32_t pointer, uint8_t value) {
    if (pointer >= cpu_data->ram_size) return;
    cpu_data->ram[pointer] = value;
}

void cpu_ram_write_value16(CpuData *cpu_data, uint32_t pointer, uint16_t value) {
    if (pointer + 1 >= cpu_data->ram_size) return;
    cpu_data->ram[pointer] = (uint8_t)value;
    cpu_data->ram[pointer + 1] = (uint8_t)(value >> 8);
}

void cpu_ram_write_value24(CpuData *cpu_data, uint32_t pointer, uint32_t value) {
    if (pointer + 2 >= cpu_data->ram_size) return;
    cpu_data->ram[pointer] = (uint8_t)value;
    cpu_data->ram[pointer + 1] = (uint8_t)(value >> 8);
    cpu_data->ram[pointer + 2] = (uint8_t)(value >> 16);
}

void cpu_ram_write_value32(CpuData *cpu_data, uint32_t pointer, uint32_t value) {
    if (pointer + 3 >= cpu_data->ram_size) return;
    cpu_data->ram[pointer] = (uint8_t)value;
    cpu_data->ram[pointer + 1] = (uint8_t)(value >> 8);
    cpu_data->ram[pointer + 2] = (uint8_t)(value >> 16);
    cpu_data->ram[pointer + 3] = (uint8_t)(value >> 24);
}

int init_cpu_sub(CpuData *cpu_data) {
    char ram_size_str[32];

    printf("Enter allocated RAM size (bytes, for 16 KiB use '16k'): ");

    if (fgets(ram_size_str, sizeof(ram_size_str), stdin) == NULL) {
        printf("\nPlease enter a valid input.\n");
        return 1;
    }

    uint32_t ram_size = get_data_size_from_str(ram_size_str);
    char ram_size_formated[32];

    format_data_size_to_str(ram_size_formated, sizeof(ram_size_formated), ram_size);
    printf("Selected %s RAM\n", ram_size_formated);

    if (ram_size < MINIMUM_RAM_FOR_SIM) {
        printf("The minimum allocated ram is %d bytes. (You can also type '%s')\n", MINIMUM_RAM_FOR_SIM, MINIMUM_RAM_FOR_SIM_SHORT_STR);
        return 1;
    }

    cpu_data->ram_size = ram_size;
    cpu_data->ram = malloc(ram_size);
    if (cpu_data->ram == NULL) {
        printf("Failed to allocate RAM memory.\n");
        return 1;
    }

    cpu_data->pc = 0;
    cpu_data->sp = 0;
    cpu_data->running = true;
    cpu_data->interrupts_enabled = false;

    return 0;
}

void free_cpu_sub(CpuData *cpu_data) {
    free(cpu_data->ram);
}

int cpu_tick(CpuData *cpu_data) {
    if (cpu_data->pc >= cpu_data->ram_size) {
        printf("PC out of bounds!\n");
        cpu_data->running = false;
        return 1;
    }

    uint32_t pc = cpu_data->pc;
    uint8_t inst = cpu_data->ram[pc];

    switch (inst) {
        default:
            printf("Invalid instruction 0x%02x at PC 0x%08x\n", inst, pc);
            cpu_data->running = false;
            return 1;
    }

    return 0;
}

int main() {

    // Init CPU data

    CpuData *cpu_data = malloc(sizeof(CpuData));
    if (cpu_data == NULL) {
        printf("Failed to allocate CPU data.");
        return 1;
    }

    int init_cpu_sub_error_code = init_cpu_sub(cpu_data);

    if (init_cpu_sub_error_code != 0) {
        free(cpu_data);
        return init_cpu_sub_error_code;
    }

    while (cpu_data->running) {
        if (cpu_tick(cpu_data) != 0) {
            break; 
        }
    }

    // Free CPU data

    free_cpu_sub(cpu_data);
    free(cpu_data);

    return 0;
}
