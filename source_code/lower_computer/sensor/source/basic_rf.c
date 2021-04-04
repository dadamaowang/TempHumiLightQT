/***********************************************************************************
  Filename:     basic_rf.c

  Description:  Basic RF library

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include "hal_int.h"
#include "hal_mcu.h"            // Using halMcuWaitUs()

#include "hal_rf.h"
#ifdef SECURITY_CCM
#include "hal_rf_security.h"
#endif

#include "basic_rf.h"
#ifdef SECURITY_CCM
#include "basic_rf_security.h"
#endif

#include "util.h"               // Using min()
#include "string.h"

/***********************************************************************************
* CONSTANTS AND DEFINES
*/

// Packet and packet part lengths
#define PKT_LEN_MIC                         8
#define PKT_LEN_SEC                         PKT_LEN_UNSEC + PKT_LEN_MIC
#define PKT_LEN_AUTH                        8
#define PKT_LEN_ENCR                        24

// Packet overhead ((frame control field, sequence number, PAN ID,
// destination and source) + (footer))
// Note that the length byte itself is not included included in the packet length
#define BASIC_RF_PACKET_OVERHEAD_SIZE       ((2 + 1 + 2 + 2 + 2) + (2))
#define BASIC_RF_MAX_PAYLOAD_SIZE	        (127 - BASIC_RF_PACKET_OVERHEAD_SIZE - \
    BASIC_RF_AUX_HDR_LENGTH - BASIC_RF_LEN_MIC)
#define BASIC_RF_ACK_PACKET_SIZE	        5
#define BASIC_RF_FOOTER_SIZE                2
#define BASIC_RF_HDR_SIZE                   10

// The time it takes for the acknowledgment packet to be received after the
// data packet has been transmitted.
#define BASIC_RF_ACK_DURATION		        (0.5 * 32 * 2 * ((4 + 1) + (1) + (2 + 1) + (2)))
#define BASIC_RF_SYMBOL_DURATION	        (32 * 0.5)

// The length byte
#define BASIC_RF_PLD_LEN_MASK               0x7F

// Frame control field
#define BASIC_RF_FCF_NOACK                  0x8841
#define BASIC_RF_FCF_ACK                    0x8861
#define BASIC_RF_FCF_ACK_BM                 0x0020
#define BASIC_RF_FCF_BM                     (~BASIC_RF_FCF_ACK_BM)
#define BASIC_RF_SEC_ENABLED_FCF_BM         0x0008

// Frame control field LSB
#define BASIC_RF_FCF_NOACK_L                LO_UINT16(BASIC_RF_FCF_NOACK)
#define BASIC_RF_FCF_ACK_L                  LO_UINT16(BASIC_RF_FCF_ACK)
#define BASIC_RF_FCF_ACK_BM_L               LO_UINT16(BASIC_RF_FCF_ACK_BM)
#define BASIC_RF_FCF_BM_L                   LO_UINT16(BASIC_RF_FCF_BM)
#define BASIC_RF_SEC_ENABLED_FCF_BM_L       LO_UINT16(BASIC_RF_SEC_ENABLED_FCF_BM)

// Auxiliary Security header
#define BASIC_RF_AUX_HDR_LENGTH             5
#define BASIC_RF_LEN_AUTH                   BASIC_RF_PACKET_OVERHEAD_SIZE + \
    BASIC_RF_AUX_HDR_LENGTH - BASIC_RF_FOOTER_SIZE
#define BASIC_RF_SECURITY_M                 2
#define BASIC_RF_LEN_MIC                    8
#ifdef SECURITY_CCM
#undef BASIC_RF_HDR_SIZE
#define BASIC_RF_HDR_SIZE                   15
#endif

// Footer
#define BASIC_RF_CRC_OK_BM                  0x80

/***********************************************************************************
* TYPEDEFS
*/
// The receive struct
typedef struct {
    uint8 seqNumber;
    uint16 srcAddr;
    uint16 srcPanId;
    int8 length;
    uint8* pPayload;
    uint8 ackRequest;
    int8 rssi;
    volatile uint8 isReady;
    uint8 status;
} basicRfRxInfo_t;

// Tx state
typedef struct {
    uint8 txSeqNumber;
    volatile uint8 ackReceived;
    uint8 receiveOn;
    uint32 frameCounter;
} basicRfTxState_t;


