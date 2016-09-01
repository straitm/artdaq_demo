
#include "artdaq/TransferPlugins/TransferInterface.h"
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

  class moduloTransfer : public TransferInterface {

  public:
    moduloTransfer(fhicl::ParameterSet const& ps, artdaq::TransferInterface::Role role); 

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
    size_t modulus_;
    
  };

  moduloTransfer::moduloTransfer(fhicl::ParameterSet const& pset, artdaq::TransferInterface::Role role) :
    TransferInterface(pset, role),
    modulus_(pset.get<size_t>("modulus"))
  {
    
    // I WILL replace the following code with makeTransferPlugin...

    static cet::BasicPluginFactory bpf("transfer", "make");

    fhicl::ParameterSet physical_transfer_pset;

    try {
      physical_transfer_pset = pset.get<fhicl::ParameterSet>("physical_transfer_plugin");
    }  catch (...) {
      std::stringstream errmsg;
      errmsg
	<< "Unable to find the physical transfer plugin parameters in the "
	<< "ParameterSet: \"" + pset.to_string() + "\".";
      ExceptionHandler(ExceptionHandlerRethrow::yes, errmsg.str());
    }

    try {
      physical_transfer_ =
	bpf.makePlugin<std::unique_ptr<TransferInterface>,
	const fhicl::ParameterSet&,
	TransferInterface::Role>(
				 physical_transfer_pset.get<std::string>("transferPluginType"),
				 physical_transfer_pset,
				 std::move(role));
    } catch (...) {
      std::stringstream errmsg;
      errmsg 
	<< "Problem creating physical transfer plugin with ParameterSet: \"" << physical_transfer_pset.to_string() << "\"";
      ExceptionHandler(ExceptionHandlerRethrow::yes, errmsg.str());
    }
  }


  void moduloTransfer::copyFragmentTo(bool& fragmentHasBeenCopied,
				      bool& esrHasBeenCopied,
				      bool& eodHasBeenCopied,
				      artdaq::Fragment& fragment,
				      size_t send_timeout_usec) {

    if (fragment.sequenceID() % modulus_ != 0) {
      return;
    }

    physical_transfer_->copyFragmentTo(fragmentHasBeenCopied, esrHasBeenCopied, eodHasBeenCopied, fragment, send_timeout_usec);
  }

}

DEFINE_ARTDAQ_TRANSFER(artdaq::moduloTransfer)

// Local Variables:
// mode: c++
// End:
