#include "app_thread.h"
/* App Thread entry function */
/* pvParameters contains TaskHandle_t */
void app_thread_entry(void *pvParameters) {
	FSP_PARAMETER_NOT_USED(pvParameters);

	/* TODO: add your own code here */
	FleetProvisioningDemo();
}
