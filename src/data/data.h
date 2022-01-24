#ifndef _CNODE_FASTDATA_H_
#define _CNODE_FASTDATA_H_

#include <inttypes.h>


/**
 * @brief Converts a single char to it's integer equivalent
 * 
 * @param c     character to convert
 * @return Converted integer value  
 */
unsigned char x_ctoi(char c);

/**
 * @brief Round number to nearest next multiple
 * 
 * @param number        number to round
 * @param multiple      multiple to round number to
 * @return result rounded number 
 */
uint64_t rtonm(uint64_t number, uint64_t multiple);

#endif