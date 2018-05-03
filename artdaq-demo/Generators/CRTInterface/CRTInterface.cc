#include "artdaq-demo/Generators/CRTInterface/CRTInterface.hh"
#include "artdaq-demo/Generators/CRTInterface/CRTdecode.hh"
#define TRACE_NAME "CRTInterface"
#include "artdaq/DAQdata/Globals.hh"
#include "artdaq-core-demo/Overlays/ToyFragment.hh"
#include "artdaq-core-demo/Overlays/FragmentType.hh"

#include "fhiclcpp/ParameterSet.h"
#include "cetlib_except/exception.h"

#include <random>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <dirent.h>

#include <sys/inotify.h>

/**********************************************************************/
/* Buffers and whatnot */

// Maximum size of the data, after the header, in bytes
static const ssize_t RAWBUFSIZE = 0x10000;
static const ssize_t COOKEDBUFSIZE = 0x10000;

static char rawfromhardware[RAWBUFSIZE];
static char * next_raw_byte = rawfromhardware;

/**********************************************************************/

CRTInterface::CRTInterface(fhicl::ParameterSet const& ps) :
  indir(ps.get<std::string>("indir")),
  state(CRT_WAIT),
  taking_data_(false)
{
}

// "StartDatataking" is meant to mimic actions one would take when
// telling the hardware to start sending data - the uploading of
// values to registers, etc.

void CRTInterface::StartDatataking()
{
  taking_data_ = true;

  // Already initialized by call for another board (?)
  if(inotifyfd != -1){
    fprintf(stderr, "inotify already init'd.  Maybe this is ok if we\n"
                    "stopped and restarted data taking...?\n");
    return;
  }

  if(-1 == (inotifyfd = inotify_init())){
    perror("CRTInterface::StartDatataking");
    _exit(1);
  }

  // Set the file descriptor to non-blocking so that we can immediately
  // return from FillBuffer() if no data is available.
  fcntl(inotifyfd, F_SETFL, fcntl(inotifyfd, F_GETFL) | O_NONBLOCK);
}

void CRTInterface::StopDatataking()
{
  taking_data_ = false;
  if(-1 == inotify_rm_watch(inotifyfd, inotify_watchfd)){
    perror("CRTInterface::StopDatataking");
    _exit(1); // maybe not necessary
  }
}

char * find_wr_file(const std::string & indir)
{
  printf("Hi!  find_wr_file()\n");
  DIR * dp = NULL;
  errno = 0;
  if((dp = opendir(indir.c_str())) == NULL){
    if(errno == ENOENT){
      fprintf(stderr, "No such directory %s, but will wait for it\n",
              indir.c_str());
      usleep(100000);
      return NULL;
    }
    else{
      // Other conditions we are unlikely to recover from: permission denied,
      // too many file descriptors in use, too many files open, out of memory,
      // or the name isn't a directory.
      perror("find_wr_file opendir");
      _exit(1);
    }
  }

  struct dirent * de = NULL;
  while(errno = 0, (de = readdir(dp)) != NULL){
    // Does this file name end in ".wr"?  Having ".wr" in the middle somewhere
    // is not sufficient (and also should never happen).
    //
    // If somehow there ends up being a directory ending in ".wr", ignore it
    // (and all other directories).  I suppose all other types are fine, even
    // though we only really expect regular files.  But there's no reason not
    // to accept a named pipe, etc.
    if(de->d_type != DT_DIR &&
       strstr(de->d_name, ".wr") != NULL &&
       strlen(strstr(de->d_name, ".wr")) == strlen(".wr"))
      // As per readdir(3), this pointer is good until readdir() is called
      // again on this directory.
      return de->d_name;
  }

  // If errno == 0, it just means we got to the end of the directory.
  // Otherwise, something went wrong.  This is unlikely since the only
  // error condition is "EBADF  Invalid directory stream descriptor dirp."
  if(errno) perror("find_wr_file readdir");

  errno = 0;

  closedir(dp);

  if(errno) perror("find_wr_file closedir");

  return NULL;
}

/*
  Check if there is a file ending in ".wr" in the input directory.
  If so, open it, set an inotify watch on it, and return true.
  Else return false.
*/
bool CRTInterface::try_open_file()
{
  const char * const filename = find_wr_file(indir);

  if(filename == NULL) return false;

  if(-1 == (inotify_watchfd =
            inotify_add_watch(inotifyfd, filename, IN_MODIFY | IN_MOVE_SELF))){
    if(errno == ENOENT){
      // It's possible that the file we just found has vanished by the time
      // we get here, probably by being renamed without the ".wr".  That's
      // OK, we'll just try again in a moment.
      return false;
    }
    else{
      // But other inotify_add_watch errors we probably can't recover from
      fprintf(stderr, "CRTInterface: Could not open %s\n", filename);
      perror("CRTInterface");
      _exit(1);
    }
  }

  // XXX debugging.  Remove later.
  printf("Successful inotify_add_watch on %s, fd %d\n", filename, inotify_watchfd);

  if(-1 == (datafile_fd = open(filename, O_RDONLY))){
    if(errno == ENOENT){
      // The file we just set a watch on might already be gone, as above.
      // We'll just get the next one.
      inotify_rm_watch(inotifyfd, inotify_watchfd);
      return false;
    }
    else{
      // But other errors probably indicate an unrecoverable problem.
      perror("CRTInterface::StartDatataking");
      _exit(1);
    }
  }

  state = CRT_READ_ACTIVE;

  return true;
}

