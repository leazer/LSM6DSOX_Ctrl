#define TRUE  1
#define FALSE 0
#define bool BYTE

#include "main.h"

#include "diskio.h"
#include "fatfs_sd.h"

extern SPI_HandleTypeDef hspi2; 
extern volatile uint8_t Timer1, Timer2;                 /* 10ms timeout set */

static volatile DSTATUS Stat = STA_NOINIT;              /* SD Status Flag*/
static uint8_t CardType;                                /* SD Type 0:MMC, 1:SDC, 2:Block addressing */
static uint8_t PowerFlag = 0;                           /* Power Type Flag */

/* SPI Chip Select */
static void SELECT(void)
{
  HAL_GPIO_WritePin(TF_CS_GPIO_Port, TF_CS_Pin, GPIO_PIN_RESET);
}

/* SPI Chip Deselect */
static void DESELECT(void)
{
  HAL_GPIO_WritePin(TF_CS_GPIO_Port, TF_CS_Pin, GPIO_PIN_SET);
}

/* SPI Transmit */
static void SPI_TxByte(BYTE data)
{
  while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY);
  HAL_SPI_Transmit(&hspi2, &data, 1, SPI_TIMEOUT);
}

/* SPI Receive */
static uint8_t SPI_RxByte(void)
{
  uint8_t dummy, data;
  dummy = 0xFF;
  data = 0;
  
  while ((HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY));
  HAL_SPI_TransmitReceive(&hspi2, &dummy, &data, 1, SPI_TIMEOUT);
  
  return data;
}

/* SPI Receive to ptr */
static void SPI_RxBytePtr(uint8_t *buff) 
{
  *buff = SPI_RxByte();
}

/* SD Ready wait */
static uint8_t SD_ReadyWait(void) 
{
  uint8_t res;
  
  /* 500ms timeout */
  Timer2 = 50;
  SPI_RxByte();
  
  do
  {
    /* Until SPI Recieve 0xFF or Timeout */
    res = SPI_RxByte();
  } while ((res != 0xFF) && Timer2);
  
  return res;
}

/* Power on the TF-Card */
static void SD_PowerOn(void) 
{
  uint8_t cmd_arg[6];
  uint32_t Count = 0x1FFF;
  
  /* Transfer the SPI message in Deselect state to  Set TF-Card standby. */
  DESELECT();
  
  for(int i = 0; i < 10; i++)
  {
    SPI_TxByte(0xFF);
  }
  
  /* SPI Chips Select */
  SELECT();
  
  /* Initial GO_IDLE_STATE */
  cmd_arg[0] = (CMD0 | 0x40);
  cmd_arg[1] = 0;
  cmd_arg[2] = 0;
  cmd_arg[3] = 0;
  cmd_arg[4] = 0;
  cmd_arg[5] = 0x95;
  
  /* transmit the cmd. */
  for (int i = 0; i < 6; i++)
  {
    SPI_TxByte(cmd_arg[i]);
  }
  
  /* response */
  while ((SPI_RxByte() != 0x01) && Count)
  {
    Count--;
  }
  
  DESELECT();
  SPI_TxByte(0XFF);
  
  PowerFlag = 1;
}

/* Power off the TF-Card */
static void SD_PowerOff(void) 
{
  PowerFlag = 0;
}

/* Confirm power status */
static uint8_t SD_CheckPower(void) 
{
  /*  0=off, 1=on */
  return PowerFlag;
}

/* Receiving message packet */
static bool SD_RxDataBlock(BYTE *buff, UINT btr) 
{
  uint8_t token;
  
  /* 100ms timeout */
  Timer1 = 10;
  
  /* response */		
  do 
  {    
    token = SPI_RxByte();
  } while((token == 0xFF) && Timer1);
  
  /* token is not 0xFE ,return error */
  if(token != 0xFE)
    return FALSE;
  
  /* Receive data in the buffer*/
  do 
  {     
    SPI_RxBytePtr(buff++);
    SPI_RxBytePtr(buff++);
  } while(btr -= 2);
  
  SPI_RxByte(); /* ignore CRC */
  SPI_RxByte();
  
  return TRUE;
}

/* Transmission message packet */
#if _READONLY == 0
static bool SD_TxDataBlock(const BYTE *buff, BYTE token)
{
  uint8_t resp, wc;
  uint8_t i = 0;
    
  /* The TF-Card is ready*/
  if (SD_ReadyWait() != 0xFF)
    return FALSE;
  
  /* Transmit cmd */
  SPI_TxByte(token);      
  
  /* cmd is not 0xFD */
  if (token != 0xFD) 
  { 
    wc = 0;
    
    /*transmit 512 bypes data */
    do 
    { 
      SPI_TxByte(*buff++);
      SPI_TxByte(*buff++);
    } while (--wc);
    
    SPI_RxByte();       /*ignore CRC */
    SPI_RxByte();
    
    /* Receive response*/        
    while (i <= 64) 
    {			
      resp = SPI_RxByte();
      
      /* Error processing  */
      if ((resp & 0x1F) == 0x05) 
        break;
      
      i++;
    }
    
    /* SPI Receive buffer Clear */
    while (SPI_RxByte() == 0);
  }
  
  if ((resp & 0x1F) == 0x05)
    return TRUE;
  else
    return FALSE;
}
#endif /* _READONLY */

