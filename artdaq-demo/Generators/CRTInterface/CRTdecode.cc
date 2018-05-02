#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h> // For htons, htonl
#include <deque>
#include <vector>

namespace CRT{

// The raw data stream occasionally has packets that tell the Unix time of the
// upstream CRT DAQ.  Once we get one of these, copy the latest Unix time into
// the output events.  Before this point, we'll write zeros, and probably
// discard that data.  Only a problem if data runs are verys short.
uint16_t unix_time_hi = 0, unix_time_lo = 0;

// A hit after decoding.
struct decoded_hit {
  uint8_t channel;
  int16_t charge;
};

// A module packet after decoding.  This is a possibly-unnecessary intermediate
// format that still needs serializing before it can be written out.
struct decoded_packet {
  decoded_packet()
  {
    module = 0;
    timeunix = 0;
    time16ns = 0;
  }

  uint16_t module;
  uint32_t timeunix;
  uint32_t time16ns;
  std::vector<decoded_hit> hits;
};


/*
  Return whether the input 24 bit word is part of a Unix timestamp packet, as
  revealed by its control code (bits 3-8).

  For reference, here's what's known about other control codes:

  # c1 dac pmt
  # c2 dac
  # c5 ....  -skipped
  # c6 data_hi    - number of received data words
  # c6 data_lo
  # c6 lost_hi     - number of lost data words
  # c6 lost_lo
  # c8 timestamp_hi
  # c9 timestamp_lo
*/
bool is_unix_time_word(const uint32_t wordin)
{
  const uint8_t control = (wordin >> 16) & 0xff;
  return control == 0xc8 || control == 0xc9;
}

/*
  Sets the global variables for the upper or lower half of the Unix time,
  as per the 24-bit word 'wordin'.
*/
void set_unix_time(const uint32_t wordin)
{
  const uint8_t control = (wordin >> 16) & 0xff;
  const uint16_t payload = wordin & 0xffff;
  if     (control == 0xc8) unix_time_hi = payload;
  else if(control == 0xc9) unix_time_lo = payload;
  else fprintf(stderr, "CRT: Not reached in set_unix_time()\n");
}

// Convert a 24-bit word from upstream to a 16-bit word by stripping off the
// leading control octet.  Returns true and puts the result on the end of
// 'raw16bitdata' if a conversion was performed.  Returns false if we found
// a Unix timestamp packet or if we got rubbish.
bool raw24bit_to_raw16bit(std::deque<uint16_t> & raw16bitdata,
                          uint32_t in24bitword)
{
  // Old comment here said "command word, not data" for the case that
  // the first two bits were 01b. Apparently there are 24 bit words
  // undocumented in Matt Toups' thesis that start with values other
  // than 11b, but we just ignore them.
  if(((in24bitword >> 22) & 3) != 3) return false;

  if(is_unix_time_word(in24bitword)){
    set_unix_time(in24bitword);
    return false;
  }

  raw16bitdata.push_back(in24bitword & 0xffff);

  return true;
}

/*
Converts the "decoded_packet" in-memory format in 'packet' to the serialized
output format and puts the result in 'cooked', provided that the result is
not more than max_cooked bytes long.  Returns the number of bytes written,
unless the result is too long, in which case returns -1.  In this case,
'cooked' may contain truncated data, but you probably don't want to use it.

The serialized format of a module packet is:

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Magic number | Count of hits |         Module number         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                        Unix Time stamp                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                   50 MHz counter time stamp                   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |               .              hits             .               |
   |               .               .               .               |
  
  Magic number: 0x4D = "M"

  Count of hits: Unsigned 8 bit integer.
  
  Module number: Unsigned 16 bit integer.

  Time stamp: Unsigned 32 bit integer

    Number of 20ns ticks since last sync pulse.  Shouldn't usually be
    more than 2^29 - 1.

The format of a hit is:

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Magic number |    Channel    |             Charge            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  Magic number: 0x48 = "H"

  Channel number: Unsigned 8 bit integer
    
    Actually only uses 5 bits (0-63).
    
  Charge: *Signed* 16 bit integer

    Stored in two's complement.  Represents a 12-bit ADC value.  So we
    could store this in 13 bits, but there's no great motivation to pack
    the data so closely, especially because the minimum size of a hit
    is 18 bits, which I would tend to pad out to 32 anyway.

*/
unsigned int serialize(char * cooked, const decoded_packet & packet,
                       const unsigned int max_cooked)
{
  if(packet.hits.empty()) return 0;

  const unsigned int size_needed = 2 + sizeof packet.module
    + sizeof packet.timeunix + sizeof packet.time16ns
    + packet.hits.size() *
      (1 + sizeof packet.hits[0].charge + sizeof packet.hits[0].channel);

  if(size_needed > max_cooked){
    fprintf(stderr, "Can't write packet of size %d with max %d\n",
            size_needed, max_cooked);
    return 0;
  }

  unsigned int bytes = 0;
  cooked[bytes++] = 'M';
  cooked[bytes++] = packet.hits.size();
  memcpy(cooked+bytes, &packet.module, sizeof packet.module);
  bytes += sizeof packet.module;
  memcpy(cooked+bytes, &packet.timeunix, sizeof packet.timeunix);
  bytes += sizeof packet.timeunix;
  memcpy(cooked+bytes, &packet.time16ns, sizeof packet.time16ns);
  bytes += sizeof packet.time16ns;

  for(unsigned int m = 0; m < packet.hits.size(); m++) {
    cooked[bytes++] = 'H';
    cooked[bytes++] = packet.hits[m].channel;
    memcpy(cooked+bytes, &packet.hits[m].charge, sizeof packet.hits[m].charge);
    bytes += sizeof packet.hits[m].charge;
  }

  if(bytes != size_needed)
    fprintf(stderr, "CRT: size mismatch %d != %d\n", bytes, size_needed);

  return bytes; 
}

/*
  Decodes the 16-bit data in 'raw' to an ADC packet written into 'cooked' and
  returns the length of the packet in bytes.  Deletes the used contents of raw,
  leaving trailing unused words.  If the data in 'raw' cannot be
  decoded, or would decode to a size larger than max_cooked, returns zero and
  leaves 'cooked' undefined. 
*/
unsigned int make_a_packet(char * cooked, std::deque<uint16_t> & raw,
                           const unsigned int max_cooked)
{
  // ADC packet word indices.  As per Toups thesis:
  //
  // 0xffff                         | Header word
  // 1, mod#[7 bits], wdcnt[8 bits] | Data type, module #, word count
  // time                           | High 16 bits of 62.5 MHz clock counter
  // time                           | Low  16 bits of 62.5 MHz clock counter
  // adc                            | ADC ...
  // channel                        | ... and channel number, repeated N times
  // parity                         | Parity
  enum ADC_WINX { ADC_WIDX_HEAD   = 0, ADC_WIDX_MODLEN = 1,
                  ADC_WIDX_CLKHI  = 2, ADC_WIDX_CLKLO  = 3,
                  ADC_WIDX_HIT    = 4 };

  // Try to decode the data in 'raw'. Stop trying if 'raw' is empty, or
  // if it starts out correctly with 0xffff but has nothing else, or if it is
  // shorter than the length it claims to have.  But otherwise, drop the
  // first word and try to decode again.

  // First word of all packets other than unix timestamp packets is 0xffff
  // If there's any junk before 0xffff, discard it.
  while(!raw.empty() && raw[0] != 0xffff){
    fprintf(stderr, "CRT: Discarding a junk word 0x%04x\n", raw[0]);
    raw.pop_front();
  }

  if(raw.size() < 2) return 0;

  unsigned int len = raw[ADC_WIDX_MODLEN] & 0xff;
  if(len == 0){
    fprintf(stderr, "CRT: Discarding empty packet\n");
    raw.clear();
    return 0;
  }

  // we don't have all the data in this packet yet
  if(raw.size() < len + 1) return 0;

  if(!(raw[ADC_WIDX_MODLEN] >> 15)){
    fprintf(stderr, "CRT: Non-ADC packet found, skipping\n");
    return 0;
  }

  unsigned int parity = 0;

  decoded_packet packet;
  packet.timeunix = ((uint32_t)unix_time_hi << 16) + unix_time_lo;
  packet.module = (raw[ADC_WIDX_MODLEN] >> 8) & 0x7f;

  for(unsigned int wordi = ADC_WIDX_MODLEN; wordi < len; wordi++){
    parity ^= raw[wordi];

    if(wordi == ADC_WIDX_CLKHI) {
      packet.time16ns |= (raw[wordi] << 16);
    }
    else if(wordi == ADC_WIDX_CLKLO) {
      packet.time16ns |= raw[wordi];
    }
    else{ // we are in the words that give the hit info
      // hits start on even numbered words
      if(wordi%2 == 0) {
        decoded_hit hit;
        hit.channel = raw[wordi+1];
        hit.charge  = raw[wordi];
        packet.hits.push_back(hit);
      }
    }
  }

  if(parity != raw[len])
    fprintf(stderr, "CRT: Parity error in USB stream\n");

  // Discard raw data that has been decoded
  for(unsigned int i = 0; i < len + 1; i++) raw.pop_front();

  return serialize(cooked, packet, max_cooked);
}

unsigned int raw2cook(char * const cooked_data,
                      const unsigned int max_cooked,
                      char * rawfromhardware,
                      char * & next_raw_byte)
{
  /*
    Undocumented input file format is revealed by inspection to be
    constructed like this:

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0|     A     |0 1|      B    |1 0|     C     |1 1|     D     |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   Where the bits of A, B, C, and D concatenated make the 24-bit words
   described in Matt Toups' thesis.
 */

  uint32_t word = 0; // holds 24-bit word being built, must be unsigned
  char expcounter = 0; // expecting this counter next

  std::deque<uint16_t> raw16bitdata;

  char * readptr = rawfromhardware;

  unsigned int used_raw_bytes = 0;
  unsigned int cooked_bytes = 0;

  for(; readptr < next_raw_byte; readptr++){
    const char counter = ((*readptr) >> 6) & 3;
    const char payload = (*readptr) & 0x3f;
    if(counter == 0){
      expcounter = 1;
      word = payload;
    }
    else if(counter == expcounter){
      word = (word << 6) | payload;
      if(++expcounter == 4){
        expcounter = 0;

        // Every time we assemble a 24-bit word that could be part of
        // an ADC packet, try to decode the whole stream so far.

        unsigned int newcookedbytes = 0;
        if(raw24bit_to_raw16bit(raw16bitdata, word) &&
           (newcookedbytes = make_a_packet(cooked_data + cooked_bytes,
                                           raw16bitdata,
                                           max_cooked - cooked_bytes))){
          used_raw_bytes = readptr - rawfromhardware + 1; 
          cooked_bytes += newcookedbytes;
        }
      }
    }
    else{
      fprintf(stderr, "CRT: corrupted data: expected counter %d, got %d\n",
              expcounter, counter);
      expcounter = 0;
    }
  }

  printf("Used %u bytes\n", used_raw_bytes);

  // Rotate buffer in the most wasteful way possible, by actually moving
  // the undecoded bytes to the front.
  if(used_raw_bytes){
    printf("Moving %ld bytes\n", next_raw_byte - rawfromhardware - used_raw_bytes);
    memmove(rawfromhardware, rawfromhardware + used_raw_bytes,
            next_raw_byte - rawfromhardware - used_raw_bytes);
  }

  next_raw_byte -= used_raw_bytes;
  return used_raw_bytes;
}

} // end namespace CRT