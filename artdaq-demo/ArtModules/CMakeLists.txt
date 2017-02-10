art_make( BASENAME_ONLY
 LIBRARY_NAME artdaq-demo_ArtModules
 SUBDIRS detail
  NO_PLUGINS
  DICT_DICTIONARIES
  ${ART_PERSISTENCY_PROVENANCE}
  ${CANVAS_PERSISTENCY_PROVENANCE}
 LIB_LIBRARIES 
 artdaq-core-demo_Overlays 
 artdaq_ArtModules
  art_Framework_Core
  art_Framework_IO_Sources
  art_Framework_Principal
  canvas_Persistency_Common
  art_Persistency_Provenance
  canvas_Persistency_Provenance
 canvas_Utilities
  MF_MessageLogger
  ${ROOT_CINTEX}
  ${ROOT_RIO}
  ${ROOT_CORE}
  ${ROOT_NET}
  ${ROOT_REFLEX}
  cetlib
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
simple_plugin(ToyDump "module" 
              artdaq-core_Data 
			  artdaq-core-demo_Overlays )
simple_plugin(ASCIIDump "module" 
              artdaq-core_Data 
			  artdaq-core-demo_Overlays )

simple_plugin(RootApplication "module")
simple_plugin(WFViewer "module" 
              artdaq-core_Data 
			  artdaq-core-demo_Overlays 
			  MF_MessageLogger
			  ${ROOT_GPAD} 
			  ${ROOT_HIST} 
			  ${ROOT_GUI} 
			  ${ROOT_RIO})

simple_plugin(CheckIntegrity "module"
              ${MF_MESSAGELOGGER}				
	      ${MF_UTILITIES}
)

simple_plugin(DemoInput "source"
  artdaq-demo_ArtModules
  artdaq_ArtModules
  art_Framework_Core
  art_Framework_IO_Sources
  art_Framework_Principal
  art_Persistency_Provenance
  art_Utilities
  MF_MessageLogger
)
