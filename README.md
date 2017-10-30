# w5500_stm32cube_driver
drive w5500 network chip using STM32Cube HAL

This project depends Wiznet ioLibrary: https://github.com/Wiznet/ioLibrary_Driver   
No need to change the ioLibrary code. 
Just add the .c and .h files to your project.

Then add network.h and network.c to your project.

You need to label the w5500 spi cs pin as WCS in STM32CubeMX.  
Or change the code in network.c to proper value.  
Then include network.h in main.c and call network_start();  

More information and code examples here:  
https://github.com/Wiznet   