// Basic RF packet header (IEEE 802.15.4)
typedef struct {
    uint8   packetLength;
    uint8   fcf0;           // Frame control field LSB
    uint8   fcf1;           // Frame control field MSB
    uint8   seqNumber;
    uint16  panId;
    uint16  destAddr;
    uint16  srcAddr;
    #ifdef SECURITY_CCM
    uint8   securityControl;
    uint8  frameCounter[4];
    #endif
} basicRfPktHdr_t;


/***********************************************************************************
* LOCAL VARIABLES
*/
static basicRfRxInfo_t  rxi=      { 0xFF }; // Make sure sequence numbers are
static basicRfTxState_t txState=  { 0x00 }; // initialised and distinct.

static basicRfCfg_t* pConfig;
static uint8 txMpdu[BASIC_RF_MAX_PAYLOAD_SIZE+BASIC_RF_PACKET_OVERHEAD_SIZE+1];
static uint8 rxMpdu[128];

/***********************************************************************************
* GLOBAL VARIABLES
*/


/***********************************************************************************
* LOCAL FUNCTIONS
*/


/***********************************************************************************
* @fn          basicRfBuildHeader
*
* @brief       Builds packet header according to IEEE 802.15.4 frame format
*
* @param       buffer - Pointer to buffer to write the header
*              destAddr - destination short address
*              payloadLength - length of higher layer payload
*
* @return      uint8 - length of header
*/
static uint8 basicRfBuildHeader(uint8* buffer, uint16 destAddr, uint8 payloadLength)  //��ͷ����
{
    basicRfPktHdr_t *pHdr;
    uint16 fcf;

    pHdr= (basicRfPktHdr_t*)buffer;

    // Populate packet header
    pHdr->packetLength = payloadLength + BASIC_RF_PACKET_OVERHEAD_SIZE;
    //pHdr->frameControlField = pConfig->ackRequest ? BASIC_RF_FCF_ACK : BASIC_RF_FCF_NOACK;
    fcf= pConfig->ackRequest ? BASIC_RF_FCF_ACK : BASIC_RF_FCF_NOACK;
    pHdr->fcf0 = LO_UINT16(fcf);
    pHdr->fcf1 = HI_UINT16(fcf);
    pHdr->seqNumber= txState.txSeqNumber;
    pHdr->panId= pConfig->panId;
    pHdr->destAddr= destAddr;
    pHdr->srcAddr= pConfig->myAddr;

    #ifdef SECURITY_CCM

    // Add security to FCF, length and security header
    pHdr->fcf0 |= BASIC_RF_SEC_ENABLED_FCF_BM_L;
    pHdr->packetLength += PKT_LEN_MIC;
    pHdr->packetLength += BASIC_RF_AUX_HDR_LENGTH;

    pHdr->securityControl= SECURITY_CONTROL;
    pHdr->frameCounter[0]=   LO_UINT16(LO_UINT32(txState.frameCounter));
    pHdr->frameCounter[1]=   HI_UINT16(LO_UINT32(txState.frameCounter));
    pHdr->frameCounter[2]=   LO_UINT16(HI_UINT32(txState.frameCounter));
    pHdr->frameCounter[3]=   HI_UINT16(HI_UINT32(txState.frameCounter));

    #endif

    // Make sure bytefields are network byte order
    UINT16_HTON(pHdr->panId);
    UINT16_HTON(pHdr->destAddr);
    UINT16_HTON(pHdr->srcAddr);

    return BASIC_RF_HDR_SIZE;
}


/***********************************************************************************
* @fn          basicRfBuildMpdu
*
* @brief       Builds mpdu (MAC header + payload) according to IEEE 802.15.4
*              frame format
*
* @param       destAddr - Destination short address
*              pPayload - pointer to buffer with payload
*              payloadLength - length of payload buffer
*
* @return      uint8 - length of mpdu
*/
static uint8 basicRfBuildMpdu(uint16 destAddr, uint8* pPayload, uint8 payloadLength)  //����MAC ���ݰ�ͷ
{
    uint8 hdrLength, n;

    hdrLength = basicRfBuildHeader(txMpdu, destAddr, payloadLength);     //��ͷ���ȼ���

    for(n=0;n<payloadLength;n++)                                          //������Ч����
    {
        txMpdu[hdrLength+n] = pPayload[n];
    }
    return hdrLength + payloadLength; // total mpdu length               //�������ݰ��ܳ���
}


