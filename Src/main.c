#include <stdio.h>
#include "STM32L476xx.h"
#include "signals.h"
#include "uart.h"
#include "arm_math.h"
#include "systick.h"


#define OFFSET1		5
#define OFFSET2		10


extern float _5hz_signal[HZ_5_SIG_LEN];
extern float32_t input_signal_f32_1kHz_15kHz[KHZ1_15_SIG_LEN];
extern float32_t impulse_response[IMP_RSP_LENGTH];

float32_t output_signal_arr[KHZ1_15_SIG_LEN+IMP_RSP_LENGTH-1];
float g_in_sig_sample;
float g_imp_rsp_sample;


void serial_plot_all(void);

void convolution(float32_t * sig_src_arr,
				 float32_t * sig_dest_arr,
				 float32_t * imp_response_arr,
				 uint32_t 	 sig_src_length,
				 uint32_t 	imp_response_length);

static void pseudo_dly(int dly);
static void fpu_enable(void);

uint32_t g_before, g_after, g_time_taken;
float32_t g_secounds,g_milisecounds_func, g_milisecounds_dsp;
const float SINGLE_CYCLE = 0.00000025;
const int SEC_TO_MSEC = 1000;


int main()
{
	/*Enable FPU */
	fpu_enable();

	/*init the uart*/
	uart2_tx_init();

	/*initilize systick counter*/
	systick_counter_init();

	g_before = SysTick->VAL;
	convolution((float32_t *) input_signal_f32_1kHz_15kHz,
	(float32_t *) output_signal_arr,
	(float32_t *) impulse_response,
	(uint32_t) 	KHZ1_15_SIG_LEN,
	(uint32_t) IMP_RSP_LENGTH);
	g_after = SysTick->VAL;

	/*compute time taken*/
	g_time_taken = g_before - g_after;

	g_secounds = g_time_taken * SINGLE_CYCLE;
	g_milisecounds_func = g_secounds * SEC_TO_MSEC;

	/*CMSIS-DSP*/

	g_before = SysTick->VAL;
    arm_conv_f32((float32_t *) input_signal_f32_1kHz_15kHz,
    			 (uint32_t) 	KHZ1_15_SIG_LEN,
				 (float32_t *) impulse_response,
				 (uint32_t) IMP_RSP_LENGTH,
				 (float32_t *) output_signal_arr
				 );
	g_after = SysTick->VAL;

	/*compute time taken*/
	g_time_taken = g_before - g_after;

	g_secounds = g_time_taken * SINGLE_CYCLE;
	g_milisecounds_dsp = g_secounds * SEC_TO_MSEC;

	printf("Compute time taken costum function: %f [ms] \n " , g_milisecounds_func );
	printf("Compute time taken dsp-cmsis library: %f [ms] \n" , g_milisecounds_dsp );

    while(1)
    {
        //serial_plot_all();
    }
}


void convolution(float32_t * sig_src_arr,
				 float32_t * sig_dest_arr,
				 float32_t * imp_response_arr,
				 uint32_t 	 sig_src_length,
				 uint32_t 	imp_response_length)
{
	uint32_t i,j;
	uint32_t sig_dest_length = sig_src_length + imp_response_length - 1;

	/*clear outpt signal buffer*/
	for(i = 0; i < sig_dest_length; i++)
	{
		sig_dest_arr[i] = 0;
	}

	/* convolution */;
	for(i = 0; i < sig_src_length; i++)
	{
		for(j = 0; j < imp_response_length; j++)
		{
			sig_dest_arr[i+j] = sig_dest_arr[i+j] + sig_src_arr[i]*imp_response_arr[j];
		}
	}
}


void serial_plot_all(void)
{
	uint32_t i,j,k;
	i = j =0;

	for( k = 0; k < (KHZ1_15_SIG_LEN + IMP_RSP_LENGTH - 1) ; k++)
	{
		i++;
		j++;
		if(i == KHZ1_15_SIG_LEN )
		{
			i = 0;
		}
		if(j == IMP_RSP_LENGTH )
		{
			j = 0;
		}
		printf("%f,",OFFSET1 + input_signal_f32_1kHz_15kHz[i]);
		printf("%f,",OFFSET2 + impulse_response[j]);
		printf("%f\n\r", output_signal_arr[k]);
		pseudo_dly(9000);
	}
}


static void pseudo_dly(int dly)
{
	for(int i =0 ; i < dly; i++){}
}

static void fpu_enable(void)
{
	/*Enable FPU */
	SCB->CPACR |=((3UL << 10*2) | (3UL << 11*2));

}
