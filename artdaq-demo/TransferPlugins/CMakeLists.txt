
include_directories($ENV{MPICH_INC} )

simple_plugin(NthEvent "transfer"
  artdaq_TransferPlugins
  artdaq-core_Utilities
  artdaq_DAQrate
  MF_MessageLogger
  fhiclcpp
  ${CETLIB}
  ${ROOT_RIO}
  ${ROOT_NET}
  ${ROOT_CORE}
)

install_headers()
install_source()
