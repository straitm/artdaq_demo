#include "art/Framework/IO/Sources/Source.h"
#include "artdaq/ArtModules/detail/SharedMemoryReader.hh"
#include "artdaq-core-demo/Overlays/FragmentType.hh"
#include "art/Framework/Core/InputSourceMacros.h"

#include <string>
using std::string;

/**
 * \brief The artdaq_demo namespace
 */
namespace demo
{
	/**
	 * \brief DemoInput is an art::Source using the detail::RawEventQueueReader class
	 */
	typedef art::Source<artdaq::detail::SharedMemoryReader<demo::makeFragmentTypeMap>> DemoInput;
}

DEFINE_ART_INPUT_SOURCE(demo::DemoInput)
