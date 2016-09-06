
#include "artdaq/TransferPlugins/TransferInterface.h"
#include "artdaq/TransferPlugins/MakeTransferPlugin.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Utilities/ExceptionHandler.hh"
#include "cetlib/BasicPluginFactory.h"

#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/ParameterSet.h"

#include <boost/tokenizer.hpp>

#include <sys/shm.h>
#include <memory>
#include <iostream>
#include <string>
#include <limits>
#include <sstream>

namespace artdaq {

  class NthEventTransfer : public TransferInterface {

  public:
    NthEventTransfer(fhicl::ParameterSet const& ps, artdaq::TransferInterface::Role role); 

    void copyFragmentTo(bool& fragmentHasBeenCopied,
			bool& esrHasBeenCopied,
			bool& eodHasBeenCopied,
			artdaq::Fragment& fragment,
			size_t send_timeout_usec = std::numeric_limits<size_t>::max());

    size_t receiveFragmentFrom(artdaq::Fragment& fragment,
			       size_t receiveTimeout) {
      return physical_transfer_->receiveFragmentFrom(fragment, receiveTimeout);
    }


  private:

    std::unique_ptr<TransferInterface> physical_transfer_;
    size_t nth_;
    
  };

  NthEventTransfer::NthEventTransfer(fhicl::ParameterSet const& pset, artdaq::TransferInterface::Role role) :
    TransferInterface(pset, role),
    nth_(pset.get<size_t>("nth"))
  {
    physical_transfer_ = MakeTransferPlugin(pset, "physical_transfer_plugin", role);    
  }


  void NthEventTransfer::copyFragmentTo(bool& fragmentHasBeenCopied,
				      bool& esrHasBeenCopied,
				      bool& eodHasBeenCopied,
				      artdaq::Fragment& fragment,
				      size_t send_timeout_usec) {

    if (fragment.sequenceID() % nth_ != 0) {
      return;
    }

    physical_transfer_->copyFragmentTo(fragmentHasBeenCopied, esrHasBeenCopied, eodHasBeenCopied, fragment, send_timeout_usec);
  }

}

DEFINE_ARTDAQ_TRANSFER(artdaq::NthEventTransfer)

// Local Variables:
// mode: c++
// End:
