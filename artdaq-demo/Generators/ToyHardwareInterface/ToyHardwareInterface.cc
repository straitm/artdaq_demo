
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
  taking_data_(false),
  nADCcounts_(ps.get<size_t>("nADCcounts", 600000)), 
  fragment_type_(demo::toFragmentType(ps.get<std::string>("fragment_type"))), 
  throttle_usecs_(ps.get<size_t>("throttle_usecs", 100000)),
  distribution_type_(static_cast<DistributionType>(ps.get<int>("distribution_type"))),
  max_adc_(pow(2, NumADCBits() ) - 1),
  engine_(ps.get<int64_t>("random_seed", 314159)),
  uniform_distn_(new std::uniform_int_distribution<data_t>(0, max_adc_)),
  gaussian_distn_(new std::normal_distribution<double>( 0.5*max_adc_, 0.1*max_adc_))
{
}

// JCF, Mar-18-2017

// "StartDatataking" is meant to mimic actions one would take when
// telling the hardware to start sending data - the uploading of
// values to registers, etc.

void ToyHardwareInterface::StartDatataking() {
  taking_data_ = true;
}

void ToyHardwareInterface::StopDatataking() {
  taking_data_ = false;
}


void ToyHardwareInterface::FillBuffer(char* buffer, size_t* bytes_read) {

  if (taking_data_) {

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

    // Generate nADCcounts ADC values ranging from 0 to max based on
    // the desired distribution

    std::function<data_t()> generator;

    switch (distribution_type_) {
    case DistributionType::uniform:
      generator = [&]() {
	return static_cast<data_t>
	((*uniform_distn_)( engine_ ));
      };
      break;

    case DistributionType::gaussian:
      generator = [&]() {

	data_t gen(0);
	do {
	  gen = static_cast<data_t>( std::round( (*gaussian_distn_)( engine_ ) ) );
	} 
	while(gen > max_adc_);                                                                    
	return gen;
      };
      break;

    default:
      throw cet::exception("HardwareInterface") <<
	"Unknown distribution type specified";
    }

    std::generate_n(reinterpret_cast<data_t*>( reinterpret_cast<demo::ToyFragment::Header*>(buffer) + 1 ), 
		    nADCcounts_,
		    generator
		    );

  } else {
    throw cet::exception("ToyHardwareInterface") <<
      "Attempt to call FillBuffer when not sending data";
  }
}

void ToyHardwareInterface::AllocateReadoutBuffer(char** buffer) {
  
  *buffer = reinterpret_cast<char*>( new uint8_t[ sizeof(demo::ToyFragment::Header) + nADCcounts_*sizeof(data_t) ] );
}

void ToyHardwareInterface::FreeReadoutBuffer(char* buffer) {
  delete [] buffer;
}

// Pretend that the "BoardType" is some vendor-defined integer which
// differs from the fragment_type_ we want to use as developers (and
// which must be between 1 and 224, inclusive) so add an offset

int ToyHardwareInterface::BoardType() const {
  return static_cast<int>(fragment_type_) + 1000;
}

int ToyHardwareInterface::NumADCBits() const {

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

int ToyHardwareInterface::SerialNumber() const {
  return 999;
}

