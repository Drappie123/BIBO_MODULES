#ifndef RFID_MFRC522_H
#define RFID_MFRC522_H

#include <stdint.h>
#include <stdbool.h>

    typedef struct {
    volatile uint8_t *port;
    uint8_t pin;
    volatile uint8_t *ddr;
} spi_device;

void spi_init();
uint8_t spi_transfer(uint8_t data);

void rfid_init(spi_device dev);
bool rfid_check_tag_present(spi_device dev);
bool rfid_tag_detected(spi_device dev);

#endif

