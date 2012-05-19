#include <stddef.h>
#include "1wire.h"
#include "m128_hal.h"

// global search state
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t LastDeviceFlag;
uint8_t crc8_var;
uint8_t ROM_NO[OW_ROM_BYTE_LEN];

static uint8_t ow_read_bit(void);
static void ow_write_bit(uint8_t bitval);
static void ow_write_byte(uint8_t val);
static uint8_t ow_read_byte(void);
static uint8_t crc8(uint8_t *data_in, uint8_t size);
static uint8_t ow_search(void);
static uint8_t ow_first();
static uint8_t ow_next();
static uint8_t ow_reset(void);
static uint16_t _round(uint16_t _x);
static uint8_t docrc8(uint8_t value);
static uint8_t dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
};

ow_ret_val_t ow_read_temperature(ow_device_t *ow_device, ow_temp_t *ow_temp)
{
    if (ow_device == NULL || ow_temp == NULL) {
        return OW_RET_FAIL;
    }


    ow_reset();
    ow_write_byte(OW_CMD55_MATCH_ROM);

    for (int8_t i=0; i<OW_ROM_BYTE_LEN; i++) {
        ow_write_byte(ow_device->addr[i]);
    }

    ow_write_byte(OW_CMD44_CONV_TEMP);
    while(ow_read_byte() == 0)
        ;      

    ow_scratchpad_t scrpad;

    if (ow_read_scratchpad(ow_device, &scrpad) == OW_RET_OK) {
        ow_temp->temp = ((scrpad.data[1]&0x7) << 4) & 0x7f;
        ow_temp->temp |= (scrpad.data[0] & 0xF0) >> 4;
    //    printk("%u %u %u \n",scrpad.data[1],scrpad.data[0], scrpad.data[0] & 0x0F);
        ow_temp->dec = _round((scrpad.data[0] & 0x0F) * THERM_DECIMAL_STEPS_12BIT);
        return OW_RET_OK;
    } 
    return OW_RET_FAIL;
}   

ow_ret_val_t ow_get_devices(ow_device_t *ow_devices)
{
    uint8_t rslt,cnt;
    if (ow_devices == NULL) {
        return OW_RET_FAIL;
    }

    cnt = 0;
    rslt = ow_first();
    while (rslt)
    {
        for(int8_t i=7; i >= 0; i--) {
            ow_devices[cnt].addr[i] = ROM_NO[i];
        }
        ++cnt;
        rslt = ow_next();
    }

    return OW_RET_OK;
}

uint8_t ow_num_devices()
{
    uint8_t rslt,cnt;
    cnt = 0;
    rslt = ow_first();
    while (rslt)
    {
        ++cnt;
        rslt = ow_next();
    }

    return cnt;
}

ow_ret_val_t ow_read_scratchpad(ow_device_t *ow_device, ow_scratchpad_t *scratchpad)
{
    ow_reset();
    ow_write_byte(OW_CMD55_MATCH_ROM);
    for (int8_t i=0; i<OW_ROM_BYTE_LEN; i++) {
        ow_write_byte(ow_device->addr[i]);
    }
    ow_write_byte(OW_CMDBE_READ_SCRATCHPAD);

    for (uint8_t i = 0; i < sizeof(ow_scratchpad_t); i++) {
        scratchpad->data[i] = ow_read_byte();
    }

    if (crc8(scratchpad->data, 8) != scratchpad->data[8]) {
        return OW_RET_FAIL;
    }
    
    return OW_RET_OK;
}

static uint8_t docrc8(uint8_t value)
{
   crc8_var = dscrc_table[crc8_var ^ value];
   return crc8_var;
}

static uint8_t ow_first()
{
   // reset the search state
   LastDiscrepancy = 0;
   LastDeviceFlag = FALSE;
   LastFamilyDiscrepancy = 0;

   return ow_search();
}
//--------------------------------------------------------------------------
// Find the 'next' devices on the 1-Wire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
static uint8_t ow_next()
{
   return ow_search();
}