/* CMD packet transfer */
static BYTE SD_SendCmd(BYTE cmd, DWORD arg) 
{
  uint8_t crc, res;
  
  /* TF-Card wait */
  if (SD_ReadyWait() != 0xFF)
    return 0xFF;
  
  /* CMD packet transfer */
  SPI_TxByte(cmd); 			/* Command */
  SPI_TxByte((BYTE) (arg >> 24)); 	/* Argument[31..24] */
  SPI_TxByte((BYTE) (arg >> 16)); 	/* Argument[23..16] */
  SPI_TxByte((BYTE) (arg >> 8)); 	/* Argument[15..8] */
  SPI_TxByte((BYTE) arg); 		/* Argument[7..0] */
  
  /* CRC check by cmd.*/
  crc = 0;  
  if (cmd == CMD0)
    crc = 0x95; /* CRC for CMD0(0) */
  
  if (cmd == CMD8)
    crc = 0x87; /* CRC for CMD8(0x1AA) */
  
  /* CRC transfer */
  SPI_TxByte(crc);
  
  /*The CRC deletes a response byte when it transmits the CMD12 Stop Reading command*/
  if (cmd == CMD12)
    SPI_RxByte();
  
  /* Receive normal data within 10 times. */
  uint8_t n = 10; 
  do
  {
    res = SPI_RxByte();
  } while ((res & 0x80) && --n);
  
  return res;
}

/*-----------------------------------------------------------------------
  fatfs Global function.
  using in user_diskio.c.
-----------------------------------------------------------------------*/

/* Init TF-Card */
DSTATUS SD_disk_initialize(BYTE drv) 
{
  uint8_t n, type, ocr[4];
  
  /* Only one drive is supported*/
  if(drv)
    return STA_NOINIT;  
  
  /* TF-card not inserted */
  if(Stat & STA_NODISK)
    return Stat;        
  
  /* TF-card Power On */
  SD_PowerOn();         
  
  /* TF-card Chip Select */
  SELECT();             
  
  /* Reset the TF-Card type */
  type = 0;
  
  /* Enter Idle status */
  if (SD_SendCmd(CMD0, 0) == 1) 
  { 
    /* Set timeout 1 secend  */
    Timer1 = 100;
    
    /* Confirm TF-Card  */
    if (SD_SendCmd(CMD8, 0x1AA) == 1) 
    { 
      /* SDC Ver2+ */
      for (n = 0; n < 4; n++)
      {
        ocr[n] = SPI_RxByte();
      }
      
      if (ocr[2] == 0x01 && ocr[3] == 0xAA) 
      { 
        /* 2.7-3.6V Voltage range */
        do {
          if (SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 1UL << 30) == 0)
            break; /* ACMD41 with HCS bit */
        } while (Timer1);
        
        if (Timer1 && SD_SendCmd(CMD58, 0) == 0) 
        { 
          /* Check CCS bit */
          for (n = 0; n < 4; n++)
          {
            ocr[n] = SPI_RxByte();
          }
          
          type = (ocr[0] & 0x40) ? 6 : 2;
        }
      }
    } 
    else 
    { 
      /* SDC Ver1 or MMC */
      type = (SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 0) <= 1) ? 2 : 1; /* SDC : MMC */
      
      do {
        if (type == 2) 
        {
          if (SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 0) == 0)
            break; /* ACMD41 */
        } 
        else 
        {
          if (SD_SendCmd(CMD1, 0) == 0)
            break; /* CMD1 */
        }
      } while (Timer1);
      
      if (!Timer1 || SD_SendCmd(CMD16, 512) != 0) 
      {
        /* Select block length */
        type = 0;
      }
    }
  }
  
  CardType = type;
  
  DESELECT();
  
  SPI_RxByte(); /* Switch to idle state (Release DO) */
  
  if (type) 
  {
    /* Clear STA_NOINIT */
    Stat &= ~STA_NOINIT; 
  }
  else
  {
    /* Initialization failed */
    SD_PowerOff();
  }
  
  return Stat;
}

/* Check disk status */
DSTATUS SD_disk_status(BYTE drv) 
{
  if (drv)
    return STA_NOINIT; 
  
  return Stat;
}

