#ifndef artdaq_demo_Generators_AsciiSimulator_hh
#define artdaq_demo_Generators_AsciiSimulator_hh

// AsciiSimulator is a simple type of fragment generator intended to be
// studied by new users of artdaq as an example of how to create such
// a generator in the "best practices" manner. Derived from artdaq's
// CommandableFragmentGenerator class, it can be used in a full DAQ
// simulation, generating all ADC counts with equal probability via
// the std::uniform_int_distribution class

// AsciiSimulator is designed to simulate values coming in from one of
// two types of digitizer boards, one called "TOY1" and the other
// called "TOY2"; the only difference between the two boards is the #
// of bits in the ADC values they send. These values are declared as
// FragmentType enum's in artdaq-demo's
// artdaq-core-demo/Overlays/FragmentType.hh header.

// Some C++ conventions used:

// -Append a "_" to every private member function and variable

#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Application/CommandableFragmentGenerator.hh"
#include "artdaq-core-demo/Overlays/AsciiFragment.hh"
#include "artdaq-core-demo/Overlays/FragmentType.hh"

#include <random>
#include <vector>
#include <atomic>

namespace demo
{
	class AsciiSimulator : public artdaq::CommandableFragmentGenerator
	{
	public:
		explicit AsciiSimulator(fhicl::ParameterSet const& ps);

	private:

		// The "getNext_" function is used to implement user-specific
		// functionality; it's a mandatory override of the pure virtual
		// getNext_ function declared in CommandableFragmentGenerator

		bool getNext_(artdaq::FragmentPtrs& output) override;

		// Explicitly declare that there is nothing special to be done
		// by the start, stop, and stopNoMutex methods in this class
		void start() override {}
		void stop() override {}
		void stopNoMutex() override {}

		// FHiCL-configurable variables. Note that the C++ variable names
		// are the FHiCL variable names with a "_" appended

		FragmentType const fragment_type_; // Type of fragment (see FragmentType.hh)

		std::size_t const throttle_usecs_; // Sleep at start of each call to getNext_(), in us
		std::size_t const throttle_usecs_check_; // Period between checks for stop/pause during the sleep (must be less than, and an integer divisor of, throttle_usecs_)

		// Members needed to generate the simulated data
		std::string string1_;
		std::string string2_;
	};
}

#endif /* artdaq_demo_Generators_AsciiSimulator_hh */
