#ifndef artdaq_demo_Generators_V172xFileReader_hh
#define artdaq_demo_Generators_V172xFileReader_hh

#include "artdaq-core-demo/Overlays/FragmentType.hh"
#include "artdaq-core-demo/Overlays/V172xFragment.hh"
#include "artdaq/Application/CommandableFragmentGenerator.hh"
#include "artdaq-core/Data/Fragments.hh"
#include "fhiclcpp/fwd.h"

#include <atomic>
#include <string>
#include <utility>
#include <vector>

namespace demo {
  // V172xFileReader reads DS50 events from a file or set of files.

  class V172xFileReader : public artdaq::CommandableFragmentGenerator {
  public:
    explicit V172xFileReader(fhicl::ParameterSet const &);

  private:
    bool getNext_(artdaq::FragmentPtrs & output) override;

    void produceSecondaries_(artdaq::FragmentPtrs & frags);
    artdaq::FragmentPtr
    convertFragment_(artdaq::Fragment const & source,
                     demo::FragmentType dType,
                     artdaq::Fragment::fragment_id_t id);

    // Explicitly declare that there is nothing special to be done
    // by the start, stop, and stopNoMutex methods in this class
    void start() override {}
    void stop() override {}
    void stopNoMutex() override {}

    // Configuration.
    std::vector<std::string> const fileNames_;
    uint64_t const max_set_size_bytes_;
    int const max_events_;
    FragmentType const primary_type_;
    std::vector<FragmentType> secondary_types_;

    bool const size_in_words_; // To cope with malformed files.
    V172xFragment::adc_type const seed_;

    // State
    size_t events_read_;
    std::pair<std::vector<std::string>::const_iterator, uint64_t> next_point_;
    std::atomic<bool> should_stop_;
    std::independent_bits_engine<std::minstd_rand, 2, V172xFragment::adc_type> twoBits_;

  };
}

#endif /* artdaq_demo_Generators_V172xFileReader_hh */
