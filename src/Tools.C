#include <cstdint>
#include <cstdlib>
#include <string>
#include "Tools.h"

/** 
 * builds a 64-bit long out of an array of 8 bytes
 *
 * @param array of 8 bytes
 * @return uint64_t where the low order byte is bytes[0] and
 *         the high order byte is bytes[7]
 *
 * RULES: 
 *   1) no more than ten lines of code
 *   2) use a loop
*/
uint64_t Tools::buildLong(uint8_t bytes[LONGSIZE])
{
	uint64_t result = 0;
	for (int i = LONGSIZE-1; i >= 0; i--){
		result = (result << 8) | bytes[i];	
	}	
	return result;
}

/** 
 * accepts as input an uint64_t and returns the designated byte
 * within the uint64_t; returns 0 if the indicated byte number
 * is out of range 
 *
 * for example, getByte(0x1122334455667788, 7) returns 0x11
 *              getByte(0x1122334455667788, 1) returns 0x77
 *              getByte(0x1122334455667788, 8) returns 0
 *
 * @param uint64_t source that is the source data
 * @param int32_t byteNum that indicates the byte to return (0 through 7)
 *                byte 0 is the low order byte
 * @return 0 if byteNum is out of range
 *         byte 0, 1, .., or 7 of source if byteNum is within range
 *
 * RULES:
 * 1) you can use an if to handle error checking on input
 * 2) no loops or conditionals (other than for 1) or switch
*/
uint64_t Tools::getByte(uint64_t source, int32_t byteNum)
{
	if (byteNum > 7 || byteNum < 0) 
  		return 0;
	
	source = (source >> (byteNum*8)) & 0xFF;
	return source;	
	
}

/**
 * accepts as input an uint64_t and returns the bits low through 
 * high of the uint64_t. bit 0 is the low order bit and bit 63
 * is the high order bit. returns 0 if the low or high bit numbers 
 * are out of range
 *
 * for example, getBits(0x8877665544332211, 0, 7) returns 0x11
 *              getBits(0x8877665544332211, 4, 11) returns 0x21
 *              getBits(0x8877665544332211, 0, 63) returns 0x8877665544332211
 *              getBits(0x8877665544332211, 0, 64) returns 0
 *
 * @param uint64_t source that holds the bits to be grabbed and 
 *        returned
 * @param int32_t low that is the bit number of the lowest numbered
 *        bit to be returned
 * @param int32_t high that is the bit number of the highest numbered
 *        bit to be returned
 * @return an uint64_t that holds a subset of the source bits
 *         that is returned in the low order bits; returns 0 if low or high 
 *         is out of range
 *
 * RULES:
 * 1) you can use an if to handle error checking on input
 * 2) no loops or conditionals (other than for 1) or switch
 */
uint64_t Tools::getBits(uint64_t source, int32_t low, int32_t high)
{
	uint64_t mask = 0xFFFFFFFFFFFFFFFF;
  	if (low < 0 || high > 63 || low > high || high < low)
		return 0;
	
	source = source >> low;
	source = source & (mask >> (63 - (high - low)));
	return source;
}

/**
 * sets the bits of source in the range specified by the low and high
 * parameters to 1 and returns that value. returns source if the low or high
 * bit numbers are out of range
 *
 * for example, setBits(0x1122334455667788, 0, 7) returns 0x11223344556677ff
 *              setBits(0x1122334455667788, 8, 0xf) returns 0x112233445566ff88
 *              setBits(0x1122334455667788, 8, 64) returns 0x1122334455667788
 *                      note: 64 is out of range
 *
 * @param uint64_t source 
 * @param int32_t low that is the bit number of the lowest numbered
 *        bit to be set to 1
 * @param int32_t high that is the bit number of the highest numbered
 *        bit to be set to 1
 * @return an uint64_t that holds the modified source
 *
 * RULES:
 * 1) you can use an if to handle error checking on input
 * 2) no loops or conditionals (other than for 1) or switch
 * 3) you can use other functions you have written, for example, getBits
 */
uint64_t Tools::setBits(uint64_t source, int32_t low, int32_t high)
{
	uint64_t mask = 0xFFFFFFFFFFFFFFFF;
	if (low < 0 || high > 63 || low > high || high < low) 
		return source;
	
	mask = getBits(mask, low, high);	
	mask = mask << low;
	source = source | mask;	
	return source;
}