/***********************************************************************************
* @fn          basicRfRxFrmDoneIsr
*
* @brief       Interrupt service routine for received frame from radio
*              (either data or acknowlegdement)
*
* @param       rxi - file scope variable info extracted from the last incoming
*                    frame
*              txState - file scope variable that keeps tx state info
*
* @return      none
*/
static void basicRfRxFrmDoneIsr(void)  //�����ж�
{
    basicRfPktHdr_t *pHdr;
    uint8 *pStatusWord;
    #ifdef SECURITY_CCM
    uint8 authStatus=0;
    #endif

    // Map header to packet buffer
    pHdr= (basicRfPktHdr_t*)rxMpdu;

    // Clear interrupt and disable new RX frame done interrupt
    halRfDisableRxInterrupt();           //�ر�RF�����ж�

    // Enable all other interrupt sources (enables interrupt nesting)
    halIntOn();                         //ʹ��ȫ���ж�

    // Read payload length.
    halRfReadRxBuf(&pHdr->packetLength,1);   //��ȡ��Ч�غɵĳ���
    pHdr->packetLength &= BASIC_RF_PLD_LEN_MASK; // Ignore MSB  //ȥ�����λ
    
    // Is this an acknowledgment packet?
    // Only ack packets may be 5 bytes in total.
    if (pHdr->packetLength == BASIC_RF_ACK_PACKET_SIZE)  //�жϳ����Ƿ���ȷ����ȷ���ȡ����
    {

        // Read the packet
        halRfReadRxBuf(&rxMpdu[1], pHdr->packetLength);  //��ȡ��Ч����

        // Make sure byte fields are changed from network to host byte order
        
    	UINT16_NTOH(pHdr->panId);      //����Ϊ�������ֽ�ת��Ϊ�����ֽ�
    	UINT16_NTOH(pHdr->destAddr);
    	UINT16_NTOH(pHdr->srcAddr);
        #ifdef SECURITY_CCM
        UINT32_NTOH(pHdr->frameCounter);
        #endif

        rxi.ackRequest = !!(pHdr->fcf0 & BASIC_RF_FCF_ACK_BM_L);  //ACKȷ��

        // Read the status word and check for CRC OK
        pStatusWord= rxMpdu + 4;

        // Indicate the successful ACK reception if CRC and sequence number OK
        if ((pStatusWord[1] & BASIC_RF_CRC_OK_BM) && (pHdr->seqNumber == txState.txSeqNumber)) 
        {
            txState.ackReceived = TRUE;
        }

        // No, it is data
    }
    else 
    {

        // It is assumed that the radio rejects packets with invalid length.
        // Subtract the number of bytes in the frame overhead to get actual payload.

        rxi.length = pHdr->packetLength - BASIC_RF_PACKET_OVERHEAD_SIZE;

        #ifdef SECURITY_CCM
        rxi.length -= (BASIC_RF_AUX_HDR_LENGTH + BASIC_RF_LEN_MIC);
        authStatus = halRfReadRxBufSecure(&rxMpdu[1], pHdr->packetLength, rxi.length,
                                        BASIC_RF_LEN_AUTH, BASIC_RF_SECURITY_M);
        #else
        halRfReadRxBuf(&rxMpdu[1], pHdr->packetLength);
        #endif

        // Make sure byte fields are changed from network to host byte order
    	UINT16_NTOH(pHdr->panId);
    	UINT16_NTOH(pHdr->destAddr);
    	UINT16_NTOH(pHdr->srcAddr);
        #ifdef SECURITY_CCM
        UINT32_NTOH(pHdr->frameCounter);
        #endif

        rxi.ackRequest = !!(pHdr->fcf0 & BASIC_RF_FCF_ACK_BM_L);

        // Read the source address
        rxi.srcAddr= pHdr->srcAddr;

        // Read the packet payload
        rxi.pPayload = rxMpdu + BASIC_RF_HDR_SIZE;

        // Read the FCS to get the RSSI and CRC
        pStatusWord= rxi.pPayload+rxi.length;
        #ifdef SECURITY_CCM
        pStatusWord+= BASIC_RF_LEN_MIC;
        #endif
        rxi.rssi = pStatusWord[0];

        // Notify the application about the received data packet if the CRC is OK
        // Throw packet if the previous packet had the same sequence number
        if( (pStatusWord[1] & BASIC_RF_CRC_OK_BM) && (rxi.seqNumber != pHdr->seqNumber) ) {
            // If security is used check also that authentication passed
            #ifdef SECURITY_CCM
            if( authStatus==SUCCESS ) {
                if ( (pHdr->fcf0 & BASIC_RF_FCF_BM_L) ==
                    (BASIC_RF_FCF_NOACK_L | BASIC_RF_SEC_ENABLED_FCF_BM_L)) {
                        rxi.isReady = TRUE;
                }
            }
            #else
            if ( ((pHdr->fcf0 & (BASIC_RF_FCF_BM_L)) == BASIC_RF_FCF_NOACK_L) ) {
                rxi.isReady = TRUE;
            }              
            #endif
        }
        rxi.seqNumber = pHdr->seqNumber;
    }
  
    // Enable RX frame done interrupt again
    halIntOff();
    halRfEnableRxInterrupt();
}


