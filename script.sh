sudo rmmod wls
cd water_level_sensor
sudo make clean
make
sudo insmod wls.ko
cd ..
rm app
gcc -o app app.c