//--------------------------------------------------------------------------
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
// search state.
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
static uint8_t ow_search(void)
{
   uint8_t last_zero, rom_byte_number, search_result;
   uint8_t id_bit, id_bit_number, cmp_id_bit;
   uint8_t rom_byte_mask, search_direction;

   // initialize for search
   id_bit_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = 0;
   crc8_var = 0;

   // if the last call was not the last one
   if (!LastDeviceFlag)
   {
      // 1-Wire reset
      if (!ow_reset())
      {
         // reset the search
         LastDiscrepancy = 0;
         LastDeviceFlag = FALSE;
         LastFamilyDiscrepancy = 0;
         return FALSE;
      }

      // issue the search command 
      ow_write_byte(OW_CMDF0_SEARCH_ROM);  
        _delay_us(100);
      // loop to do the search
      do
      {
         // read a bit and its complement
         id_bit = ow_read_bit();
         cmp_id_bit = ow_read_bit();

         // check for no devices on 1-wire
         if ((id_bit == 1) && (cmp_id_bit == 1)) {
            break;
         }
         else
         {
            // all devices coupled have 0 or 1
            if (id_bit != cmp_id_bit) {
               search_direction = id_bit;  // bit write value for search
            }
            else
            {
               // if this discrepancy if before the Last Discrepancy
               // on a previous next then pick the same as last time
               if (id_bit_number < LastDiscrepancy) {
                  search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
               } else {
                  // if equal to last pick 1, if not then pick 0
                  search_direction = (id_bit_number == LastDiscrepancy);
               }
               // if 0 was picked then record its position in LastZero
               if (search_direction == 0)
               {
                  last_zero = id_bit_number;

                  // check for Last discrepancy in family
                  if (last_zero < 9)
                     LastFamilyDiscrepancy = last_zero;
               }
            }

            // set or clear the bit in the ROM byte rom_byte_number
            // with mask rom_byte_mask
            if (search_direction == 1) {
              ROM_NO[rom_byte_number] |= rom_byte_mask;
            }
            else {
              ROM_NO[rom_byte_number] &= ~rom_byte_mask;
            }

            // serial number search direction write bit
            ow_write_bit(search_direction);

            // increment the byte counter id_bit_number
            // and shift the mask rom_byte_mask
            id_bit_number++;
            rom_byte_mask <<= 1;

            /* if the mask is 0 then go to new SerialNum byte rom_byte_number
             * and reset mask
             */
            if (rom_byte_mask == 0)
            {
                docrc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
                rom_byte_number++;
                rom_byte_mask = 1;
            }
         }
      }
      while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

      // if the search was successful then
      if (!((id_bit_number <= OW_ROM_BIT_LEN) || (crc8_var != 0))) {
         // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
         LastDiscrepancy = last_zero;

         // check for last device
         if (LastDiscrepancy == 0)
            LastDeviceFlag = TRUE;
         
         search_result = TRUE;
      }
   }
   // if no device found then reset counters so next 'search' will be like a first
   if (!search_result || !ROM_NO[0]) {
      LastDiscrepancy = 0;
      LastDeviceFlag = FALSE;
      LastFamilyDiscrepancy = 0;
      search_result = FALSE;
   }

   return search_result;
}

static uint8_t ow_reset(void)
{
    uint8_t ret_val = 0;
    DQ_DDR = (1<<DQ_PIN);

    DQ_LOW();
    _delay_us(480);
    DQ_HIGH();
    DQ_DDR = 0;
    _delay_us(60);
    if ((DQ_PIN & DQ_MASK) == 0) {
        ret_val = 1;
    }
    _delay_us(240); 
    return ret_val;
}

static void ow_write_bit(uint8_t bitval)
{
    DQ_DDR = 0xff;
    DQ_LOW();
    if(bitval==1) {
        _delay_us(2);
        DQ_HIGH();
    }
    _delay_us(60);
    DQ_HIGH();
}

static void ow_write_byte(uint8_t val)
{
    _delay_us(1);
    uint8_t i;
    uint8_t tmp;
    for (i=0; i<8; i++) {
        tmp = val>>i;
        tmp &= 0x01;
        ow_write_bit(tmp);
    }
   _delay_us(100);
}

static uint8_t ow_read_bit(void)
{
    uint8_t val;
    _delay_us(1);
    DQ_DDR = 0xff;
    DQ_LOW();
    _delay_us(1);
    DQ_DDR = 0x00;
    DQ_PORT = 0x00;
    _delay_us(15);
    val = (DQ_IN & DQ_MASK);
    _delay_us(45);
    return (val>>DQ_PIN); 
}

static uint8_t ow_read_byte(void)
{
    uint8_t value = 0;
    for (uint8_t i=0; i<8; i++) {
        if (ow_read_bit()) {
             value|=0x01<<i;
        } 
        _delay_us(60);
    }
    return(value);
}

static uint8_t crc8(uint8_t *data_in, uint8_t size)
{
    uint8_t  crc, bit_counter, data, feedback_bit;
    crc = CRC8INIT;
    for (uint8_t i = 0; i != size; i++) {
        data = data_in[i];

        bit_counter = 8;
        do {
            feedback_bit = (crc ^ data) & 0x01;
            if ( feedback_bit == 0x01 ) {
                crc = crc ^ CRC8POLY;
            }
            crc = (crc >> 1) & 0x7F;
            if ( feedback_bit == 0x01 ) {
                crc = crc | 0x80;
            }
            data = data >> 1;
            bit_counter--;
        } while (bit_counter > 0);
    }   
    return crc;
}

static uint16_t _round(uint16_t _x)
{
    uint16_t diff = _x;
    if (_x >= 100) {
        if (_x >= 10000) {
            _x /= 100;
        } 
        if (_x < 1000) {
            _x *= 10;
        }
        diff = _x / 100;
        if (_x % 100 >= 50) {
            diff += 1;
        }
    } 
    if (_x < 10) {
        diff *= 10;
    }
    
    return diff;
}
