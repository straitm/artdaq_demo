#ifndef artdaq_demo_Generators_V172xSimulator_hh
#define artdaq_demo_Generators_V172xSimulator_hh

#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragments.hh"
#include "artdaq/Application/CommandableFragmentGenerator.hh"
#include "artdaq-core-demo/Overlays/V172xFragment.hh"
#include "artdaq-core-demo/Overlays/FragmentType.hh"

#include <random>
#include <vector>
#include <atomic>

namespace demo {
  // V172xSimulator creates simulated DS50 events, with data
  // distributed according to a "histogram" provided in the configuration
  // data.

  class V172xSimulator : public artdaq::CommandableFragmentGenerator {
  public:
    explicit V172xSimulator(fhicl::ParameterSet const & ps);

  private:
    bool getNext_(artdaq::FragmentPtrs & output) override;

    // Explicitly declare that there is nothing special to be done
    // by the start, stop, and stopNoMutex methods in this class
    void start() override {}
    void stop() override {}
    void stopNoMutex() override {}

    // Configuration.
    std::size_t const nChannels_;
    FragmentType const fragment_type_;

    // State.
    std::vector<std::vector<size_t>> adc_freqs_;
    std::vector<std::discrete_distribution<V172xFragment::adc_type>> content_generator_;
    std::mt19937 engine_;

  };
}

#endif /* artdaq_demo_Generators_V172xSimulator_hh */
