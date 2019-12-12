#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

//#define CS_MCP3208	6	// BCM_GPIO 25
#define CS_MCP3208	10	// BCM_GPIO 8


#define SPI_CHANNEL	0
#define SPI_SPEED	1000000	// 1MHz

#define INTERVAL	100000

int read_mcp3208_adc(unsigned char adcChannel){
	unsigned char buff[3];
	int adcValue = 0;

	buff[0] = 0x06 | ((adcChannel & 0x07) >> 7); 
	buff[1] = ((adcChannel & 0x07) << 6);
	buff[2] = 0x00;

	digitalWrite(CS_MCP3208, 0);	// Low : CS Active, active adc

	wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);	// read data and store in buff

	buff[1] = 0x0F & buff[1];
	//printf("buff[1] : %x\n", buff[1]);
	//printf("buff[2] : %x\n", buff[2]);
	adcValue = (buff[1] << 8) | buff[2];

	digitalWrite(CS_MCP3208, 1);	// High : CS Inactive, deactive adc

	return adcValue;
}

int main(void){
	int adcChannel = 0;
	int adcValue = 0;

	
	if(wiringPiSetup() == -1){
		fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
		return 1;
	}
	
	if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1){
		fprintf(stdout, "wiringPiSPISetup Failed: %s\n", strerror(errno));
		return 1;
	}
	
	
	
	pinMode(CS_MCP3208, OUTPUT);
	

	while(1){
		usleep(INTERVAL);
		adcValue = read_mcp3208_adc(adcChannel);
		printf("adc0 Value = %u\n", adcValue);
	}

	return 0;
}











