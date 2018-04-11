namespace CRT{

/*
  Decodes the data in 'rawfromhardware' and puts the result in
  'cooked_data', returning the number of bytes put into cooked_data,
  which can be up to 'max_cooked'.

  If there is not a complete event in rawfromhardware, it returns zero
  and leaves all arguments unmodified.  Otherwise, it rotates the buffer
  rawfromhardware to put the first unused byte at the beginning and sets
  next_raw_byte to one past the end of the remaining data, i.e. to where
  the caller should write new data.

  If the data in rawfromhardware would decode to more than max_cooked
  bytes, decodes only as many events as can fit in cooked_data.  If this
  is zero, emits a warning and flushes the input buffer, i.e. sets
  next_raw_byte to rawfromhardware and returns zero.
*/
unsigned int raw2cook(char * const cooked_data,
                      const unsigned int max_cooked,
                      char * rawfromhardware,
                      char * & next_raw_byte);

}