/***********************************************************************************
* GLOBAL FUNCTIONS
*/

/***********************************************************************************
* @fn          basicRfInit
*
* @brief       Initialise basic RF datastructures. Sets channel, short address and
*              PAN id in the chip and configures interrupt on packet reception
*
* @param       pRfConfig - pointer to BASIC_RF_CONFIG struct.
*                          This struct must be allocated by higher layer
*              txState - file scope variable that keeps tx state info
*              rxi - file scope variable info extracted from the last incoming
*                    frame
*
* @return      none
*/
uint8 basicRfInit(basicRfCfg_t* pRfConfig)  //��ʼ��RF��һЩ��������
{
    if (halRfInit()==FAILED)                //�ж�RF��Ӳ�������Ƿ�ɹ�
        return FAILED;

    halIntOff();                           //�ر�ȫ���ж�

    // Set the protocol configuration
    pConfig = pRfConfig;                   //ȡ������Ϣ
    
    rxi.pPayload   = NULL;                  //�������ݰ���Ч����ָ���ʼ��

    txState.receiveOn = TRUE;               //����״̬
    txState.frameCounter = 0;               //���ͼ�������

    // Set channel
    halRfSetChannel(pConfig->channel);     //�����ŵ�

    // Write the short address and the PAN ID to the CC2530 RAM
    halRfSetShortAddr(pConfig->myAddr);  //д��̵�ַ
    halRfSetPanId(pConfig->panId);       //д��PANID

    // if security is enabled, write key and nonce
    #ifdef SECURITY_CCM                   //��ȫʹ�� ��û��ѡ��ñ���ѡ�
    basicRfSecurityInit(pConfig);
    #endif

    // Set up receive interrupt (received data or acknowlegment)
    halRfRxInterruptConfig(basicRfRxFrmDoneIsr);  //���ý����ж�

    halIntOn();                            //ȫ���ж�ʹ��

    return SUCCESS;
}


