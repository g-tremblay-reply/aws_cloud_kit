/***********************************************************************************************************************
 * File Name    : cloud_prov_network.c
 * Description  : Contains functions used in Renesas Cloud Connectivity application
 **********************************************************************************************************************/

#include "cloud_app/cloud_app.h"
#include "led/led.h"
#include "console/console.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_DHCP.h"


#define CLOUD_PROV_ETH_CONFIG    "\r\n\r\n--------------------------------------------------------------------------------"\
                                "\r\nEthernet adapter Configuration for Renesas "KIT_NAME": Post IP Init       "\
                                "\r\n--------------------------------------------------------------------------------\r\n\r\n"

/**
 * @brief External reference to task handle of cloud app.
 * @details This is needed to be used as input parameter in xTaskNotifyFromISR(), called in
 *          vApplicationIPNetworkEventHook(), a FreeRTOS_IP user callback implemented by cloud_app module.
 */
extern TaskHandle_t cloud_app_thread;

IPV4Parameters_t xNd = {RESET_VALUE,
                        RESET_VALUE,
                        RESET_VALUE,
                        {RESET_VALUE,RESET_VALUE},
                        RESET_VALUE,
                        RESET_VALUE};
static uint8_t ucIPAddress[ipIP_ADDRESS_LENGTH_BYTES] =        { RESET_VALUE };
static uint8_t ucNetMask[ipIP_ADDRESS_LENGTH_BYTES] =          { 255, 255, 255, 255 };
static uint8_t ucGatewayAddress[ipIP_ADDRESS_LENGTH_BYTES] =   { RESET_VALUE };
static uint8_t ucDNSServerAddress[ipIP_ADDRESS_LENGTH_BYTES] = {75, 75, 75, 75};
static uint8_t ucMACAddress[ipMAC_ADDRESS_LENGTH_BYTES] =       { 0x00, 0x11, 0x22, 0x33, 0x44, 0x57 };


/**********************************************************************************************************************
                                    LOCAL FUNCTION PROTOTYPES
**********************************************************************************************************************/
/**
 * @brief      Creates and prints  the IP configuration to display on the  console
 * @param[in]  void
 * @retval     None
 */
static void CloudProv_PrintIPConfig(void);


/**********************************************************************************************************************
                                    LOCAL FUNCTIONS
**********************************************************************************************************************/
static void CloudProv_PrintIPConfig(void)
{
#if( ipconfigUSE_DHCP != 0 )

    ucNetMask[3] = (uint8_t) ((xNd.ulNetMask & 0xFF000000) >> 24);
    ucNetMask[2] = (uint8_t) ((xNd.ulNetMask & 0x00FF0000) >> 16);
    ucNetMask[1] = (uint8_t) ((xNd.ulNetMask & 0x0000FF00) >> 8);
    ucNetMask[0] = (uint8_t) (xNd.ulNetMask & 0x000000FF);

    ucGatewayAddress[3] = (uint8_t) ((xNd.ulGatewayAddress & 0xFF000000) >> 24);
    ucGatewayAddress[2] = (uint8_t) ((xNd.ulGatewayAddress & 0x00FF0000) >> 16);
    ucGatewayAddress[1] = (uint8_t) ((xNd.ulGatewayAddress & 0x0000FF00) >> 8);
    ucGatewayAddress[0] = (uint8_t) (xNd.ulGatewayAddress & 0x000000FF);

    ucDNSServerAddress[3] = (uint8_t)((xNd.ulDNSServerAddresses[0] & 0xFF000000)>> 24);
    ucDNSServerAddress[2] = (uint8_t)((xNd.ulDNSServerAddresses[0] & 0x00FF0000)>> 16);
    ucDNSServerAddress[1] = (uint8_t)((xNd.ulDNSServerAddresses[0] & 0x0000FF00)>> 8);
    ucDNSServerAddress[0] = (uint8_t)(xNd.ulDNSServerAddresses[0] & 0x000000FF);

    ucIPAddress[3] = (uint8_t)((xNd.ulIPAddress & 0xFF000000) >> 24);
    ucIPAddress[2] = (uint8_t)((xNd.ulIPAddress & 0x00FF0000) >> 16);
    ucIPAddress[1] = (uint8_t)((xNd.ulIPAddress & 0x0000FF00) >> 8);
    ucIPAddress[0] = (uint8_t)(xNd.ulIPAddress & 0x000000FF);
#endif
    APP_PRINT(CLOUD_PROV_ETH_CONFIG);

    APP_PRINT("\tDescription . . . . . . . . . . . : Renesas "KIT_NAME" Ethernet\r\n");
    APP_PRINT("\tPhysical Address. . . . . . . . . : %02x-%02x-%02x-%02x-%02x-%02x\r\n", ucMACAddress[0],
              ucMACAddress[1], ucMACAddress[2], ucMACAddress[3], ucMACAddress[4], ucMACAddress[5]);
    APP_PRINT("\tDHCP Enabled. . . . . . . . . . . : %s\r\n", "Yes" );
    APP_PRINT("\tIPv4 Address. . . . . . . . . . . : %d.%d.%d.%d\r\n", ucIPAddress[0], ucIPAddress[1], ucIPAddress[2],
              ucIPAddress[3]);
    APP_PRINT("\tSubnet Mask . . . . . . . . . . . : %d.%d.%d.%d\r\n", ucNetMask[0], ucNetMask[1], ucNetMask[2],
              ucNetMask[3]);
    APP_PRINT("\tDefault Gateway . . . . . . . . . : %d.%d.%d.%d\r\n", ucGatewayAddress[0], ucGatewayAddress[1],
              ucGatewayAddress[2], ucGatewayAddress[3]);
    APP_PRINT("\tDNS Servers . . . . . . . . . . . : %d.%d.%d.%d\r\n\r\n", ucDNSServerAddress[0], ucDNSServerAddress[1],
              ucDNSServerAddress[2], ucDNSServerAddress[3]);
}
/**********************************************************************************************************************
                                    GLOBAL FUNCTION PROTOTYPES
**********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief      DHCP Hook function to populate the user defined Host name for the Kit.
 * @param[in]  None
 * @retval     Hostname
 **********************************************************************************************************************/