/* Read the sector */
DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) 
{
  if (pdrv || !count)
    return RES_PARERR;
  
  if (Stat & STA_NOINIT)
    return RES_NOTRDY;
  
  if (!(CardType & 4))
    sector *= 512;      /* Changes the specified sector to the Byaddress unit */
  
  SELECT();
  
  if (count == 1) 
  { 
    /* Single block read */
    if ((SD_SendCmd(CMD17, sector) == 0) && SD_RxDataBlock(buff, 512))
      count = 0;
  } 
  else 
  { 
    /* Multi block read  */
    if (SD_SendCmd(CMD18, sector) == 0) 
    {       
      do {
        if (!SD_RxDataBlock(buff, 512))
          break;
        
        buff += 512;
      } while (--count);
      
      /* STOP TRANSMISION. After all blocks have been read, request to STOP transmitting */
      SD_SendCmd(CMD12, 0); 
    }
  }
  
  DESELECT();
  SPI_RxByte(); /* Idle status(Release DO) */
  
  return count ? RES_ERROR : RES_OK;
}

/* Write sectors */
#if _READONLY == 0
DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count) 
{
  if (pdrv || !count)
    return RES_PARERR;
  
  if (Stat & STA_NOINIT)
    return RES_NOTRDY;
  
  if (Stat & STA_PROTECT)
    return RES_WRPRT;
  
  if (!(CardType & 4))
    sector *= 512; /* Changes the specified sector to the Byaddress unit */
  
  SELECT();
  
  if (count == 1) 
  { 
    /* Single block write*/
    if ((SD_SendCmd(CMD24, sector) == 0) && SD_TxDataBlock(buff, 0xFE))
      count = 0;
  } 
  else 
  { 
    /* Multi block write*/
    if (CardType & 2) 
    {
      SD_SendCmd(CMD55, 0);
      SD_SendCmd(CMD23, count); /* ACMD23 */
    }
    
    if (SD_SendCmd(CMD25, sector) == 0) 
    {       
      do {
        if(!SD_TxDataBlock(buff, 0xFC))
          break;
        
        buff += 512;
      } while (--count);
      
      if(!SD_TxDataBlock(0, 0xFD))
      {        
        count = 1;
      }
    }
  }
  
  DESELECT();
  SPI_RxByte();
  
  return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY */

/* Other function*/
DRESULT SD_disk_ioctl(BYTE drv, BYTE ctrl, void *buff) 
{
  DRESULT res;
  BYTE n, csd[16], *ptr = buff;
  WORD csize;
  
  if (drv)
    return RES_PARERR;
  
  res = RES_ERROR;
  
  if (ctrl == CTRL_POWER) 
  {
    switch (*ptr) 
    {
    case 0:
      if (SD_CheckPower())
        SD_PowerOff();          /* Power Off */
      res = RES_OK;
      break;
    case 1:
      SD_PowerOn();             /* Power On */
      res = RES_OK;
      break;
    case 2:
      *(ptr + 1) = (BYTE) SD_CheckPower();
      res = RES_OK;             /* Power Check */
      break;
    default:
      res = RES_PARERR;
    }
  } 
  else 
  {
    if (Stat & STA_NOINIT)
      return RES_NOTRDY;
    
    SELECT();
    
    switch (ctrl) 
    {
    case GET_SECTOR_COUNT: 
      /* The number of sectors in an SD card (DWORD) */
      if ((SD_SendCmd(CMD9, 0) == 0) && SD_RxDataBlock(csd, 16)) 
      {
        if ((csd[0] >> 6) == 1) 
        { 
          /* SDC ver 2.00 */
          csize = csd[9] + ((WORD) csd[8] << 8) + 1;
          *(DWORD*) buff = (DWORD) csize << 10;
        } 
        else 
        { 
          /* MMC or SDC ver 1.XX */
          n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
          csize = (csd[8] >> 6) + ((WORD) csd[7] << 2) + ((WORD) (csd[6] & 3) << 10) + 1;
          *(DWORD*) buff = (DWORD) csize << (n - 9);
        }
        
        res = RES_OK;
      }
      break;
      
    case GET_SECTOR_SIZE: 
      /* Unit size of Sector (WORD) */
      *(WORD*) buff = 512;
      res = RES_OK;
      break;
      
    case CTRL_SYNC: 
      /* Synchronous write */
      if (SD_ReadyWait() == 0xFF)
        res = RES_OK;
      break;
      
    case MMC_GET_CSD: 
      /* Receive CSD information (16 bytes) */
      if (SD_SendCmd(CMD9, 0) == 0 && SD_RxDataBlock(ptr, 16))
        res = RES_OK;
      break;
      
    case MMC_GET_CID: 
      /* Receive CID information (16 bytes) */
      if (SD_SendCmd(CMD10, 0) == 0 && SD_RxDataBlock(ptr, 16))
        res = RES_OK;
      break;
      
    case MMC_GET_OCR: 
      /* Receive OCR information (4 bytes) */
      if (SD_SendCmd(CMD58, 0) == 0) 
      {         
        for (n = 0; n < 4; n++)
        {
          *ptr++ = SPI_RxByte();
        }
        
        res = RES_OK;
      }     
      
    default:
      res = RES_PARERR;
    }
    
    DESELECT();
    SPI_RxByte();
  }
  
  return res;
}
