
// For an explanation of this class, look at its header,
// ToySimulator.hh, as well as
// https://cdcvs.fnal.gov/redmine/projects/artdaq-demo/wiki/Fragments_and_FragmentGenerators_w_Toy_Fragments_as_Examples

#include "artdaq-demo/Generators/ToySimulator.hh"

#include "art/Utilities/Exception.h"

#include "artdaq/Application/GeneratorMacros.hh"
#include "artdaq-core/Utilities/SimpleLookupPolicy.h"

#include "artdaq-core-demo/Overlays/ToyFragment.hh"
#include "artdaq-core-demo/Overlays/FragmentType.hh"


#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>

#include <unistd.h>


demo::ToySimulator::ToySimulator(fhicl::ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  hardware_interface_( new ToyHardwareInterface(ps) ),
  readout_buffer_(nullptr)
{
  hardware_interface_->AllocateReadoutBuffer(&readout_buffer_);   

  metadata_.board_serial_number = hardware_interface_->SerialNumber();
  metadata_.num_adc_bits = hardware_interface_->NumADCBits();
}

demo::ToySimulator::~ToySimulator() {
  hardware_interface_->FreeReadoutBuffer(readout_buffer_);
}

bool demo::ToySimulator::getNext_(artdaq::FragmentPtrs & frags) {

  if (should_stop()) {
    return false;
  }

  std::size_t bytes_read = 0;
  hardware_interface_->FillBuffer(readout_buffer_ , &bytes_read);

  // We'll use the static factory function 

  // artdaq::Fragment::FragmentBytes(std::size_t payload_size_in_bytes, sequence_id_t sequence_id,
  //  fragment_id_t fragment_id, type_t type, const T & metadata)

  // which will then return a unique_ptr to an artdaq::Fragment
  // object. The advantage of this approach over using the
  // artdaq::Fragment constructor is that, if we were to want to
  // initialize the artdaq::Fragment with a nonzero-size payload (data
  // after the artdaq::Fragment header and metadata), we could provide
  // the size of the payload in bytes, rather than in units of the
  // artdaq::Fragment's RawDataType (8 bytes, as of 3/26/14). The
  // artdaq::Fragment constructor itself was not altered so as to
  // maintain backward compatibility.

  std::unique_ptr<artdaq::Fragment> fragptr(
   					    artdaq::Fragment::FragmentBytes(bytes_read,  
   									    ev_counter(), fragment_id(),
   									    hardware_interface_->BoardType(), 
   									    metadata_));

  memcpy(fragptr->dataBeginBytes(), readout_buffer_, bytes_read );

  // Use the overlay class to check and make sure that no ADC values
  // in this fragment are larger than the max allowed

  ToyFragment fragoverlay( *fragptr );
  fragoverlay.fastVerify( metadata_.num_adc_bits );

  frags.emplace_back( std::move(fragptr ));

  if(metricMan_ != nullptr) {
    metricMan_->sendMetric("Fragments Sent",ev_counter(), "Events", 3);
  }

  ev_counter_inc();

  return true;
}

void demo::ToySimulator::start() {
  hardware_interface_->ConfigureInterface();
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::ToySimulator) 
