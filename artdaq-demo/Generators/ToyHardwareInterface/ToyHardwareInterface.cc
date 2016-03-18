
#include "artdaq-demo/Generators/ToyHardwareInterface/ToyHardwareInterface.hh"
#include "artdaq-core-demo/Overlays/ToyFragment.hh"
#include "artdaq-core-demo/Overlays/FragmentType.hh"

#include "fhiclcpp/ParameterSet.h"
#include "cetlib/exception.h"

#include <random>
#include <unistd.h>

// JCF, Mar-17-2016

// ToyHardwareInterface is meant to mimic a vendor-provided hardware
// API, usable within the the ToySimulator fragment generator. For
// purposes of realism, it's a C++03-style API, as opposed to, say, one
// based in C++11 capable of taking advantage of smart pointers, etc.

ToyHardwareInterface::ToyHardwareInterface(fhicl::ParameterSet const & ps) :
  interface_configured_(false),
  nADCcounts_(ps.get<size_t>("nADCcounts", 600000)), 
  fragment_type_(demo::toFragmentType(ps.get<std::string>("fragment_type"))), 
  throttle_usecs_(ps.get<size_t>("throttle_usecs", 100000)),
  engine_(ps.get<int64_t>("random_seed", 314159)),
  uniform_distn_(new std::uniform_int_distribution<data_t>(0, pow(2, NumADCBits() ) - 1 ))
{
}

// JCF, Mar-18-2017

// "ConfigureInterface" is meant to mimic the uploading of values to registers, etc. 

void ToyHardwareInterface::ConfigureInterface() {
  interface_configured_ = true;
}


void ToyHardwareInterface::FillBuffer(char* buffer, size_t* bytes_read) {

  if (interface_configured_) {

    usleep( throttle_usecs_ );

    *bytes_read = sizeof(demo::ToyFragment::Header) + nADCcounts_ * sizeof(data_t);
      
    // Make the fake data, starting with the header

    if ( *bytes_read % sizeof(demo::ToyFragment::Header::data_t) != 0) {
      throw cet::exception("HardwareInterface") <<
	"Not (yet) able to handle a fragment whose size isn't evenly divisible by the demo::ToyFragment::Header::data_t type size of " <<
	sizeof(demo::ToyFragment::Header::data_t) << " bytes";
    }

    demo::ToyFragment::Header* header = reinterpret_cast<demo::ToyFragment::Header*>(buffer);

    header->event_size = *bytes_read / sizeof(demo::ToyFragment::Header::data_t) ;
    header->trigger_number = 99;

    // Generate nADCcounts ADC values ranging from 0 to max with an
    // equal probability over the full range (a specific and perhaps
    // not-too-physical example of how one could generate simulated
    // data)

    std::generate_n(reinterpret_cast<data_t*>( reinterpret_cast<demo::ToyFragment::Header*>(buffer) + 1 ), 
		    nADCcounts_,
		    [&]() {
		      return static_cast<data_t>
			((*uniform_distn_)( engine_ ));
		    }
		    );

  } else {
    // what to do here? std::exit(1) ??
  }
}

void ToyHardwareInterface::AllocateReadoutBuffer(char** buffer) {
  
  *buffer = reinterpret_cast<char*>( new uint8_t[ sizeof(demo::ToyFragment::Header) + nADCcounts_*sizeof(data_t) ] );
}

void ToyHardwareInterface::FreeReadoutBuffer(char* buffer) {
  delete [] buffer;
}

uint8_t ToyHardwareInterface::BoardType() const {
  return fragment_type_;
}

uint8_t ToyHardwareInterface::NumADCBits() const {

  switch (fragment_type_) {
  case demo::FragmentType::TOY1:
    return 12;
    break;
  case demo::FragmentType::TOY2:
    return 14;
    break;
  default:
    throw cet::exception("ToyHardwareInterface")
      << "Unknown board type "
      << fragment_type_
      << " ("
      << demo::fragmentTypeToString(fragment_type_)
      << ").\n";
  };

}

uint16_t ToyHardwareInterface::SerialNumber() const {
  return 999;
}

