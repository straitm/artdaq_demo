
simple_plugin(CRTFragGen "generator"
  artdaq-demo_Generators_CRTInterface
  artdaq-core-demo_Overlays
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  artdaq-utilities_Plugins
  ${CANVAS_LIB_LIST}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
${CETLIB_EXCEPT}
  )

simple_plugin(AsciiSimulator "generator"
  artdaq-core-demo_Overlays
  artdaq_Application
  artdaq_DAQdata
  artdaq-core_Utilities
  artdaq-core_Data
  ${Boost_SYSTEM_LIBRARY}
  ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  )

simple_plugin(UDPReceiver "generator"
    artdaq-core-demo_Overlays
    artdaq_Application
    artdaq_DAQdata
    artdaq-core_Utilities
    artdaq-core_Data
  ${Boost_SYSTEM_LIBRARY}
  ${CANVAS_LIB_LIST}
    ${FHICLCPP}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
    ${CETLIB}
 ${CETLIB_EXCEPT}
    )

add_subdirectory(CRTInterface)
