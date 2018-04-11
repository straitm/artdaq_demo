#ifndef artdaq_demo_Generators_ToyHardwareInterface_ToyHardwareInterface_hh
#define artdaq_demo_Generators_ToyHardwareInterface_ToyHardwareInterface_hh

#include "artdaq-core-demo/Overlays/FragmentType.hh"

#include "fhiclcpp/fwd.h"

#include <random>
#include <chrono>

class ToyHardwareInterface
{
public:

	explicit ToyHardwareInterface(fhicl::ParameterSet const& ps);

	void StartDatataking();

	void StopDatataking();

	/**
	 * \brief Use configured generator to fill a buffer with data
	 * \param buffer Buffer to fill
	 * \param bytes_read Number of bytes to fill
	 */
	void FillBuffer(char* buffer, size_t* bytes_read);

	/**
	 * \brief Request a buffer from the hardware
	 * \param buffer (output) Pointer to buffer
	 */
	void AllocateReadoutBuffer(char** buffer);

	/**
	 * \brief Release the given buffer to the hardware
	 * \param buffer Buffer to release
	 */
	void FreeReadoutBuffer(char* buffer);

private:

	bool taking_data_;

  // File descriptor associated with the inotify event queue, which is
  // used to find out when there is new data to read.
  int inotifyfd = -1;

  // File descriptor associated with the inotify watch on the data file
  int inotify_watchfd = -1;

  // File descriptor for the data file we are reading
  int datafile_fd = -1;

	demo::FragmentType fragment_type_;
};

#endif
