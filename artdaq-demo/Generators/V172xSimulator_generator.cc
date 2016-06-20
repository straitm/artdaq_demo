#include "artdaq-demo/Generators/V172xSimulator.hh"

#ifdef CANVAS
#include "canvas/Utilities/Exception.h"
#else
#include "art/Utilities/Exception.h"
#endif

#include "artdaq/Application/GeneratorMacros.hh"
#include "cetlib/exception.h"
#include "artdaq-core-demo/Overlays/V172xFragment.hh"
#include "artdaq-core-demo/Overlays/V172xFragmentWriter.hh"
#include "artdaq-core-demo/Overlays/FragmentType.hh"
#include "fhiclcpp/ParameterSet.h"
#include "artdaq-core/Utilities/SimpleLookupPolicy.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>

#include <unistd.h>

namespace {
  size_t typeToADC(demo::FragmentType type)
  {
    switch (type) {
    case demo::FragmentType::V1720:
      return 12;
      break;
    case demo::FragmentType::V1724:
      return 14;
      break;
    default:
      throw art::Exception(art::errors::Configuration)
        << "Unknown board type "
        << type
        << " ("
        << demo::fragmentTypeToString(type)
        << ").\n";
    };
  }

  void read_adc_freqs(std::string const & fileName, std::string const & filePath,
                      std::vector<std::vector<size_t>> & freqs, int adc_bits) {

    // 06-Jan-2013, KAB - added the ability to find the specified data file
    // in a list of paths specified in an environmental variable
    if (getenv(filePath.c_str()) == nullptr) {
      setenv(filePath.c_str(), ".", 0);
    }
    artdaq::SimpleLookupPolicy lookup_policy(filePath);
    std::string fullPath = fileName;
    try {fullPath = lookup_policy(fileName);}
    catch (...) {}

    std::ifstream is(fullPath);
    if (!is) {
      throw cet::exception("FileOpenError")
        << "Unable to open distribution data file "
        << fileName << " with paths in " << filePath << ".";
    }
    std::string header;
    std::getline(is, header);
    is.ignore(1);
    std::vector<std::string> split_headers;
    cet::split(header, ' ', std::back_inserter(split_headers));
    size_t nHeaders = split_headers.size();
    freqs.clear();
    freqs.resize(nHeaders -1); // Take account of ADC column.
    for (auto & freq : freqs) {
      freq.resize(demo::V172xFragment::adc_range(adc_bits));
    }
    while (is.peek() != EOF) { // If we get EOF here, we're OK.
      if (!is.good()) {
        throw cet::exception("FileReadError")
          << "Error reading distribution data file "
          << fileName;
      }
      demo::V172xFragment::adc_type channel;
      is >> channel;
      for (auto & freq : freqs) {
        is >> freq[channel];
      }
      is.ignore(1);
    }
    is.close();
  }
}

demo::V172xSimulator::V172xSimulator(fhicl::ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  nChannels_(ps.get<size_t>("nChannels", 600000)),
  fragment_type_(toFragmentType(ps.get<std::string>("fragment_type", "V1720"))),
  adc_freqs_(),
  content_generator_(),
  engine_(ps.get<int64_t>("random_seed", 314159))
{

  // Read frequency tables.
  size_t const adc_bits = typeToADC(fragment_type_);
  auto const fragments_per_board = fragmentIDs().size();
  content_generator_.reserve(fragments_per_board);
  read_adc_freqs(ps.get<std::string>("freqs_file"),
                 ps.get<std::string>("freqs_path", "DAQ_INDATA_PATH"),
                 adc_freqs_,
                 adc_bits);

  // Initialize content generators and set up separate random # generators for each fragment on the board
  for (size_t i = 0; i < fragments_per_board; ++i) {
    content_generator_.emplace_back(V172xFragment::adc_range(adc_bits),
                                    -0.5,
                                    V172xFragment::adc_range(adc_bits) - 0.5,
                                    [this, i](double x) -> double { return adc_freqs_[i][std::round(x)]; }
                                   );
  }
    
}


bool demo::V172xSimulator::getNext_(artdaq::FragmentPtrs & frags) {

  if (should_stop()) {
    return false;
  }

  // 11-Jan-2014, KAB: sleep a short time to avoid getting ahead of
  // the eventbuilders
  usleep(100000);

// #pragma omp parallel for shared(fragID, frags)
// TODO: Allow parallel operation by having multiple engines (with different seeds, of course).
  for (size_t i = 0; i < fragmentIDs().size(); ++i) {
    frags.emplace_back(new artdaq::Fragment);
    V172xFragmentWriter newboard(*frags.back());
    newboard.resize(nChannels_);
    newboard.setBoardID( board_id() ); 
    newboard.setEventCounter(ev_counter());

    demo::V172xFragment::Header::channel_mask_t mask = 0;
    for (size_t j = 0; j < fragmentIDs().size(); ++j) {
      mask |= (1 << j);
    }


    newboard.setChannelMask(1);

    std::generate_n(newboard.dataBegin(),
                    nChannels_,
                    [this, i]() {
                      return static_cast<V172xFragment::adc_type>
                        (std::round(content_generator_[i]( engine_ )));
                    }
                   );

    //    cout << "Fragment " << i << ", vals are: ";
    //    for (auto it = newboard.dataBegin(); it != newboard.dataEnd(); it++) {
    //      cout << *it << " ";
    //    }
    //    cout << endl;
    

    artdaq::Fragment& frag = *frags.back();

    frag.setFragmentID (fragmentIDs()[i]); 
    frag.setSequenceID (ev_counter());
    frag.setUserType(fragment_type_);
  }

  ev_counter_inc();

  return true;
}

DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::V172xSimulator)
