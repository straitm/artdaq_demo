namespace CRT{

/*
  Decodes the data in 'rawfromhardware' and puts the result in
  'cooked_data', returning the number of bytes put into cooked_data,
  which can be up to 'max_cooked'.

  'cooked_data' will consist of zero or more "module packets", which
  are collections of hits from a single module sharing a single time
  stamp.  Up to the limits described below, it may contain many packets
  from any span of time if that's what was available to be read.

  If there is not a complete module packet in rawfromhardware, it returns zero
  and leaves all arguments unmodified.  Otherwise, it rotates the buffer
  rawfromhardware to put the first unused byte (which hopefully represents the
  beginning of a currently-incomplete module packet) at the beginning and sets
  next_raw_byte to one past the end of the remaining data, i.e. to where the
  caller should write new data.

  If the data in rawfromhardware would decode to more than max_cooked bytes,
  decodes only as many module packets as can fit in cooked_data.  If this is
  zero, emits a warning and flushes the input buffer, i.e. sets next_raw_byte
  to rawfromhardware and returns zero.
*/
unsigned int raw2cook(char * const cooked_data,
                      const unsigned int max_cooked,
                      char * rawfromhardware,
                      char * & next_raw_byte);

}