/**
 * clears the bits of source in the range low to high to 0 (clears them) and
 * returns that value. returns source if the low or high
 * bit numbers are out of range
 *
 * for example, clearBits(0x1122334455667788, 0, 7) returns 0x1122334455667700
 *              clearBits(0x1122334455667788, 8, f) returns 0x1122334455660088
 *
 * @param uint64_t source 
 * @param int32_t low that is the bit number of the lowest numbered
 *        bit to be set to 0
 * @param int32_t high that is the bit number of the highest numbered
 *        bit to be set to 0
 * @return an uint64_t that holds the modified source
 *
 * RULES:
 * 1) you can use an if to handle error checking on input
 * 2) no loops or conditionals (other than for 1) or switch
 * 3) you can use other functions you have written, for example, getBits
 */
uint64_t Tools::clearBits(uint64_t source, int32_t low, int32_t high)
{	
	uint64_t mask = 0xFFFFFFFFFFFFFFFF; 
  	if (low < 0 || high > 63 || low > high)
		return source;	
	
	mask = getBits(mask, low, high);
	mask = mask << low;
	mask = ~mask;
	source = source & mask;
	return source;

}

/**
 * assumes source contains a 64 bit two's complement value and
 * returns the sign (1 or 0)
 *
 * for example, sign(0xffffffffffffffff) returns 1
 *              sign(0x0000000000000000) returns 0
 *              sign(0x8000000000000000) returns 1
 *
 * @param uint64_t source
 * @return 1 if source is negative when treated as a two's complement 
 *         value and 0 otherwise
 *
 * RULES:
 * 1) no loops or conditionals
 * 2) you can use other functions you have written, for example, getBits
 */
uint64_t Tools::sign(uint64_t source)
{
  return getBits(source, 63, 63);
}

/**
 * assumes that op1 and op2 contain 64 bit two's complement values
 * and returns true if an overflow would occur if they are summed
 * and false otherwise
 *
 * for example, addOverflow(0x8000000000000000, 0x8000000000000000) returns 1
 *              addOverflow(0x7fffffffffffffff, 0x7fffffffffffffff) returns 1
 *              addOverflow(0x8000000000000000, 0x7fffffffffffffff) returns 0
 *
 * @param uint64_t op1 that is one of the operands of the addition
 * @param uint64_t op2 that is the other operand of the addition
 * @return true if op1 + op2 would result in an overflow assuming that op1
 *         and op2 contain 64-bit two's complement values
 *
 * RULES:
 * 1) no loops 
 * 2) you can use other functions you have written, for example, sign
 * 3) no more than 10 lines of code
 */
bool Tools::addOverflow(uint64_t op1, uint64_t op2)
{
 	uint64_t result = op1 + op2;
	if (sign(op1) == 0  && sign(op2) == 0 && sign(result) == 1)
		return true;
	else if (sign(op1) == 1  && sign(op2) == 1 && sign(result) == 0)
		return true;
	return false;

}

/**
 * assumes that op1 and op2 contain 64 bit two's complement values
 * and returns true if an overflow would occur from op2 - op1
 * and false otherwise
 *
 * for example, subOverflow(0x8000000000000000, 0x8000000000000000) returns 0
 *              subOverflow(0x7fffffffffffffff, 0x7fffffffffffffff) returns 0
 *              subOverflow(0x8000000000000000, 0x7fffffffffffffff) returns 1
 *
 * @param uint64_t op1 that is one of the operands of the subtraction
 * @param uint64_t op2 that is the other operand of the subtraction
 * @return true if op2 - op1 would result in an overflow assuming that op1
 *         and op2 contain 64-bit two's complement values
 *
 * RULES:
 * 1) no loops 
 * 2) you can use other functions you have written, for example, sign
 * 3) you cannot use addOverflow (it doesn't work in all cases).
 * 4) no more than 10 lines of code
 */
bool Tools::subOverflow(uint64_t op1, uint64_t op2)
{
  	uint64_t result = op2 - op1;
	if (sign(op2) == 0 && sign(op1) == 1 && sign(result) == 1)
		return true;
	else if (sign(op2) == 1 && sign(op1) == 0 && sign(result) == 0)
		return true;
  	return false;
}

/**
 * returns the largest magnitude positive value that can be stored
 * in a 64-bit two's complement number. (largest magnitude means furthest
 * from 0)
 *
 * RULES:
 * 1) no loops or conditionals
 * 2) no casts
 * 3) any constants you use must be between 0 and 255 (inclusive)
 * 4) can only use these operators: !, ~, >>, <<, |, ||, &, &&
*/
uint64_t Tools::maxPos()
{
	uint64_t result = 1;
	result = result << 63;
	result = ~result;
	return result;
}

