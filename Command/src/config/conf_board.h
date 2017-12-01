/**
 * \file
 *
 * \brief User board configuration template
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#define				AVR32_GPIO_IRQ7			71		// Groupe 2 , line 7 (32 * 2) + 7 = 71 (int PB24..27)
#define				AVR32_GPIO_IRQ3			67		// Groupe 2 , line 3 (32 * 2) + 3 = 67 (int PA28)

#define SD_MMC_SPI ((volatile avr32_spi_t*) &AVR32_SPI1)
#define SD_MMC_SPI_NPCS 1

#define DAC1_SPI AVR32_SPI1_ADDRESS
#define DAC1_SPI_NPCS 2

#define SCREEN_SPI AVR32_SPI0_ADDRESS
#define SCREEN_SPI_NPCS 0


#define BOARD_OSC0_HZ	64000000

// Définition des interruptions des timers
#define TC2_CHANNEL 2
#define TC1_CHANNEL 1
#define TC0_CHANNEL 0


// Définition des broches de l'UART0 pour l'affichage 7 segements //
#define				USART1_TXD_PIN			6
#define				USART1_TXD_FCT			0
#define				USART1_RXD_PIN			5
#define				USART1_RXD_FCT			0
#define				USART1_CLK_PIN			7
#define				USART1_CLK_FCT			0
#define				USART1_RTS_PIN			8
#define				USART1_RTS_FCT			0
#define				USART1_CTS_PIN			9
#define				USART1_CTS_FCT			0


// Définition des broches des deux SPI //
#define				PIN_MISO_SPI0		11
#define				FCT_MISO_SPI0		0		// Fonction A MISO SPI0
#define				PIN_MOSI_SPI0		12
#define				FCT_MOSI_SPI0		0		// Fonction A MOSI SPI0
#define				PIN_SCK_SPI0		13
#define				FCT_SCK_SPI0		0		// Fonction A SCK SPI0

#define				PIN_SCK_SPI1		15
#define				FCT_SCK_SPI1		1		// Fonction B SCK SPI1
#define				PIN_MOSI_SPI1		16
#define				FCT_MOSI_SPI1		1		// Fonction B MOSI SPI1
#define				PIN_MISO_SPI1		17
#define				FCT_MISO_SPI1		1		// Fonction B MISO SPI1

#define				PIN_NPCS_TFT		10
#define				FCT_NPCS_TFT		0		// Fonction A
#define				PIN_NPCS_SD			18
#define				FCT_NPCS_SD			1		// Fonction B NPCS[1] SPI1
#define				PIN_NPCS_DA			19
#define				FCT_NPCS_DA			1		// Fonction B NPCS[2] SPI1
#define				PIN_NPCS_KEY		20
#define				FCT_NPCS_KEY		1		// Fonction B NPCS[3] SPI1

// Définition des broches de l'I2C pour le DS3232M //
#define				PIN_SDA				29
#define				FCT_SDA				0		// Fonction A TWI SDA
#define				PIN_SCL				30
#define				FCT_SCL				0		// Fonction A TWI SCL
#define				PIN_INT1			28
#define				PIN_RTC_RST			31

// Définition des broches des interruptions externes //
#define				EXT_INT0_PIN		21
#define				EXT_INT0_FCT		1		// Fonction B EIM EXTINT[0]
#define				EXT_INT1_PIN		22
#define				EXT_INT1_FCT		1		// Fonction B EIM EXTINT[1]
#define				EXT_INT3_PIN		24
#define				EXT_INT3_FCT		1		// Fonction B EIM EXTINT[1]


// Définition des broches pour les switchs //
#define				PIN_SWITCH0			56
#define				PIN_SWITCH1			57
#define				PIN_SWITCH2			58
#define				PIN_SWITCH3			59

// Définition des broches de l'affichage TFT //
#define				PIN_PWM_TFT			60
#define				FCT_PWM_TFT			1
#define				PIN_RESET_TFT		14

// Définition des broches du clavier tactile //
#define				PIN_INT_KEY			61
#define				PIN_BUSY_KEY		39

// Essai du Audio bitStream DAC //
#define				DAC_DATA1			23
#define				FCT_DATA1			2
#define				DAC_DATAN1			24
#define				FCT_DATAN1			2
#define				DAC_DATA0			3
#define				FCT_DATA0			2
#define				DAC_DATAN0			4
#define				FCT_DATAN0			2
#define				PIN_SHUTDOWN		25

// Définition des broches du convertisseur DA5333
#define				DAC_DB0_PIN			32
#define				DAC_DB1_PIN			33
#define				DAC_DB2_PIN			34
#define				DAC_DB3_PIN			35
#define				DAC_DB4_PIN			36
#define				DAC_DB5_PIN			37
#define				DAC_DB6_PIN			38
#define				DAC_DB7_PIN			39
#define				DAC_DB8_PIN			40
#define				DAC_DB9_PIN			41
#define				DAC_PD_PIN			42
#define				DAC_A0_PIN			43
#define				DAC_CS_PIN			44
#define				DAC_WR_PIN			45
#define				DAC_LDAC_PIN		46
#define				DAC_CLR_PIN			47

// Définition des broches de Timer 
#define PIN_T1_IOA	57
#define FCT_T1_IOA 0		//Fonction A

#define PIN_T0_IOA 55
#define FCT_T0_IOA 0

// Définition des ports
#define PORTA port[0]
#define PORTB port[1]
#define PORTC port[2]


#endif // CONF_BOARD_H
