#include <linux/init.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/fs.h> 
#include <linux/uaccess.h> 
#include <linux/slab.h> 

#include <asm/mach/map.h> 
#include <asm/uaccess.h> 

#define SOIL_MOIS_MAJOR_NUMBER	501
#define SOIL_MOIS_DEV_NAME		"soil_mois"

#define GPIO_BASE_ADDRESS	0x3F200000

#define GPFSEL0	0x00
#define GPLEV0	0x34
#define GPSET0	0x1C
#define GPCLR0	0x28

#define SPI_CHANNEL	0	// CH0 in ADC
#define SPI_SPEED	1000000	// 1MHz

#define IOCTL_MAGIC_NUMBER	'j'
#define IOCTL_CMD_SET_SPI_ACTIVE		_IOWR(IOCTL_MAGIC_NUMBER, 0, int)
#define IOCTL_CMD_SET_SPI_INACTIVE		_IOWR(IOCTL_MAGIC_NUMBER, 1, int)

static void __iomem * gpio_base;
volatile unsigned int * gpsel0;
volatile unsigned int * gplev0;
volatile unsigned int * gpset0;
volatile unsigned int * gpclr0;

int soil_mois_open(struct inode * inode, struct file * filp){
	printk(KERN_ALERT "soil_mois driver open\n");
	
	gpio_base = ioremap(GPIO_BASE_ADDRESS, 0xFF);
	gpsel0 = (volatile unsigned int *)(gpio_base + GPFSEL0);
	gplev0 = (volatile unsigned int *)(gpio_base + GPLEV0);
	gpset0 = (volatile unsigned int *)(gpio_base + GPSET0);
	gpclr0 = (volatile unsigned int *)(gpio_base + GPCLR0);
	
	return 0;
}

int soil_mois_release(struct inode * inode, struct file * filp){
	printk(KERN_ALERT "soil_mois driver close\n");
	iounmap((void *)gpio_base);
	return 0;
}

// arg is adcChannel
long soil_mois_ioctl(struct file * filp, unsigned int cmd, unsigned long arg){
	unsigned char buff[3];	// communication with ADC
	int get_value;
	
	switch (cmd){
		case IOCTL_CMD_SET_SPI_ACTIVE:
			// set RBP gpio to output mode
			*gpsel0 |= (1 << 24); // BCM gpio 8 is setted output mode
			
			// LOW : chip select active in adc
			*gpclr0 |= (1 << 8);
			break;
		case IOCTL_CMD_SET_SPI_INACTIVE:
			// High : chip select inactive in adc
			*gpset0 |= (1 << 8);
			break;
		default:
			printk(KERN_ALERT "ioctl : command error\n");
	}
	return 0;
}

static struct file_operations soil_mois_fops = {
	.owner = THIS_MODULE,
	.open = soil_mois_open,
	.release = soil_mois_release,
	.unlocked_ioctl = soil_mois_ioctl
};

int __init soil_mois_init (void){
	if(register_chrdev(SOIL_MOIS_MAJOR_NUMBER, SOIL_MOIS_DEV_NAME, &soil_mois_fops) < 0)
		printk(KERN_ALERT "soil_mois driver initialization failed\n");
	else
		printk(KERN_ALERT "soil_mois driver suceed\n");
	
	return 0;
}

void __exit soil_mois_exit(void){
	unregister_chrdev(SOIL_MOIS_MAJOR_NUMBER, SOIL_MOIS_DEV_NAME);
	printk(KERN_ALERT "soil mois driver exit");
}

module_init(soil_mois_init);
module_exit(soil_mois_exit);