#if( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
const char* pcApplicationHostnameHook(void)
{
    return KIT_NAME;
}
#endif

#if( ipconfigUSE_DHCP != 0 )
/**
 * @brief      This is the User Hook for the DHCP Response. xApplicationDHCPHook() is called by DHCP Client Code when DHCP
 *             handshake messages are exchanged from the Server.
 * @param[in]  Different Phases of DHCP Phases and the Offered IP Address
 * @retval     Returns DHCP Answers.
 */
eDHCPCallbackAnswer_t xApplicationDHCPHook(eDHCPCallbackPhase_t eDHCPPhase, uint32_t lulIPAddress)
{
    eDHCPCallbackAnswer_t eReturn = eDHCPContinue;
    /*
     * This hook is called in a couple of places during the DHCP process, as identified by the eDHCPPhase parameter.
     */
    switch (eDHCPPhase)
    {
        case eDHCPPhasePreDiscover:
            /*
             *  A DHCP discovery is about to be sent out.  eDHCPContinue is returned to allow the discovery to go out.
             *  If eDHCPUseDefaults had been returned instead then the DHCP process would be stopped and the statically
             *  configured IP address would be used.
             *  If eDHCPStopNoChanges had been returned instead then the DHCP process would be stopped and whatever the
             *  current network configuration was would continue to be used.
             */
            break;

        case eDHCPPhasePreRequest:
            /* An offer has been received from the DHCP server, and the offered IP address is passed in the lulIPAddress
             * parameter.
             */
            /*
             * The sub-domains don’t match, so continue with the DHCP process so the offered IP address is used.
             */
            /* Update the Structure, the DHCP state Machine is not updating this */
            xNd.ulIPAddress = lulIPAddress;
            xNd.ulNetMask = FreeRTOS_GetNetmask();
            xNd.ulGatewayAddress = FreeRTOS_GetGatewayAddress();
            xNd.ulDNSServerAddresses[0] = FreeRTOS_GetDNSServerAddress();
            break;

        default:
            /*
             * Cannot be reached, but set eReturn to prevent compiler warnings where compilers are disposed to generating one.
             */
            break;
    }
    return eReturn;
}
#endif

#if ( ipconfigUSE_NETWORK_EVENT_HOOK == 1 )
/**
 * @brief      Network event callback. Indicates the Network event. Added here to avoid the build errors
 * @param[in]  None
 * @retval     Hostname
 */
void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{
    if (eNetworkUp == eNetworkEvent)
    {
        uint32_t lulIPAddress;
        uint32_t lulNetMask;
        uint32_t lulGatewayAddress;
        uint32_t lulDNSServerAddress;
        int8_t lcBuffer[16];

        /* Signal application the network is UP */
        xTaskNotifyFromISR(cloud_app_thread, eNetworkUp, eSetBits, NULL);

        /* The network is up and configured.  Print out the configuration
         obtained from the DHCP server. */
        FreeRTOS_GetAddressConfiguration (&lulIPAddress,
                                          &lulNetMask,
                                          &lulGatewayAddress,
                                          &lulDNSServerAddress);

        /* Convert the IP address to a string then print it out. */
        FreeRTOS_inet_ntoa (lulIPAddress, (char*) lcBuffer);

        /* Convert the net mask to a string then print it out. */
        FreeRTOS_inet_ntoa (lulNetMask, (char*) lcBuffer);

        /* Convert the IP address of the gateway to a string then print it out. */
        FreeRTOS_inet_ntoa (lulGatewayAddress, (char*) lcBuffer);

        /* Convert the IP address of the DNS server to a string then print it out. */
        FreeRTOS_inet_ntoa (lulDNSServerAddress, (char*) lcBuffer);
    }
}
#endif

void CloudProv_InitIPStack(void)
{
    BaseType_t status;

    status = FreeRTOS_IPInit (ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);
    if(status != pdTRUE)
    {
        FAILURE_INDICATION;
        APP_ERR_PRINT("User Network Initialization Failed");
        APP_ERR_TRAP(pdFALSE);
    }
    else
    {
        APP_PRINT("[ORANGE]Waiting for IP stack link up...[WHITE]");
        /* Wait on notification for cloud_app_thread Task. This notification will come from
         * vApplicationIPNetworkEventHook() function, which is a FreeRTOS callback defined by the user. Using
         * this patterns allows to have a synchronous IP stack initialization */
        xTaskNotifyWait(pdFALSE, pdFALSE, NULL, portMAX_DELAY);

        /* Indicate that network is up with a LED on the device */
        NETWORK_CONNECT_INDICATION;

        /* Print IP config on console screen */
        CloudProv_PrintIPConfig();
    }
}
