#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define SERVO_MAJOR_NUMBER 505
#define SERVO_DEV_NAME   "servo"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0X04

// clock control
#define CLK_BASE_ADDR 0x3F101000

//offset
#define CLK_PWM_CTL 0xa0
#define CLK_PWM_DIV 0xa4

//
#define BCM_PASSWORD 0x5A000000

//PWM address set.
#define PWM_BASE_ADDR 0x3F20C000
#define PWM_CTL 0x00
#define PWM_RNG1 0x10
#define PWM_DAT1 0x14

static void __iomem *gpio_base;
volatile unsigned int *gpsel1;

static void __iomem *clk;
volatile unsigned int *clkdiv;
volatile unsigned int *clkctl;

static void __iomem *pwm;
volatile unsigned int *pwmctl;
volatile unsigned int *pwmrng1;
volatile unsigned int *pwmdat1;


#define IOCTL_MAGIC_NUMBER 'p'
#define IOCTL_CMD_SET_DIRECTION _IOWR(IOCTL_MAGIC_NUMBER, 0, int)

int init_pwm(void);

int servo_open(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "SERVO driver open!!\n");
   
   gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
   gpsel1=(volatile unsigned int *)(gpio_base+GPFSEL1);
   
   clk = ioremap(CLK_BASE_ADDR, 0xFF);   //size
   clkdiv=(volatile unsigned int*)(clk+CLK_PWM_DIV);
   clkctl=(volatile unsigned int*)(clk+CLK_PWM_CTL);
   
   pwm = ioremap(PWM_BASE_ADDR, 0xFF);
   pwmctl = (volatile unsigned int*)(pwm+PWM_CTL);
   pwmrng1 = (volatile unsigned int*)(pwm+PWM_RNG1);
   pwmdat1 = (volatile unsigned int*)(pwm+PWM_DAT1);

   return 0;
}

int servo_release(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "SERVO driver close!!\n");
   iounmap((void*)gpio_base);
   iounmap((void*)pwm);
   iounmap((void*)clk);
   return 0;
}

long servo_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
   int kbuf = 0;
   int i = 0;
   init_pwm();
   switch(cmd) {
      //Not only direction setting but PWM Setting. 
      case IOCTL_CMD_SET_DIRECTION:
         //Original Code of LED_IOCTL
         copy_from_user(&kbuf, (const void*)arg, 4);
         *gpsel1 &= ~(1<<1);   
         *gpsel1 |= (1<<25);
         *gpsel1 &= ~(1<<1);
         printk(KERN_ALERT "SERVO set direction out!!\n");
         *pwmctl |= (1);      //PWEN       1
         *pwmctl &= ~(1<<1);    //MODE1      0
         *pwmctl |= (1<<7);    //MSEN1      MS   1
         *pwmrng1 = 320;      //RANGE      320
         *pwmdat1|= (1<<5);    //DAT      32   (1/10)
         break;
      default:
         printk(KERN_ALERT "SERVO ioctl cmd error!\n");
         break;
   }

   return 0;
}

int init_pwm(void) {
   int pwm_ctrl = *pwmctl;
   *pwmctl = 0; // store PWM control and stop PWM
   msleep(10);//
   *clkctl = BCM_PASSWORD | (0x01 << 5); // stop PWM Clock
   msleep(10);//

   int idiv = (int)(19200000.0f / 16000.0f); // Oscilloscope to 16kHz
   *clkdiv = BCM_PASSWORD | (idiv << 12); // integer part of divisior register
   *clkctl = BCM_PASSWORD | (0x11); //set source to oscilloscope & enable PWM CLK

   *pwmctl = pwm_ctrl; // restore PWM control and enable PWM
}

static struct file_operations servo_fops = {
   .owner = THIS_MODULE,
   .open = servo_open,
   .release = servo_release,
   .unlocked_ioctl = servo_ioctl,
};
   
int __init servo_init(void){
   if(register_chrdev(SERVO_MAJOR_NUMBER, SERVO_DEV_NAME, &servo_fops) < 0)
      printk(KERN_ALERT "SERVO driver initialization fail\n");
   else
      printk(KERN_ALERT "SERVO driver initialization success\n");
   
   return 0;
}

void __exit servo_exit(void){
   unregister_chrdev(SERVO_MAJOR_NUMBER, SERVO_DEV_NAME);
   printk(KERN_ALERT "SERVO driver exit\n");
}

module_init(servo_init);
module_exit(servo_exit);

   
