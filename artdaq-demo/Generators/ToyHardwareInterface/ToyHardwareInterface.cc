#include "artdaq-demo/Generators/ToyHardwareInterface/ToyHardwareInterface.hh"
#include "artdaq-demo/Generators/ToyHardwareInterface/CRTdecode.hh"
#define TRACE_NAME "ToyHardwareInterface"
#include "artdaq/DAQdata/Globals.hh"
#include "artdaq-core-demo/Overlays/ToyFragment.hh"
#include "artdaq-core-demo/Overlays/FragmentType.hh"

#include "fhiclcpp/ParameterSet.h"
#include "cetlib_except/exception.h"

#include <random>
#include <unistd.h>
#include <iostream>
#include <cstdlib>

#include <sys/inotify.h>

/**********************************************************************/
/* Buffers and whatnot */

// Maximum size of the data, after the header, in bytes
static const ssize_t RAWBUFSIZE = 0x10000;
static const ssize_t COOKEDBUFSIZE = 0x10000;

static char rawfromhardware[RAWBUFSIZE];
static char * next_raw_byte = rawfromhardware;

/**********************************************************************/

ToyHardwareInterface::ToyHardwareInterface(
  __attribute__((unused)) fhicl::ParameterSet const& ps) :
  taking_data_(false)
{
}

// "StartDatataking" is meant to mimic actions one would take when
// telling the hardware to start sending data - the uploading of
// values to registers, etc.

void ToyHardwareInterface::StartDatataking()
{
  taking_data_ = true;

  // Already initialized by call for another board (?)
  if(inotifyfd != -1){
    fprintf(stderr, "inotify already init'd.  Maybe this is ok if we\n"
                    "stopped and restarted data taking...?\n");
    return;
  }

  if(-1 == (inotifyfd = inotify_init())){
    perror("ToyHardwareInterface::StartDatataking");
    _exit(1);
  }

  // Hardcoded for now.  Later to track input files if we go this
  // route.
  const char * const filename = "/tmp/1506152664_21";

  if(-1 == (inotify_watchfd =
            inotify_add_watch(inotifyfd, filename, IN_MODIFY))){
    perror("ToyHardwareInterface::StartDatataking");
    _exit(1);
  }

  printf("Successful inotify_add_watch, fd %d\n", inotify_watchfd);

  if(-1 == (datafile_fd = open(filename, O_RDONLY))){
    perror("ToyHardwareInterface::StartDatataking");
    _exit(1);
  }
}

void ToyHardwareInterface::StopDatataking()
{
  taking_data_ = false;
  if(-1 == inotify_rm_watch(inotifyfd, inotify_watchfd)){
    perror("ToyHardwareInterface::StopDatataking");
    _exit(1); // maybe not necessary
  }
}

void ToyHardwareInterface::FillBuffer(char* cooked_data, size_t* bytes_ret)
{
  *bytes_ret = 0;

  if(!taking_data_)
    throw cet::exception("ToyHardwareInterface") <<
      "Attempt to call FillBuffer when not sending data";

  char filechange[sizeof(struct inotify_event) + NAME_MAX + 1];

  ssize_t inotify_bread = 0;

  if(-1 ==
     (inotify_bread = read(inotifyfd, &filechange, sizeof(filechange)))){
    // Might should be a fatal error.  If we can't read from inotify
    // once, we probably won't be able to again.
    perror("ToyHardwareInterface::FillBuffer");
    return;
  }

  if(inotify_bread == 0){
    // This means that the file has not changed, so we have no new data
    return;
  }

  if(inotify_bread < (ssize_t)sizeof(struct inotify_event)){
    fprintf(stderr, "Non-zero, yet wrong number (%ld) of bytes from inotify\n",
            inotify_bread);
    _exit(1);
  }

  // Oh boy!  Since we're here, it means that the file has changed.
  // Hopefully that means *appended to*, in which case we're going
  // to read the new bytes.  At the moment, let's ponderously read one
  // at a time.  If by "changed", in fact the file was truncated or
  // that some contents prior to our current position were changed,
  // we'll get nothing here, which will signal that such shenanigans occured.

  ssize_t read_bread = 0;

  while(next_raw_byte < rawfromhardware + RAWBUFSIZE &&
        -1 != (read_bread = read(datafile_fd, next_raw_byte, 1))){
    if(read_bread != 1) break;

    next_raw_byte += read_bread;
  }

  if(read_bread == -1){
    perror("ToyHardwareInterface::FillBuffer");
    // Maybe should be fatal?
  }

  printf("%ld bytes in raw buffer.\n", next_raw_byte - rawfromhardware);
  *bytes_ret = CRT::raw2cook(cooked_data, COOKEDBUFSIZE,
                             rawfromhardware, next_raw_byte);
  printf("Decoded to %ld bytes\n", *bytes_ret);
}

void ToyHardwareInterface::AllocateReadoutBuffer(char** cooked_data)
{
  *cooked_data = new char[COOKEDBUFSIZE];
}

void ToyHardwareInterface::FreeReadoutBuffer(char* cooked_data)
{
  delete[] cooked_data;
}
