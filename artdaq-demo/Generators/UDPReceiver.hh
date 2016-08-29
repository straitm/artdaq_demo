#ifndef artdaq_ots_Generators_UDPTFGTest_hh
#define artdaq_ots_Generators_UDPTFGTest_hh

// The UDP Receiver class recieves UDP data from an OtSDAQ applicance and
// puts that data into UDPFragments for further ARTDAQ analysis.
//
// It currently assumes two things to be true:
// 1. The first word of the UDP packet is an 8-bit flag with information
// about the status of the sender
// 2. The second word is an 8-bit sequence ID, used for detecting
// dropped UDP datagrams

// Some C++ conventions used:

// -Append a "_" to every private member function and variable
#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragments.hh" 
#include "artdaq/Application/CommandableFragmentGenerator.hh"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <list>
#include <queue>
#include <atomic>

namespace demo {    

enum class CommandType : uint8_t {
    Read = 0,
    Write = 1,
    Start_Burst = 2,
    Stop_Burst = 3,
      };

enum class ReturnCode : uint8_t {
    Read = 0,
      First = 1,
      Middle = 2,
      Last = 3,
      };

  enum class DataType : uint8_t {
    Raw = 0,
      JSON = 1,
      String = 2,
  };

struct CommandPacket {
  CommandType type;
  uint8_t dataSize;
  uint64_t address;
  uint64_t data[182];
};

  typedef std::array<uint8_t, 1500> packetBuffer_t;
  typedef std::list<packetBuffer_t> packetBuffer_list_t;

  class UDPReceiver : public artdaq::CommandableFragmentGenerator {
  public:
    explicit UDPReceiver(fhicl::ParameterSet const & ps);

  private:

    // The "getNext_" function is used to implement user-specific
    // functionality; it's a mandatory override of the pure virtual
    // getNext_ function declared in CommandableFragmentGenerator

    bool getNext_(artdaq::FragmentPtrs & output) override;

    void start() override;
    void stop() override;
    void stopNoMutex() override {}  // nothing special needs to be done in this method
    void pause() override;
    void resume() override;
   
    DataType getDataType(uint8_t byte) { return static_cast<DataType>((byte & 0xF0) >> 4); }
    ReturnCode getReturnCode(uint8_t byte) { return static_cast<ReturnCode>(byte & 0xF); }
    void send(CommandType flag);
    // FHiCL-configurable variables. Note that the C++ variable names
    // are the FHiCL variable names with a "_" appended

    int dataport_;
    std::string ip_;

    //The packet number of the next packet. Used to discover dropped packets
    uint8_t expectedPacketNumber_;

    //Socket parameters
    struct sockaddr_in si_data_;
    int datasocket_;
    bool sendCommands_;

    packetBuffer_list_t packetBuffers_;

    bool rawOutput_;
    std::string rawPath_;
  };
}

#endif /* artdaq_demo_Generators_ToySimulator_hh */