/***********************************************************************************
* @fn          basicRfSendPacket
*
* @brief       Send packet
*
* @param       destAddr - destination short address
*              pPayload - pointer to payload buffer. This buffer must be
*                         allocated by higher layer.
*              length - length of payload
*              txState - file scope variable that keeps tx state info
*              mpdu - file scope variable. Buffer for the frame to send
*
* @return      basicRFStatus_t - SUCCESS or FAILED
*/
uint8 basicRfSendPacket(uint16 destAddr, uint8* pPayload, uint8 length)  //���ݰ����ͺ���
{
    uint8 mpduLength;
    uint8 status;

    // Turn on receiver if its not on
    if(!txState.receiveOn)   //������չر� ���򿪽��չ���
    {
        halRfReceiveOn();
    }

    // Check packet length
    length = min(length, BASIC_RF_MAX_PAYLOAD_SIZE);  //������ݰ������Ƿ�ϸ�

    // Wait until the transceiver is idle
    halRfWaitTransceiverReady();                    //�ȴ�����״̬

    // Turn off RX frame done interrupt to avoid interference on the SPI interface
    halRfDisableRxInterrupt();  //�رս����ж�

    mpduLength = basicRfBuildMpdu(destAddr, pPayload, length);  //���ݰ���������MAC ��ͷ�ȣ� �õ����ݰ����ܳ���

    #ifdef SECURITY_CCM   //��ȫ���� ��û��ѡ��ñ���ѡ�
    halRfWriteTxBufSecure(txMpdu, mpduLength, length, BASIC_RF_LEN_AUTH, BASIC_RF_SECURITY_M);
    txState.frameCounter++;     // Increment frame counter field
    #else
    halRfWriteTxBuf(txMpdu, mpduLength);   //�����ݰ�������д�뵽RFD�Ĵ�����
    #endif

    // Turn on RX frame done interrupt for ACK reception
    halRfEnableRxInterrupt();             //�򿪽����ж�

    // Send frame with CCA. return FAILED if not successful
    if(halRfTransmit() != SUCCESS)      //�ȴ��������
    {
        status = FAILED;
    }

    // Wait for the acknowledge to be received, if any
    if (pConfig->ackRequest)        //���Ҫ����ACK�����ж��Ƿ��յ�ACK
    {
        txState.ackReceived = FALSE;

        // We'll enter RX automatically, so just wait until we can be sure that the ack reception should have finished
        // The timeout consists of a 12-symbol turnaround time, the ack packet duration, and a small margin
        halMcuWaitUs((12 * BASIC_RF_SYMBOL_DURATION) + (BASIC_RF_ACK_DURATION) + (2 * BASIC_RF_SYMBOL_DURATION) + 10);

        // If an acknowledgment has been received (by RxFrmDoneIsr), the ackReceived flag should be set
        status = txState.ackReceived ? SUCCESS : FAILED;

    } 
    else 
    {
        status = SUCCESS;
    }

    // Turn off the receiver if it should not continue to be enabled
    if (!txState.receiveOn) 
    {
        halRfReceiveOff();  //�رս��չ���
    }

    if(status == SUCCESS) {
        txState.txSeqNumber++;
    }

#ifdef SECURITY_CCM
    halRfIncNonceTx();          // Increment nonce value
#endif

    return status;

}


/***********************************************************************************
* @fn          basicRfPacketIsReady
*
* @brief       Check if a new packet is ready to be read by next higher layer
*
* @param       none
*
* @return      uint8 - TRUE if a packet is ready to be read by higher layer
*/
uint8 basicRfPacketIsReady(void)
{
    return rxi.isReady;
}


/**********************************************************************************
* @fn          basicRfReceive
*
* @brief       Copies the payload of the last incoming packet into a buffer
*
* @param       pRxData - pointer to data buffer to fill. This buffer must be
*                        allocated by higher layer.
*              len - Number of bytes to read in to buffer
*              rxi - file scope variable holding the information of the last
*                    incoming packet
*
* @return      uint8 - number of bytes actually copied into buffer
*/
uint8 basicRfReceive(uint8* pRxData, uint8 len, int16* pRssi)  //���������յ������ݰ�
{
    // Accessing shared variables -> this is a critical region
    // Critical region start
    halIntOff();                                               //�ر������ж�
    memcpy(pRxData, rxi.pPayload, min(rxi.length, len));       //������Ч����
    if(pRssi != NULL)                                          //RSSI����                        
    {
        if(rxi.rssi < 128)
        {
            *pRssi = rxi.rssi - halRfGetRssiOffset();          //CC2530 RSSI �������㣨73��
        }
        else
        {
            *pRssi = (rxi.rssi - 256) - halRfGetRssiOffset();
        }
    }
    rxi.isReady = FALSE;                                       //����ձ�־
    halIntOn();                                                //���ж�

    // Critical region end

    return min(rxi.length, len);
}


/**********************************************************************************
* @fn          basicRfGetRssi
*
* @brief       Copies the payload of the last incoming packet into a buffer
*
* @param       none

* @return      int8 - RSSI value
*/
int8 basicRfGetRssi(void)
{
    if(rxi.rssi < 128){
        return rxi.rssi - halRfGetRssiOffset();
    }
    else{
        return (rxi.rssi - 256) - halRfGetRssiOffset();
    }
}

/***********************************************************************************
* @fn          basicRfReceiveOn
*
* @brief       Turns on receiver on radio
*
* @param       txState - file scope variable
*
* @return      none
*/
void basicRfReceiveOn(void)  //��RF�Ľ��չ���
{
    txState.receiveOn = TRUE;
    halRfReceiveOn();
}


/***********************************************************************************
* @fn          basicRfReceiveOff
*
* @brief       Turns off receiver on radio
*
* @param       txState - file scope variable
*
* @return      none
*/
void basicRfReceiveOff(void)  //�ر�RF�Ľ��չ���
{
    txState.receiveOn = FALSE;
    halRfReceiveOff();
}



