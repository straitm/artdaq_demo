#include "art/Framework/IO/Sources/Source.h"
#include "artdaq-demo/ArtModules/detail/RawEventQueueReader.hh"
#include "art/Framework/Core/InputSourceMacros.h"

#include <string>
using std::string;

namespace demo {

  typedef art::Source<detail::RawEventQueueReader> DemoInput;
}

DEFINE_ART_INPUT_SOURCE(demo::DemoInput)
