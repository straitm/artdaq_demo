#ifdef CANVAS
#include "canvas/Persistency/Common/Wrapper.h"
#else
#include "art/Persistency/Common/Wrapper.h"
#endif

#include "artdaq-demo/Products/Channel.hh"

#include <vector>

template class std::vector<darkart::Channel>;
template class art::Wrapper<std::vector<darkart::Channel> >;
template class art::Wrapper<darkart::Channel>;