/**
 * returns the largest magnitude negative value that can be stored
 * in a 64-bit two's complement number. (largest magnitude means furthest
 * from 0)
 *
 * RULES:
 * 1) no loops or conditionals
 * 2) no casts
 * 3) any constants you use must be between 0 and 255 (inclusive)
 * 4) can only use these operators: !, ~, >>, <<, |, ||, &, &&
*/
uint64_t Tools::maxNeg()
{
   	uint64_t result = 1; 
	result = result << 63;
	return result;
}

/**
 * returns true (1) if op is <= 0 when treated as
 * as a 64 bit two's complement values
 *
 * for example, isLessThanOrEq0(0x8000000000000000) returns 1
 *              isLessThanOrEq0(0x7fffffffffffffff) returns 0
 *              isLessThanOrEq0(0x0000000000000000) returns 1
 *              isLessThanOrEq0(0xFFFFFFFFFFFFFFFF) returns 1
 *
 * @param uint64_t op contains a 64-bit two's complement number
 * @return true if (int64_t) op <= 0 and false otherwise
 *
 * RULES:
 * 1) no loops or conditionals
 * 2) can use a cast
 * 3) any constants you use must be between 0 and 255 (inclusive)
 * 4) can only use these operators: !, ~, >>, <<, |, ||, &, &&
 */
bool Tools::isLessThanOrEq0(uint64_t op)
{
	int isZero = !op;
	int sign = op >> 63;
   	return sign || isZero;
}

/**
 * returns true (1) if op is > 2 when treated as
 * as a 64 bit two's complement values
 *
 * for example, isGreaterThanTwo(0x8000000000000000) returns 0
 *              isGreaterThanTwo(0x7fffffffffffffff) returns 1
 *              isGreaterThanTwo(0x0000000000000002) returns 0
 *              isGreaterThanTwo(0x0000000000000003) returns 1
 *
 * @param uint64_t op contains a 64-bit two's complement number
 * @return true if (int64_t) op > 2 and false otherwise
 *
 * RULES:
 * 1) no loops or conditionals
 * 2) can use a cast
 * 3) any constants you use must be between 0 and 255
 * 4) can only use these operators: !, ~, >>, <<, |, ||, &, &&
 */
bool Tools::isGreaterThanTwo(uint64_t op)
{
	int sign = op >> 63;
	uint64_t midBits = op >> 2;
	midBits = !!midBits;
	sign = !sign;
   	return sign && (midBits || ((op & 2) && (op & 1)));
}

/**
 * takes as input a 64-bit two's complement number and returns that
 * number with its even numbered bits (bits 0, 2, 4, 6, ..., 60, 62) flipped.
 * In other words, if the bit is 0 it becomes 1 and if the bit is 1, it
 * becomes 0.
 *
 * for example, flipEvenBits(0xFFFFFFFFFFFFFFFF) returns 0xAAAAAAAAAAAAAAAA
 *              flipEvenBits(0xFAFAFAFAFAFAFAFA) returns 0xAFAFAFAFAFAFAFAF
 *              flipEvenBits(0) returns 0x5555555555555555
 *
 * @param uint64_t op contains a 64-bit two's complement number
 * @return op with even numbered bits flipped
 *
 * RULES:
 * 1) no loops or conditionals
 * 2) can use a cast
 * 3) any constants you use must be between 0 and 255
 * 4) can only use these operators: >>, <<, |, ||, &, &&, ^
 */
uint64_t Tools::flipEvenBits(uint64_t op)
{
   uint64_t mask = 0x55;
   mask = (mask << 8) | mask;
   mask = (mask << 16) | mask;
   mask = (mask << 32) | mask;
   return mask ^ op;
}

/**
 * takes as input a 64-bit two's complement number and returns that
 * true if 2 can be added to the number without an overflow occurring.
 *
 * for example, addTwoOK(0xFFFFFFFFFFFFFFFF) returns 1
 *              addTwoOK(0x7FFFFFFFFFFFFFFF) returns 0
 *
 * @param uint64_t op contains a 64-bit two's complement number
 * @return true if (int64_t) op + 2 doesn't overflow and false otherwise
 *
 * RULES:
 * 1) no loops or conditionals
 * 2) can use a cast
 * 3) any constants you use must be between 0 and 255
 * 4) can only use these operators: >>, <<, |, ||, &, &&, ^, !, ~
 */
bool Tools::addTwoOK(uint64_t op)
{
	uint64_t tmax = 1;
	tmax = tmax << 63;
	tmax = ~tmax;
	uint64_t tmaxMinusOne = tmax >> 1;
	tmaxMinusOne = tmaxMinusOne << 1;
	return tmax ^ op && tmaxMinusOne ^ op;
}



