art_make(NO_PLUGINS # WAnt different link lists for modues
 BASENAME_ONLY
 LIBRARY_NAME artdaq-demo_ArtModules
 LIB_LIBRARIES artdaq-core-demo_Overlays artdaq-demo_Products
)

#art_make(NO_PLUGINS # WAnt different link lists for modues
#  LIB_LIBRARIES artdaq-core-demo_Overlays 
#)

# foreach (mod NthEvent )
#   simple_plugin(${mod} "module" artdaq-demo_ArtModules)
# endforeach()

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

simple_plugin(NthEvent "module")
 simple_plugin(EventDump "module")
simple_plugin(ToyDump "module")
simple_plugin(ASCIIDump "module")

simple_plugin(RootApplication "module")
simple_plugin(WFViewer "module" artdaq-core-demo_Overlays ${ROOT_GPAD} ${ROOT_HIST} ${ROOT_GUI} ${ROOT_RIO})

simple_plugin(CheckIntegrity "module"
              ${MF_MESSAGELOGGER}				
	      ${MF_UTILITIES}
)
