#include "rfid_mfrc522.h"
#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>

#define RST_DDR     DDRH
#define RST_PORT    PORTH
#define RST_PIN     PH6  // Deze pin moet misschien nog aangepast worden!

#define MFRC522_COMMAND_REG  0x01
#define MFRC522_FIFO_DATA_REG 0x09
#define MFRC522_FIFO_LEVEL_REG 0x0A
#define MFRC522_BIT_FRAMING_REG 0x0D
#define MFRC522_TX_CONTROL_REG 0x14
#define MFRC522_COMMAND_TRANSCEIVE 0x0C
#define MFRC522_COM_IRQ_REG 0x04
#define MFRC522_COLL_REG 0x0E
#define MFRC522_MODE_REG 0x11
#define MFRC522_TMODE_REG 0x2A
#define MFRC522_TPRESCALER_REG 0x2B
#define MFRC522_TRELOAD_REG_L 0x2D
#define MFRC522_TRELOAD_REG_H 0x2C
#define MFRC522_TXASK_REG 0x15

#define PICC_CMD_REQA 0x26

void spi_init() {
    DDRB |= (1 << PB2) | (1 << PB1) | (1 << PB0); // MOSI (D51), SCK (D52), SS (D53)
    DDRB &= ~(1 << PB3); // MISO (D50) als input
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0); // SPI Enable, Master mode, clk/16
}

uint8_t spi_transfer(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
    return SPDR;
}

static void write_register(spi_device dev, uint8_t reg, uint8_t val) {
    *(dev.port) &= ~(1 << dev.pin);     // Slave select
    spi_transfer((reg << 1) & 0x7E);
    spi_transfer(val);
    *(dev.port) |= (1 << dev.pin);      // Slave deselect
}

static uint8_t read_register(spi_device dev, uint8_t reg) {
    uint8_t val;
    *(dev.port) &= ~(1 << dev.pin);     // Slave select
    spi_transfer(((reg << 1) & 0x7E) | 0x80);
    val = spi_transfer(0);
    *(dev.port) |= (1 << dev.pin);      // Slave deselect
    return val;
}

void rfid_init(spi_device dev) {
    *(dev.ddr) |= (1 << dev.pin);   // SS output
    RST_DDR |= (1 << RST_PIN);         // Reset output
    *(dev.port) |= (1 << dev.pin);  // Deselect slave
    RST_PORT |= (1 << RST_PIN);        // Startup

    spi_init();

    RST_PORT &= ~(1<<RST_PIN);         // Hard reset
    RST_PORT |= (1 << RST_PIN);        // Startup

    while(read_register(dev, 0x37) != 0x92); // Check version to see if reset is complete
    write_register(dev, 0x26, 0x70); // Max gain
    write_register(dev, MFRC522_TMODE_REG, 0x80);
    write_register(dev, MFRC522_TPRESCALER_REG, 0xA9);
    write_register(dev, MFRC522_TRELOAD_REG_L, 0xE8);
    write_register(dev, MFRC522_TRELOAD_REG_H, 0x03);
    write_register(dev, MFRC522_TXASK_REG, 0x40);
    write_register(dev, MFRC522_MODE_REG, 0x3D);

    uint8_t val = read_register(dev, MFRC522_TX_CONTROL_REG);
    if (!(val & 0x03)) {
        write_register(dev, MFRC522_TX_CONTROL_REG, val | 0x03);
    }


}

bool rfid_check_tag_present(spi_device dev) {
    write_register(dev, MFRC522_BIT_FRAMING_REG, 0x07); // Set bitframing to 7 bits
    write_register(dev, MFRC522_COMMAND_REG, 0x00); // Set to idle
    write_register(dev, MFRC522_FIFO_DATA_REG, PICC_CMD_REQA); // Reqa command
    write_register(dev, MFRC522_COMMAND_REG, MFRC522_COMMAND_TRANSCEIVE); // Transceive
    write_register(dev, MFRC522_BIT_FRAMING_REG, 0x87);

    uint8_t i = 0;
    for (i = 0; i < 200; i++) {
        uint8_t irq = read_register(dev, MFRC522_COM_IRQ_REG);
        if (irq & 0x30) break;
    }

    uint8_t error = read_register(dev, 0x06); // ErrorReg
    if (error & 0x1B) return false;

    uint8_t len = read_register(dev, MFRC522_FIFO_LEVEL_REG);
    write_register(dev, MFRC522_FIFO_LEVEL_REG, 0x80); // Flush FIFO
    return (len != 0);
}

bool rfid_tag_detected(spi_device dev){
    for(int i = 0; i < 2; i++){
        if(rfid_check_tag_present(dev)){
            return 1;
        }
        // Very short delay needed, can be replaced with short timer
        _delay_ms(1);
    }
    return 0;
}
