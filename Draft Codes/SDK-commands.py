make -r clean
make -r 
openocd -s C:/MaximSDK/Tools/OpenOCD/scripts -f interface/cmsis-dap.cfg -f target/max78000.cfg -c "program build/max78000.elf reset exit"