################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
char_receiver.obj: ../char_receiver.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C5500 Compiler'
	"C:/Programs/ti/ccsv6/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -g --include_path="C:/Programs/ti/ccsv6/tools/compiler/c5500_4.4.1/include" --include_path="C:/Programs/ti/ccsv7/ccs_base/emulation/boards/usbstk5515_v1/include" --define=c5515 --display_error_number --diag_warning=225 --ptrdiff_size=16 --preproc_with_compile --preproc_dependency="char_receiver.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


