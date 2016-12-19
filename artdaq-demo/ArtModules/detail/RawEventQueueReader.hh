#ifndef artdaq_demo_ArtModules_detail_RawEventQueueReader_hh
#define artdaq_demo_ArtModules_detail_RawEventQueueReader_hh

#include <string>
#include <map>

#include "artdaq/ArtModules/detail/RawEventQueueReader.hh"

namespace demo {
  namespace detail {
    struct RawEventQueueReader : public artdaq::detail::RawEventQueueReader {
      RawEventQueueReader(RawEventQueueReader const &) = delete;
      RawEventQueueReader & operator=(RawEventQueueReader const &) = delete;

      RawEventQueueReader(fhicl::ParameterSet const & ps,
                          art::ProductRegistryHelper & help,
                          art::SourceHelper const & pm);

      RawEventQueueReader(fhicl::ParameterSet const & ps,
                          art::ProductRegistryHelper & help,
                          art::SourceHelper const & pm,
			  art::MasterProductRegistry&) : RawEventQueueReader(ps, help, pm) {}
    };

  } // detail
} // demo


#endif /* artdaq_demo_ArtModules_detail_RawEventQueueReader_hh */
