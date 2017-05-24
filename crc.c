// from http://www.drdobbs.com/implementing-the-ccitt-cyclical-redundan/199904926

/**************************************************************************
//
// crc16.c - generate a ccitt 16 bit cyclic redundancy check (crc)
//
//      The code in this module generates the crc for a block of data.
//
**************************************************************************/
 
/*
//                                16  12  5
// The CCITT CRC 16 polynomial is X + X + X + 1.
// In binary, this is the bit pattern 1 0001 0000 0010 0001, and in hex it
//  is 0x11021.
// A 17 bit register is simulated by testing the MSB before shifting
//  the data, which affords us the luxury of specifiy the polynomial as a
//  16 bit value, 0x1021.
// Due to the way in which we process the CRC, the bits of the polynomial
//  are stored in reverse order. This makes the polynomial 0x8408.
*/

//#define POLY 0x8408
////#define POLY 0x1021
 
/*
// note: when the crc is included in the message, the valid crc is:
//      0xF0B8, before the compliment and byte swap,
//      0x0F47, after compliment, before the byte swap,
//      0x470F, after the compliment and the byte swap.
*/
 
extern  crc_ok;
int     crc_ok = 0x470F;
 
/**************************************************************************
//
// crc16() - generate a 16 bit crc
//
//
// PURPOSE
//      This routine generates the 16 bit remainder of a block of
//      data using the ccitt polynomial generator.
//
// CALLING SEQUENCE
//      crc = crc16(data, len);
//
// PARAMETERS
//      data    <-- address of start of data block
//      len     <-- length of data block
//
// RETURNED VALUE
//      crc16 value. data is calcuated using the 16 bit ccitt polynomial.
//
// NOTES
//      The CRC is preset to all 1's to detect errors involving a loss
//        of leading zero's.
//      The CRC (a 16 bit value) is generated in LSB MSB order.
//      Two ways to verify the integrity of a received message
//        or block of data:
//        1) Calculate the crc on the data, and compare it to the crc
//           calculated previously. The location of the saved crc must be
//           known.
/         2) Append the calculated crc to the end of the data. Now calculate
//           the crc of the data and its crc. If the new crc equals the
//           value in "crc_ok", the data is valid.
//
// PSEUDO CODE:
//      initialize crc (-1)
//      DO WHILE count NE zero
//        DO FOR each bit in the data byte, from LSB to MSB
//          IF (LSB of crc) EOR (LSB of data)
//            crc := (crc / 2) EOR polynomial
//          ELSE
//            crc := (crc / 2)
//          FI
//        OD
//      OD
//      1's compliment and swap bytes in crc
//      RETURN crc
//
**************************************************************************/
/*
unsigned short crc16(data_p, length)
char *data_p;
unsigned short length;
{
       unsigned char i;
       unsigned int data;
       unsigned int crc;
        
       crc = 0xffff;
        
       if (length == 0)
              return (~crc);
        
       do {
              for (i = 0, data = (unsigned int)0xff & *data_p++;
                  i < 8;
                  i++, data >>= 1) {
                    if ((crc & 0x0001) ^ (data & 0x0001))
                           crc = (crc >> 1) ^ POLY;
                    else
                           crc >>= 1;
              }
       } while (--length);
        
       crc = ~crc;
        
       data = crc;
       crc = (crc << 8) | (data >> 8 & 0xFF);
        
       return (crc);
}
*/
/*
	def calc_CRC(self,message):
		#CRC-16-CITT poly, the CRC sheme used by ymodem protocol
	#    poly = 0x11021
		poly = 0x1021
		#16bit operation register, initialized to zeros
	#    reg = 0xFFFF
		reg = 0
		#pad the end of the message with the size of the poly
		message += '\x00\x00' 
		#for each bit in the message
		for byte in message:
			mask = 0x80
			while(mask > 0):
				#left shift by one
				reg<<=1
				#input the next bit from the message into the right hand side of the op reg
				if ord(byte) & mask:   
					reg += 1
				mask>>=1
				#if a one popped out the left of the reg, xor reg w/poly
				if reg > 0xffff:            
					#eliminate any one that popped out the left
					reg &= 0xffff           
					#xor with the poly, this is the remainder
					reg ^= poly
		return reg

*/

// 01-04-6e-04-c2d5
/*
#define POLY 0x1021

unsigned short crc16(data_p, length)
char *data_p;
unsigned short length;
{
       unsigned int data;
       unsigned char mask;
       unsigned long crc;
       
       length +=2; 
       
//       crc = 0xffff;
       crc = 0x0000;

       if (length == 0)
              return (~crc);
        
       do {
			for ( mask = 0x80, data = (unsigned int)0xff & *data_p++; (mask & 0xff) >0 ; mask>>=1 ) {
				crc <<= 1;
				if (data & mask) crc |=1;
				if (crc & 0x10000) {
					crc &= 0xffff;
					crc ^= POLY;
				}
			}

       } while (--length);
       
	return(crc & 0xffff);
}
*/

unsigned short crc16(const unsigned char* data_p, unsigned char length){
    unsigned char x;
//    unsigned short crc = 0xFFFF;
    unsigned short crc = 0;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}