/*
  Checks for inotify events that alert us to a file being appended to
  or renamed, and update 'state' appropriately.  If no events, return
  false, meaning there is nothing to do now.
*/
bool CRTInterface::check_events()
{
  char filechange[sizeof(struct inotify_event) + NAME_MAX + 1];

  ssize_t inotify_bread = 0;

  // read() is non-blocking because I set O_NONBLOCK above
  if(-1 ==
     (inotify_bread = read(inotifyfd, &filechange, sizeof(filechange)))){

    // If there are no events, we get this error
    if(errno == EAGAIN) return false;

    // Anything else maybe should be a fatal error.  If we can't read from
    // inotify once, we probably won't be able to again.
    perror("CRTInterface::FillBuffer");
    return false;
  }

  if(inotify_bread == 0){
    // This means that the file has not changed, so we have no new data
    // (or maybe this never happens because we'd get EAGAIN above).
    return false;
  }

  if(inotify_bread < (ssize_t)sizeof(struct inotify_event)){
    fprintf(stderr, "Non-zero, yet wrong number (%ld) of bytes from inotify\n",
            inotify_bread);
    _exit(1);
  }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
  const uint32_t mask = ((struct inotify_event *)filechange)->mask;
#pragma GCC diagnostic pop

  if(mask == IN_MODIFY){
    // Active file has been modified again
    if(state == CRT_READ_ACTIVE) return true;
    else{
      fprintf(stderr, "File modified, but not watching an open file...\n");
      return false; // Should be fatal?
    }
  }
  else /* mask == IN_MOVE_SELF */ {

    // Active file has been renamed, meaning it will no longer be
    // written to.  We should read the rest and then find the next file.
    if(state == CRT_READ_ACTIVE){
      state = CRT_READ_CLOSED;
      return true;
    }
    else{
      fprintf(stderr, "Not reached.  Closed file renamed.\n");
      return false; // should be fatal?
    }
  }
}

/*
  Reads all available data from the open file.
*/
size_t CRTInterface::read_everything_from_file(char * cooked_data)
{
  // Oh boy!  Since we're here, it means we have a new file, or that the file
  // has changed.  Hopefully that means *appended to*, in which case we're
  // going to read the new bytes.  At the moment, let's ponderously read one at
  // a time.  If by "changed", in fact the file was truncated or that some
  // contents prior to our current position were changed, we'll get nothing
  // here, which will signal that such shenanigans occured.

  ssize_t read_bread = 0;

  while(next_raw_byte < rawfromhardware + RAWBUFSIZE &&
        -1 != (read_bread = read(datafile_fd, next_raw_byte, 1))){
    if(read_bread != 1) break;

    next_raw_byte += read_bread;
  }

  if(read_bread == -1){
    // All read() errors other than *maybe* EINTR should be fatal.
    perror("CRTInterface::FillBuffer");
    _exit(1);
  }

  printf("%ld bytes in raw buffer.\n", next_raw_byte - rawfromhardware);
  return CRT::raw2cook(cooked_data, COOKEDBUFSIZE,
                       rawfromhardware, next_raw_byte);
}

void CRTInterface::FillBuffer(char* cooked_data, size_t* bytes_ret)
{
  *bytes_ret = 0;

  if(!taking_data_)
    throw cet::exception("CRTInterface") <<
      "Attempt to call FillBuffer when not sending data";

  if(state == CRT_WAIT){
    if(!try_open_file()) return;
  }

  if(!check_events()) return;

  if(state == CRT_READ_ACTIVE){
    *bytes_ret = read_everything_from_file(cooked_data);
  }
  else /* if(state == CRT_READ_CLOSED */ {
    *bytes_ret = read_everything_from_file(cooked_data);
    state = CRT_WAIT;
  }
  printf("Decoded to %ld bytes\n", *bytes_ret);

}

void CRTInterface::AllocateReadoutBuffer(char** cooked_data)
{
  *cooked_data = new char[COOKEDBUFSIZE];
}

void CRTInterface::FreeReadoutBuffer(char* cooked_data)
{
  delete[] cooked_data;
}
