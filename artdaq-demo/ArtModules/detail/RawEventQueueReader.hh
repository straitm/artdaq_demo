#ifndef artdaq_demo_ArtModules_detail_RawEventQueueReader_hh
#define artdaq_demo_ArtModules_detail_RawEventQueueReader_hh

#include <string>
#include <map>

#include "artdaq/ArtModules/detail/RawEventQueueReader.hh"

namespace demo
{
	namespace detail
	{
		/**
		 * \brief A derived class of artdaq::detail::RawEventQueueReader which knows about the artdaq_core_demo Fragment types
		 */
		struct RawEventQueueReader : public artdaq::detail::RawEventQueueReader
		{
			/**
			 * \brief Copy Constructor is deleted
			 */
			RawEventQueueReader(RawEventQueueReader const&) = delete;

			/**
			 * \brief Copy Assignment operator is deleted
			 * \return Assigned RawEventQueueReader
			 */
			RawEventQueueReader& operator=(RawEventQueueReader const&) = delete;

			/**
			* \brief RawEventQueueReader Constructor
			* \param ps ParameterSet for RawEventQueueReader
			* \param help art::ProductRegistryHelper (where Fragment types are registered)
			* \param pm art::SourceHelper reference
			* 
			* This Constructor creates an artdaq::detail::RawEventQueueReader, then
			* registers the additional Fragment types defined in artdaq_core_demo.
			*/
			RawEventQueueReader(fhicl::ParameterSet const& ps,
								art::ProductRegistryHelper& help,
								art::SourceHelper const& pm);

			/**
			 * \brief RawEventQueueReader Constructor
			 * \param ps ParameterSet for RawEventQueueReader
			 * \param help art::ProductRegistryHelper (where Fragment types are registered)
			 * \param pm art::SourceHelper reference
			 * 
			 * This constructor calls the other constructor without the MasterProductRegistry
			 */
			RawEventQueueReader(fhicl::ParameterSet const& ps,
								art::ProductRegistryHelper& help,
								art::SourceHelper const& pm,
								art::MasterProductRegistry&) : RawEventQueueReader(ps, help, pm) {}
		};
	} // detail
} // demo

namespace art
{
	/**
	 * \brief  Specialize an art source trait to tell art that we don't care about
	 * source.fileNames and don't want the files services to be used.
	 */
	template <>
	struct Source_generator<demo::detail::RawEventQueueReader>
	{
		static constexpr bool value = true; ///< Used to suppress use of file services on art Source
	};
}

#endif /* artdaq_demo_ArtModules_detail_RawEventQueueReader_hh */
