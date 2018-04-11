#ifndef artdaq_demo_Generators_ToySimulator_hh
#define artdaq_demo_Generators_ToySimulator_hh

#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Application/CommandableFragmentGenerator.hh"
#include "artdaq-core-demo/Overlays/ToyFragment.hh"

#include "ToyHardwareInterface/ToyHardwareInterface.hh"

#include <random>
#include <vector>
#include <atomic>

namespace demo
{
  class ToySimulator : public artdaq::CommandableFragmentGenerator
  {
    public:

    explicit ToySimulator(fhicl::ParameterSet const& ps);
    virtual ~ToySimulator();

    private:

    /**
     * \brief The "getNext_" function is used to implement user-specific
     * functionality; it's a mandatory override of the pure virtual
     * getNext_ function declared in CommandableFragmentGenerator
     * \param output New FragmentPtrs will be added to this container
     * \return True if data-taking should continue
     */
    bool getNext_(std::list< std::unique_ptr<artdaq::Fragment> > & output) override;

    // The start, stop and stopNoMutex methods are declared pure
    // virtual in CommandableFragmentGenerator and therefore MUST be
    // overridden; note that stopNoMutex() doesn't do anything here

    /**
     * \brief Perform start actions
     * Override of pure virtual function in CommandableFragmentGenerator.
     */
    void start() override;

    /** \brief Perform stop actions
    * Override of pure virtual function in CommandableFragmentGenerator.  */
    void stop() override;

    /** \brief Override of pure virtual function in CommandableFragmentGenerator.
    * stopNoMutex does not do anything in ToySimulator */
    void stopNoMutex() override {}

    std::unique_ptr<ToyHardwareInterface> hardware_interface_;

    // I don't know what kind of a time this is.
    artdaq::Fragment::timestamp_t timestamp_;

    // Written do by the hardware interface
    char* readout_buffer_;
  };
}

#endif /* artdaq_demo_Generators_ToySimulator_hh */
