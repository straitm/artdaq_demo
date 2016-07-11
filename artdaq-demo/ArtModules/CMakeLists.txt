art_make(NO_PLUGINS # WAnt different link lists for modues
 BASENAME_ONLY
 LIBRARY_NAME artdaq-demo_ArtModules
 LIB_LIBRARIES artdaq-core-demo_Overlays artdaq-demo_Products
)

#art_make(NO_PLUGINS # WAnt different link lists for modues
#  LIB_LIBRARIES artdaq-core-demo_Overlays 
#)

 foreach (mod Baseline Converter Summer NthEvent )
   simple_plugin(${mod} "module" artdaq-demo_ArtModules)
 endforeach()

 simple_plugin(Compression "module"
   artdaq-demo_Compression
   artdaq-demo_Products
   artdaq-core_Utilities
   artdaq_DAQdata
   artdaq-core_Data
   ${MF_MESSAGELOGGER} 
   ${MF_UTILITIES}
   )

 simple_plugin(Decompression "module"
   artdaq-demo_Compression
   artdaq-demo_Products
   artdaq-core_Utilities
   artdaq_DAQdata
   artdaq-core_Data
   ${MF_MESSAGELOGGER} 
   ${MF_UTILITIES}
   )

simple_plugin(ArtdaqDemoBuildInfo "module"
  artdaq_DAQdata
  artdaq_BuildInfo
  artdaq-core_BuildInfo
  artdaq-demo_BuildInfo
  artdaq-core-demo_BuildInfo
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
)

 simple_plugin(DataCharacterizer "module")
 simple_plugin(EventDump "module")
simple_plugin(ToyDump "module")
simple_plugin(ASCIIDump "module")
simple_plugin(CompressionChecker "module" ${ARTDAQ_DAQDATA} ${ARTDAQ-CORE_DATA})

simple_plugin(RootApplication "module")
simple_plugin(WFViewer "module" artdaq-core-demo_Overlays ${ROOT_GPAD} ${ROOT_HIST} ${ROOT_GUI} ${ROOT_RIO})

simple_plugin(CheckIntegrity "module"
              ${MF_MESSAGELOGGER}				
	      ${MF_UTILITIES}
)
