
simple_plugin(ToySimulator "generator"
  artdaq-demo_Generators_ToyHardwareInterface
  artdaq-core-demo_Overlays
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  canvas_Utilities
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  )

simple_plugin(AsciiSimulator "generator"
  artdaq-core-demo_Overlays
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${FHICLCPP}
  ${CETLIB}
  )

simple_plugin(UDPReceiver "generator"
    artdaq-core-demo_Overlays
    artdaq_Application
    artdaq_DAQdata
    artdaq-core_Utilities
    artdaq-core_Data
  canvas_Utilities
    ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
    ${CETLIB}
    )

add_subdirectory(ToyHardwareInterface)
