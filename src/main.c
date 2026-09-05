#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
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

double get_current_time_seconds(void) {
    struct timespec ts;
    // CLOCK_MONOTONIC is immune to system time changes (NTP syncs, etc.)
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

typedef struct {
    uint32_t qmem[256];

    uint32_t pc;
    uint32_t sp;

    uint32_t ram_size;
    uint8_t *ram;

    bool running;
    bool interrupts_enabled;

    uint64_t ips_count;
} ChipData;

void dump_chip_data(ChipData *chip_data) {
    printf("CHIP PC: %u\n", chip_data->pc);
    printf("CHIP SP: %u\n", chip_data->sp);
    printf("CHIP RAM SIZE: %u\n", chip_data->ram_size);
    printf("CHIP RUNNING: %s\n", chip_data->running ? "true" : "false");
    printf("CHIP INTE: %s\n", chip_data->interrupts_enabled ? "true" : "false");
}

uint8_t chip_ram_get_value8(ChipData *chip_data, uint32_t pointer) {
    if (pointer >= chip_data->ram_size) return 0;
    return chip_data->ram[pointer];
}

uint16_t chip_ram_get_value16(ChipData *chip_data, uint32_t pointer) {
    if (pointer + 1 >= chip_data->ram_size) return 0;
    return chip_data->ram[pointer] | chip_data->ram[pointer + 1] << 8;
}

uint32_t chip_ram_get_value24(ChipData *chip_data, uint32_t pointer) {
    if (pointer + 2 >= chip_data->ram_size) return 0;
    return chip_data->ram[pointer] | chip_data->ram[pointer + 1] << 8 | chip_data->ram[pointer + 2] << 16;
}

uint32_t chip_ram_get_value32(ChipData *chip_data, uint32_t pointer) {
    if (pointer + 3 >= chip_data->ram_size) return 0;
    return chip_data->ram[pointer] | chip_data->ram[pointer + 1] << 8 | chip_data->ram[pointer + 2] << 16 | chip_data->ram[pointer + 3] << 24;
}

void chip_ram_write_value8(ChipData *chip_data, uint32_t pointer, uint8_t value) {
    if (pointer >= chip_data->ram_size) return;
    chip_data->ram[pointer] = value;
}

void chip_ram_write_value16(ChipData *chip_data, uint32_t pointer, uint16_t value) {
    if (pointer + 1 >= chip_data->ram_size) return;
    chip_data->ram[pointer] = (uint8_t)value;
    chip_data->ram[pointer + 1] = (uint8_t)(value >> 8);
}

void chip_ram_write_value24(ChipData *chip_data, uint32_t pointer, uint32_t value) {
    if (pointer + 2 >= chip_data->ram_size) return;
    chip_data->ram[pointer] = (uint8_t)value;
    chip_data->ram[pointer + 1] = (uint8_t)(value >> 8);
    chip_data->ram[pointer + 2] = (uint8_t)(value >> 16);
}

void chip_ram_write_value32(ChipData *chip_data, uint32_t pointer, uint32_t value) {
    if (pointer + 3 >= chip_data->ram_size) return;
    chip_data->ram[pointer] = (uint8_t)value;
    chip_data->ram[pointer + 1] = (uint8_t)(value >> 8);
    chip_data->ram[pointer + 2] = (uint8_t)(value >> 16);
    chip_data->ram[pointer + 3] = (uint8_t)(value >> 24);
}

int init_chip_sub(ChipData *chip_data) {
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

    chip_data->ram_size = ram_size;
    chip_data->ram = calloc(ram_size, 1);
    if (chip_data->ram == NULL) {
        printf("Failed to allocate RAM memory.\n");
        return 1;
    }

    chip_data->pc = 0;
    chip_data->sp = 0;
    chip_data->running = true;
    chip_data->interrupts_enabled = false;

    return 0;
}

void free_chip_sub(ChipData *chip_data) {
    free(chip_data->ram);
}

int chip_tick(ChipData *chip_data, bool dump_instruction) {
    if (chip_data->pc >= chip_data->ram_size) {
        //printf("PC out of bounds!\n");
        chip_data->pc = 0;
        return 0;
    }

    uint32_t pc = chip_data->pc;
    uint8_t inst = chip_data->ram[pc];

    switch (inst) {
        case INST_NOP:
            if (dump_instruction) { printf("0x%08x: nop\n", pc); }

            chip_data->pc++;
            break;
        
        default:
            printf("Invalid instruction 0x%02x at PC 0x%08x\n", inst, pc);
            chip_data->running = false;
            return 1;
    }

    return 0;
}

int main() {

    // Init chip data

    ChipData *chip_data = malloc(sizeof(ChipData));
    if (chip_data == NULL) {
        printf("Failed to allocate chip data.");
        return 1;
    }

    int init_chip_sub_error_code = init_chip_sub(chip_data);

    if (init_chip_sub_error_code != 0) {
        free(chip_data);
        return init_chip_sub_error_code;
    }

    double last_time = get_current_time_seconds();
    uint64_t last_ips_count = 0;

    while (chip_data->running) {
        if (chip_tick(chip_data, false) != 0) {
            break;
        }

        chip_data->ips_count++;

        // 3. Check elapsed time to calculate IPS (e.g., every 1 second)
        double current_time = get_current_time_seconds();
        double elapsed = current_time - last_time;

        if (elapsed >= 1.0) {
            // Calculate how many instructions passed since the last check
            uint64_t diff = chip_data->ips_count - last_ips_count;
            double ips = (double)diff / elapsed;

            // Format human-readable IPS output
            if (ips >= 1e6) {
                printf("Performance: %.2f MIPS (Million Instructions Per Second)\n", ips / 1e6);
            } else if (ips >= 1e3) {
                printf("Performance: %.2f kIPS (Kilo Instructions Per Second)\n", ips / 1e3);
            } else {
                printf("Performance: %.0f IPS\n", ips);
            }

            // Reset checkpoints
            last_time = current_time;
            last_ips_count = chip_data->ips_count;
        }
    }

    // Free chip data

    free_chip_sub(chip_data);
    free(chip_data);

    return 0;
}
